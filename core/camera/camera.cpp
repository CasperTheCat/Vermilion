//
// Created by canderson on 5/6/16.
//
#include <memory>
#include <cmath>

// Vermilion Headers
#include "camera.h"



// OIIO
#include "OpenImageIO/imagebuf.h"

// BOOST PNG FIX
//#define png_infopp_NULL (png_infopp)NULL
//#define int_p_NULL (int *)NULL

enum class Refl_t : uint8_t 
{
    DIFF,
    METAL,
    SPEC,
    REFR,
    COAT
};

struct BVHTravel 
{
    uint32_t i;
    float mint;
    BVHTravel(){}
    BVHTravel(int _i, float _mint) : i(_i), mint(_mint) {}
};



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


Vermilion::Camera::Camera(cameraSettings& _settings, MeshEngine *mEng) : mtRanEngine(time(0)), distrib(0, 1)
{
    uRaysFired = 0;
    uRaysHit = 0;

	mDistToFilm = _settings.fBackDistance;
	fAngleOfView = _settings.horAngleOfView;
	uMaxBounces = _settings.rayMaxBounces;
	mPosition = _settings.position;
	mRotation = _settings.rotation;
	uSamplesPerPixel = _settings.raysPerPixel;
	uTileSize = _settings.tileSize;
	uImageU = _settings.imageResX;
	uImageV = _settings.imageResY;
    mUp = float3(0,1,0);
    mFocalDistance = 0.5f;

    //GenerateTileSet(); // Don't actually do this...
    // Instead have an X by Y image and just render in buckets...

    mImage = new glm::vec4[uImageU * uImageV];
    
    // Image is currently junk, zero it out?

    mMeshEngine = mEng;
}

Vermilion::Camera::~Camera()
{
    // TODO
    if(mImage) delete mImage;
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

glm::vec4 Vermilion::Camera::renderKernel(glm::vec3 oriInWS, glm::vec3 rayInWS)
{
    glm::vec3 accum = glm::vec3(0.9,0.5,0.05);
    glm::vec3 mask = glm::vec3(1);
    glm::vec3 direct = glm::vec3(0);
    bool bHasHit = false;

    for(uint32_t bounce = 0; bounce < uMaxBounces; ++bounce)
    {
        int hitTriIdx = -1;
        int bestTriIdx = -1;
        float hitDist = 1e20;
        float st = 1e20;
        glm::vec3 objCol = glm::vec3(0);
        glm::vec3 emit = glm::vec3(0);
        glm::vec3 hitPoint = glm::vec3(0);
        glm::vec3 normal = glm::vec3(0);
        glm::vec3 orientedNormal = glm::vec3(0);
        glm::vec3 nextDir = glm::vec3(0);
        glm::vec3 triNormal = glm::vec3(0);
        Refl_t reflType;
        float ray_tmin = 0.00001f;
        float ray_tmax = 1e20;

        //bHasHit = intersectBVH(oriInWS, rayInWS, bestTriIdx, hitDist, triNormal);
        IntersectionInfo hitInfo;
        bHasHit = sceneBVH.getIntersection(oriInWS, rayInWS, &hitInfo, false);
        if(!bHasHit) break;
        hitDist = hitInfo.t;
        triNormal = hitInfo.object.getNormal();

        float t = 0.5f * (rayInWS.y + 1.2f);
        Vec3f skycolor = Vec3f(1.0f, 1.0f, 1.0f) * (1.0f - t) + Vec3f(0.9f, 0.3f, 0.0f) * t;
        
        st = hitDist;
        hitTriIdx = bestTriIdx;



        {
            hitPoint = oriInWS + rayInWS * st;

            normal = glm::normalize(triNormal);
            orientedNormal = (glm::dot(normal, rayInWS) < 0) ? normal : normal * glm::vec3(-1);

            glm::vec3 col = glm::vec3(1);

            reflType = Refl_t::REFR;
            objCol = glm::vec3(0);
            emit = glm::vec3(0);
            accum += (mask * emit);
        }
        

        if(reflType == Refl_t::DIFF)
        {
            float phi = 2 * M_PI * distrib(mtRanEngine);
            float r2 = distrib(mtRanEngine);
            float r2s = sqrtf(r2);

            // Compute orthonormal
            glm::vec3 w = orientedNormal;
            glm::vec3 u = glm::normalize(glm::cross((fabs(w.x) > .1 ? glm::vec3(0,1,0) : glm::vec3(1,0,0)), w));
            glm::vec3 v = glm::cross(w,u);

            // Compute cosine
            //nextDir = u*cosf(phi)*r2s + v*sinf(phi)*r2s + w*sqrtf(1-r2);

        }
        if(reflType == Refl_t::SPEC)
        {
            nextDir = glm::normalize(rayInWS - normal * glm::dot(normal, rayInWS) * 2.0f);
            hitPoint += orientedNormal * 0.001f;
            mask *= objCol;
        }
        if(reflType == Refl_t::REFR)
        {
            bool into = glm::dot(normal, orientedNormal) > 0;
            float nc = 1.0f;
            float nt = 1.5f;
            float nnt = into ? nc / nt : nt / nc;
            float ddn = glm::dot(rayInWS, orientedNormal);
            float cos2t = 1.0f - nnt*nnt * (1.f - ddn*ddn);

            if(cos2t < 0.0f)
            {
                nextDir = glm::normalize(rayInWS - normal * glm::vec3(2) * glm::dot(normal, rayInWS));
                hitPoint += orientedNormal * 0.001f;
            }
            else
            {
                // compute direction of transmission ray
				glm::vec3 tdir = rayInWS * nnt;
				tdir -= normal * ((into ? 1 : -1) * (ddn*nnt + sqrtf(cos2t)));
				tdir = glm::normalize(tdir);

				float R0 = (nt - nc)*(nt - nc) / (nt + nc)*(nt + nc);
				float c = 1.f - (into ? -ddn : glm::dot(tdir, normal));
				float Re = R0 + (1.f - R0) * c * c * c * c * c;
				float Tr = 1 - Re; // Transmission
				float P = .25f + .5f * Re;
				float RP = Re / P;
				float TP = Tr / (1.f - P);

				// randomly choose reflection or transmission ray
				if (distrib(mtRanEngine) < 0.2f) // reflection ray
				{
					//mask *= RP;
					nextDir = glm::normalize(rayInWS - normal * glm::vec3(2) * dot(normal, rayInWS));

					hitPoint += orientedNormal * glm::vec3(0.001f); // scene size dependent
				}
				else // transmission ray
				{
					//mask *= TP;
					nextDir = glm::normalize(tdir);

					hitPoint += orientedNormal * glm::vec3(0.001f); // epsilon must be small to avoid artefacts
                }
            }
        }

        oriInWS = hitPoint;
        rayInWS = nextDir;
    }

    return glm::vec4(accum, bHasHit);
}

void Vermilion::Camera::renderFrame()
{
    // Random


    // YawPitchRoll
    float xDir = sin(mRotation.x) * cos(mRotation.y);
    float yDir = sin(mRotation.y);
    float zDir = cos(mRotation.x) * cos(mRotation.y);
    glm::vec3 directionToCamera = glm::vec3(xDir,yDir,zDir);
    glm::vec3 viewDirection = glm::normalize(directionToCamera * glm::vec3(-1));
    glm::vec3 eyePos = mPosition.tovec3() + directionToCamera * glm::vec3(4);

    glm::vec3 mid = eyePos + viewDirection;
    glm::vec3 horAxis = glm::normalize(glm::cross(viewDirection, mUp.tovec3()));
    glm::vec3 verAxis = glm::normalize(glm::cross(horAxis, viewDirection));

    //auto temp = (direction * mDistToFilm);

    // Later, bin the pixel grid
#pragma omp parallel for
    for( int64_t pix = 0; pix < uImageU * uImageV; ++pix)
    {
        glm::vec4 accum = glm::vec4(0);
        int px = pix % uImageU;
        int py = uImageV - (pix / uImageU);

        //glm::vec3 cx = glm::vec3(uImageU * fAngleOfView / uImageV, 0, 0);
        //glm::vec3 cy = glm::cross(cx, viewDirection);
        //cy = glm::normalize(cy);

        for(uint32_t s = 0; s < uSamplesPerPixel; ++s)
        {
            auto jitterX = distrib(mtRanEngine) - 0.5;
            auto jitterY = distrib(mtRanEngine) - 0.5;
            
            auto sX = (jitterX + px) / (uImageU - 1);
            auto sY = (jitterY + py) / (uImageV - 1);

            glm::vec3 pointOnPlane = mid + (horAxis * glm::vec3(( 2 * sX) - 1)) + (verAxis * glm::vec3((2 * sY) - 1));
            glm::vec3 pointOnImagePlane = eyePos + ((pointOnPlane - eyePos) * mFocalDistance);
            
            // Skip aperture
            glm::vec3 originInWS = eyePos;

            glm::vec3 rayInWS = glm::normalize(pointOnImagePlane - originInWS);

            accum += renderKernel(originInWS, rayInWS) * glm::vec4(1.0 / uSamplesPerPixel);
        }
        mImage[pix] = accum;
    }

}

//void Vermilion::Camera::renderFrame()
//{
	// Make each of tiles render. This is serial for now because threading...
    // No buckets currently

    // random sampler
    //std::mt19937 mtRanEngine(time(0));
    //std::uniform_real_distribution<float> distrib(0, 1);

    //float3 direction = float3(0,0,-1);

    //float fOffX;
    //float fOffY;
    //float4 accum = float4(0,0,0,0);
    
    // Camera Position is mPosition less fBackDist*direction
    //auto temp = (direction * mDistToFilm); // No offset for grid yet, move by grid
    //float3 camPos = mPosition;
    //camPos.x -= temp.x;
    //camPos.y -= temp.y;
    //camPos.z -= temp.z;

//#pragma omp parallel for
//    for( int64_t p = 0; p < uImageU * uImageV; ++p)
//    {
//        // raycasts per pixel
//        float4 accum = float4(0,0,0,0);
//        //for(uint32_t i = 0; i < uSamplesPerPixel; ++i)
//        //{
//        //    // Take 1 and div by samples
//        //    float fOffX = (((p % uImageU) - (uImageU / 2)) + distrib(mtRanEngine) - 0.5) * 0.01;
//        //    float fOffY = (((p / uImageU) - (uImageV / 2)) + distrib(mtRanEngine) - 0.5) * 0.01; 
//        //    
//        //    accum = accum + rayCast(
//        //            mPosition,
//        //            float3(fOffX, -fOffY, -mDistToFilm),
//        //            fOffX,
//        //            fOffY,
//        //            true
//        //    );
//        //}
//
//        //accum /= uSamplesPerPixel;
//        float fOffX = (((p % uImageU) - (uImageU / 2))/* + distrib(mtRanEngine) - 0.5*/) * 0.01;
//        float fOffY = (((p / uImageU) - (uImageV / 2))/* + distrib(mtRanEngine) - 0.5*/) * 0.01; 
//        mImage[p] = rayCast(
//            mPosition,
//            float3(fOffX, -fOffY, -mDistToFilm),
//            0,
//            0,
//            true
//            );
//        
//        //mImage[p] = accum / uSamplesPerPixel;
//        {
//            std::unique_lock<std::mutex> lock(write_mutex);
//            if(p % 1000 == 0.f)
//                std::cout << p << " of " << uImageU * uImageV << std::endl;
//        }
//
//        //mImage[p] = float4(0.89f,0.2588f,0.204f,0.f);
//        //mImage[p] = float4(
//        //        p / float(uImageU * uImageV),
//        //p / float(uImageU * uImageV),
//        //        p / float(uImageU * uImageV),
//        //        0.f
//        //        );
//
//    }
//
//    std::cout << uRaysHit << " rays hit out of " << uRaysFired << " rays fired" << std::endl;
//
//	//for (auto t = 0; t < vTileSet.size(); t++)
//    //{
//    //    printf("Rendering Tile %d of %d\n",t, vTileSet.size());
//	//	RenderTile(vTileSet[t]);
//    //}
//}

void Vermilion::Camera::saveFrame(std::string name)
{
    auto outFrame = new unsigned char[uImageU * uImageV * 4];

    // Do 32f to 16u conversion
    for( uint64_t p = 0; p < uImageU * uImageV; ++p)
    {
        // CLAMP
        // SRGB transform
        
        outFrame[p * 4 + 0] = static_cast<unsigned char>(std::floor(mImage[p].x * 255));
        outFrame[p * 4 + 1] = static_cast<unsigned char>(std::floor(mImage[p].y * 255));
        outFrame[p * 4 + 2] = static_cast<unsigned char>(std::floor(mImage[p].z * 255));
        outFrame[p * 4 + 3] = static_cast<unsigned char>(std::floor(mImage[p].w * 255));
    }

    //TODO FILE OUT
    OpenImageIO::ImageSpec spec(uImageU, uImageV, 4);
    auto outBuffer = OpenImageIO::ImageBuf(spec, (void*)outFrame);
    outBuffer.write(name + ".png", "png");

    delete outFrame;
}



