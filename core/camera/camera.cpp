//
// Created by canderson on 5/6/16.
//
#include <memory>
#include <cmath>
#include <random>

// Vermilion Headers
#include "camera.h"
#include "../compat.h"



// OIIO
#include "OpenImageIO/imagebuf.h"

// BOOST PNG FIX
//#define png_infopp_NULL (png_infopp)NULL
//#define int_p_NULL (int *)NULL

void Vermilion::Camera::GenerateTileSet()
{
	// Get number of tiles in the dimx
	uBucketsU = uImageU / uTileSize + (1 * (uImageU % uTileSize != 0));
	uBucketsV = uImageV / uTileSize + (1 * (uImageV % uTileSize != 0));

	auto tBuckets = uBucketsU * uBucketsV;

	vTileSet.reserve(tBuckets);


	for (auto i = 0; i < uBucketsU; i++)
		for (auto j = 0; j < uBucketsV; j++)
			vTileSet.push_back(frameTile(uTileSize, uTileSize, uTileSize * i, uTileSize * j));
}


Vermilion::Camera::Camera(cameraSettings& _settings)
{
    uRaysFired = 0;
    uRaysHit = 0;
	RenderTargetSize = _settings.imageResX * _settings.imageResY;
	mDistToFilm = _settings.fBackDistance;
	fAngleOfView = _settings.horAngleOfView;
	uMaxBounces = _settings.rayMaxBounces;
	mPosition = glm::vec3(_settings.position.x, _settings.position.y, _settings.position.z);
	mRotation = glm::vec3(_settings.rotation.x, _settings.rotation.y, _settings.rotation.z);
	uSamplesPerPixel = _settings.raysPerPixel;
	uTileSize = _settings.tileSize;
	uImageU = _settings.imageResX;
	uImageV = _settings.imageResY;
	renderMode = _settings.renderMode;

    //GenerateTileSet(); // Don't actually do this...
    // Instead have an X by Y image and just render in buckets...

	switch(renderMode)
	{
	case vermRenderMode::RGB:
		mImage = new float[RenderTargetSize * 3];
		break;
	case vermRenderMode::RGBA:
		mImage = new float[RenderTargetSize * 4];
		break;
	case vermRenderMode::RGBAZ:
		mImage = new float[RenderTargetSize * 5];
		break;
	case vermRenderMode::Depth:
		mImage = new float[RenderTargetSize];
		break;
	case vermRenderMode::Depth64:
		CXX17_FALLTHROUGH
	case vermRenderMode::TOTAL_RENDER_MODES:
		CXX17_FALLTHROUGH
	default:
		throw std::exception();
	}

	mMeshEngine = nullptr;
}

Vermilion::Camera::~Camera()
{
	delete mImage;
}

void Vermilion::Camera::setPixelValue(pixelValue& newPixelValue)
{

}

void Vermilion::Camera::RenderTile(frameTile& rTile)
{
	// Render Logic :O
	for (auto i = 0; i < uTileSize; i++)
		for (auto j = 0; j < uTileSize; j++)
        {
            printf("Indexing %d,%d\n",i,j);
		    rTile.setColor(i, j, float4(0.89f,0.2588f,0.204f,1.f));
        }
}

inline Vermilion::FLOAT triangleIntersect(glm::vec3 &pos, glm::vec3 &rot, glm::vec3 &v0, glm::vec3 &v1, glm::vec3 &v2 )
{
    using namespace glm;
    // Triangle Intersection REMOVE FOR PUSH
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    // Calculate planes normal vector
    vec3 pvec = cross(rot, e2);
    float det = dot(e1, pvec);

    // Ray is parallel to plane
    if (det < 1e-8 && det > -1e-8) {
        return 0;
    }

    float inv_det = 1 / det;
    vec3 tvec = pos - v0;
    float u = dot(tvec, pvec) * inv_det;
    if (u < 0 || u > 1) {
        return 0;
    }

    vec3 qvec = cross(tvec, e1);
    float v = dot(rot, qvec) * inv_det;
    if (v < 0 || u + v > 1) {
        return 0;
    }
    return dot(e2, qvec) * inv_det;
}

bool Vermilion::Camera::rayShadowCast(glm::vec3 pos, glm::vec3 dir)
{
    aiMesh *mesh;
    glm::vec3 vU, vV, vNorm, v0,v1,v2;
    uint32_t temp;
    // Look for collisions
    //for(aiMesh *&mesh : mMeshEngine->sceneMeshes)
    for(uint32_t x = 0; x < mMeshEngine->sceneMeshes.size(); ++x)
    {
        mesh = mMeshEngine->sceneMeshes[x];
        for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            // Get the face data
            temp = mesh->mFaces[i].mIndices[0];
            v0.x = mesh->mVertices[temp].x;
            v0.y = mesh->mVertices[temp].y;
            v0.z = mesh->mVertices[temp].z;

            temp = mesh->mFaces[i].mIndices[1];
            v1.x = mesh->mVertices[temp].x;
            v1.y = mesh->mVertices[temp].y;
            v1.z = mesh->mVertices[temp].z;

            temp = mesh->mFaces[i].mIndices[2];
            v2.x = mesh->mVertices[temp].x;
            v2.y = mesh->mVertices[temp].y;
            v2.z = mesh->mVertices[temp].z;

            //printf("%f,%f,%f\n",v2.x,v2.y,v2.z);

            vU = v1 - v0;
            vV = v2 - v0;
            vNorm.x = (vU.y * vV.z) - (vU.z * vV.y);
            vNorm.y = (vU.z * vV.x) - (vU.x * vV.z);
            vNorm.z = (vU.x * vV.y) - (vU.y * vV.x);


            vNorm = vNorm * glm::vec3(-1,-1,-1);

            if(glm::dot(dir, vNorm) > 0.f)
                continue;
            if(triangleIntersect(pos, dir, v0, v1, v2) > 0.f)
                return true;
        }
    }
    return false;
}

Vermilion::float4 Vermilion::Camera::rayCast(float3 pos, float3 rot, float ofu, float ofv,
bool recursive = false)
{
    auto temp = 0;
    ++uRaysFired;
    glm::vec3 gpos;
    gpos.x = pos.x;// + ofu;
//    gpos.x /= 2;
    gpos.y = pos.y;// - ofv;
  //  gpos.y /= 2;
    gpos.z = pos.z;

    glm::vec3 grot;
    grot.x = rot.x;
    grot.y = rot.y;
    grot.z = rot.z;

    const glm::vec3 lightDirection = glm::vec3(-0.5f,0.f,0.5f);
    //const float3 lightDirection_f3 = float3(-0.5f,1.f,0.5f);

    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;

    glm::vec3 hitLoc;
    
    float4 rayColour(
        0.890196078f,
        0.258823529,
        0.203921569,
        0.f
    );

    FLOAT nearestHit = 1000000.f;
    FLOAT testHit = 0.f;

    // Look for collisions
    //for(aiMesh *&mesh : mMeshEngine->sceneMeshes)
    for(uint32_t x = 0; x < mMeshEngine->sceneMeshes.size(); ++x)
    {
        aiMesh *mesh = mMeshEngine->sceneMeshes[x];
        for(uint32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            // Get the face data
            temp = mesh->mFaces[i].mIndices[0];
            v0.x = mesh->mVertices[temp].x;
            v0.y = mesh->mVertices[temp].y;
            v0.z = mesh->mVertices[temp].z;

            temp = mesh->mFaces[i].mIndices[1];
            v1.x = mesh->mVertices[temp].x;
            v1.y = mesh->mVertices[temp].y;
            v1.z = mesh->mVertices[temp].z;

            temp = mesh->mFaces[i].mIndices[2];
            v2.x = mesh->mVertices[temp].x;
            v2.y = mesh->mVertices[temp].y;
            v2.z = mesh->mVertices[temp].z;

            //printf("%f,%f,%f\n",v2.x,v2.y,v2.z);

            glm::vec3 vU = v1 - v0;
            glm::vec3 vV = v2 - v0;
            glm::vec3 vNorm;
            vNorm.x = (vU.y * vV.z) - (vU.z * vV.y);
            vNorm.y = (vU.z * vV.x) - (vU.x * vV.z);
            vNorm.z = (vU.x * vV.y) - (vU.y * vV.x);

            //if(recursive)
                vNorm = vNorm * glm::vec3(-1,-1,-1);

            if(glm::dot(grot, vNorm) > 0.f)
                continue;


            testHit = triangleIntersect(gpos, grot, v0, v1, v2);
            //if(__builtin_expect(
            //    (testHit > 0.f && (testHit < nearestHit || !recursive)),
            //    0
            //))
            if(testHit > 0.f && testHit < nearestHit)
            {
                //const glm::vec3 lightDirection = glm::vec3(0.f,-1.f,0.0f);
                //const float3 lightDirection_f3 = float3(0.f,-1.f,0.0f);
                // ndl
                float vNDL = glm::dot(lightDirection, vNorm);
                

                // Toon, band the lighting vec
                //vNDL = (std::floor(vNDL * 16.f)) * 0.0625f;

                // Recursive call
                //if(testHit > 0.f)
                {
                    // We should attempt to shadow. #notefficient
                    // We did we actually hit?
                    hitLoc = gpos + (grot * testHit);
                    hitLoc += (vNorm * 1e-3f); //perturb be minor

                    // We are shadowing
                    auto temper = rayShadowCast(hitLoc, lightDirection);
                    if(temper)
                    {
                        vNDL = 0.f;
                    }


                    //float3 indNorm;
                    //indNorm.x = vNorm.x;
                    //indNorm.y = vNorm.y;
                    //indNorm.z = vNorm.z;

                    // Indirect lighting :0
                    //float4 ind = rayCast(hitLoc_f3, indNorm, 0.f,0.f,false);
                    //if(ind.w > 0.f)
                    //    vNDL *= 0.1f;
                } 


                // Contrast vNDL on 0.75 pivot
                //vNDL = ((vNDL - 0.75f) * 1.2) + 0.75f;

                vNDL *= 1.5f;
                vNDL *= (vNDL > 0.f); // min of 0.f
                vNDL = ((vNDL - 1.f) * (vNDL < 1.f)) + 1.f;

                float3 lC = float3(
                    0.890196078f,
                    0.258823529,
                    0.203921569
                ) * vNDL;
                
                rayColour.x =  lC.x;//* 0.5f;
                rayColour.y =  lC.y;// * 0.5f;
                rayColour.z =  lC.z;// * 0.5f;
                rayColour.w = 1.f;

                ++uRaysHit;
                nearestHit = testHit;
            }
        }

    }

    return rayColour;

}

void Vermilion::Camera::saveFrame(std::string name)
{
    auto outFrame = new unsigned char[uImageU * uImageV * 4];
	float * outDepth = nullptr;
	if(renderMode == vermRenderMode::RGBAZ)
	{
		outDepth = new float[uImageU * uImageV];
	}
	if(renderMode == vermRenderMode::Depth)
	{
		outDepth = mImage;
	}

    // Do 32f to 16u conversion
    for( uint64_t p = 0; p < uImageU * uImageV; ++p)
    {
        // CLAMP
        // SRGB transform
		switch (renderMode)
		{
		case vermRenderMode::RGB:
			outFrame[p * 4 + 0] = static_cast<unsigned char>(std::floor(mImage[p * 3] * 255));
			outFrame[p * 4 + 1] = static_cast<unsigned char>(std::floor(mImage[p * 3 + 1] * 255));
			outFrame[p * 4 + 2] = static_cast<unsigned char>(std::floor(mImage[p * 3 + 2] * 255));
			outFrame[p * 4 + 3] = 1;
			break;
		case vermRenderMode::RGBA:
			outFrame[p * 4 + 0] = static_cast<unsigned char>(std::floor(mImage[p * 4 + 0] * 255));
			outFrame[p * 4 + 1] = static_cast<unsigned char>(std::floor(mImage[p * 4 + 1] * 255));
			outFrame[p * 4 + 2] = static_cast<unsigned char>(std::floor(mImage[p * 4 + 2] * 255));
			outFrame[p * 4 + 3] = static_cast<unsigned char>(std::floor(mImage[p * 4 + 3] * 255));
			break;
		case vermRenderMode::RGBAZ:
			outFrame[p * 4 + 0] = static_cast<unsigned char>(std::floor(mImage[p * 5 + 0] * 255));
			outFrame[p * 4 + 1] = static_cast<unsigned char>(std::floor(mImage[p * 5 + 1] * 255));
			outFrame[p * 4 + 2] = static_cast<unsigned char>(std::floor(mImage[p * 5 + 2] * 255));
			outFrame[p * 4 + 3] = static_cast<unsigned char>(std::floor(mImage[p * 5 + 3] * 255));
			outDepth[p] = mImage[p * 5 + 4];
			break;
		case vermRenderMode::Depth:
			p = RenderTargetSize; // force exit
			break;
		case vermRenderMode::Depth64:
			CXX17_FALLTHROUGH
		case vermRenderMode::TOTAL_RENDER_MODES:
			CXX17_FALLTHROUGH
		default:
			throw std::exception();
		}

    }

    //TODO FILE OUT
    OpenImageIO::ImageSpec spec(uImageU, uImageV, 4);
    auto outBuffer = OpenImageIO::ImageBuf(spec, (void*)outFrame);
    outBuffer.write(name + ".png", "png");

	if (renderMode == vermRenderMode::Depth || renderMode == vermRenderMode::RGBAZ)
	{
		OpenImageIO::ImageSpec dImageSpec(uImageU, uImageV, 1, TypeDesc::FLOAT);
		auto depthOutBuffer = OpenImageIO::ImageBuf(dImageSpec, (void*)outDepth);
		depthOutBuffer.write(name + "_depth.exr", ".exr");
	}

    delete[] outFrame;
	if (renderMode != vermRenderMode::Depth)
		delete[] mImage;
}

