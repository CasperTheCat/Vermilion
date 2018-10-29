////// Vermilion Mesh Engine

#include "meshEngine.h"
#include "../accelerators/triangle.h"
#include "OpenImageIO/imagebuf.h"

Vermilion::VermiTexture::VermiTexture
(
	uint16_t w,
	uint16_t h,
	uint16_t c,
	float *d
)
{
	nWidth = w;
	nHeight = h;
	nChannels = c;
	pData = d;
}

void Vermilion::VermiTexture::Sample(const glm::vec2& TexCoord, glm::vec4 *pSample)
{
	auto safeTexCoordX = TexCoord.x - floor(TexCoord.x);
	auto safeTexCoordY = TexCoord.y - floor(TexCoord.y);
	auto mappedX = (uint32_t)round(safeTexCoordX * (nWidth - 1));
	auto mappedY = (uint32_t)round(safeTexCoordY * (nHeight - 1));

	auto ptr = &pData[(mappedY * nWidth + mappedX) * nChannels];

	switch(nChannels)
	{
		case 1:
			*pSample = glm::vec4( ptr[0] );
			break;
		case 2:
			*pSample = glm::vec4( ptr[0], ptr[1], 0,0 );
			break;
		case 3:
			*pSample = glm::vec4( ptr[0], ptr[1], ptr[2] ,0 );
			break;
		case 4:
			*pSample = glm::vec4( ptr[0], ptr[1], ptr[2], ptr[3] );
			break;
		default:;
	}
}

Vermilion::MeshEngine::MeshEngine()
{
	// MeshEngine loggers go by different rules from the global
	// Namely, they always log everything regardless of global logger level
	this->logger = new LogEngine("MeshEngine.log",VermiLogBoth,VermiLogLevelAll);
	this->bUsingInternalLogger = true;
	this->sceneAccelerator = nullptr;
}

Vermilion::MeshEngine::MeshEngine(LogEngine *overrideLogger) : logger(overrideLogger)
{
	this->bUsingInternalLogger = false;
	this->sceneAccelerator = nullptr;
}


Vermilion::MeshEngine::~MeshEngine()
{
	if (this->bUsingInternalLogger) delete this->logger;
	if (this->sceneAccelerator) delete this->sceneAccelerator;
	// Delete Textures
	for(auto tex : boundTextures)
		delete[] tex.pData;
	
}

bool Vermilion::MeshEngine::bindTexture(std::string& fName)
{
	using namespace OIIO;
	ImageInput *tex = ImageInput::open(fName);
	if(!tex) return false;

	const ImageSpec &spec = tex->spec();
	uint32_t xres = spec.width;
	uint32_t yres = spec.height;
	uint32_t channels = spec.nchannels;

	auto tempPtr = new float[xres * yres * channels];
	tex->read_image(TypeDesc::FLOAT, tempPtr);
	tex->close();
	ImageInput::destroy(tex);

	// Make VermiTexture
	boundTextures.emplace_back(xres, yres, channels, tempPtr);
	return true;
}

bool Vermilion::MeshEngine::load(std::string& fName)
{
	// Use some default flags
	// NOTE: aiProcess_Triangulate is VERY important here. OptiX Prime is only capable of handling tri's
	//int flags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_FlipUVs;// | aiProcess_GenSmoothNormals;

	//int flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs;

	int flags = 
		//aiProcess_FlipWindingOrder | 
		//aiProcess_CalcTangentSpace |
		aiProcess_PreTransformVertices |
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_GenSmoothNormals |
		//aiProcess_JoinIdenticalVertices |
		//aiProcess_ImproveCacheLocality |
		//aiProcess_LimitBoneWeights |
		//aiProcess_RemoveRedundantMaterials |
		//aiProcess_SplitLargeMeshes |
//		aiProcess_GenUVCoords |
		//aiProcess_SortByPType |
		//aiProcess_FindDegenerates |
		//aiProcess_FindInvalidData |
		//aiProcess_FindInstances |
		//aiProcess_ValidateDataStructure |
		//aiProcess_OptimizeMeshes |
		0;

	return load(fName, flags);
}

bool Vermilion::MeshEngine::load(std::string& fName, int flags)
{
	// Check for the file's existance!
	std::ifstream fCheck(fName.c_str());

	if (!fCheck.is_open())
	{
		// Handle no file
		logger->logWarn( "File " + fName + " not found" );
		return false;
	}
	fCheck.close();

	// Read file
	this->pScene = aiImporter.ReadFile(fName.c_str(), flags);
	
	if (!pScene)
	{
		logger->logWarn( aiImporter.GetErrorString() );
		return false;
	}

	return processScene();
}

inline float triangleIntersect(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &v0,const glm::vec3 &v1,const glm::vec3 &v2)
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

inline float sphereIntersect(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &p, const float rad)
{
		glm::vec3 op = p - pos; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        double t;
		double eps = 1e-10;
		double b = glm::dot(op, rot);
		double det = b * b - glm::dot(op, op) + rad * rad;
        if (det < 0)
            return 0;
        else
            det = sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
}

bool Vermilion::MeshEngine::RayCastCollision(const glm::vec3& rayStart, const glm::vec3& rayDirection)
{
	Ray r(rayStart,rayDirection);
	IntersectionInfo ii{};

	auto tw = sceneAccelerator->getIntersection(r, &ii, false);
	if(tw && ii.t > 1e-3) 
	{
		//printf("%f\n", ii.t);
		return tw;
	}
	return false;


	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
	uint32_t temp;

	for (uint32_t x = 0; x < sceneMeshes.size(); ++x)
	{
		aiMesh *mesh = sceneMeshes[x];
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
		{
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

			if(triangleIntersect(rayStart, rayDirection, v0, v1, v2) > 0.01f) return true;
		}
	}
	return false;
}

bool Vermilion::MeshEngine::RayCast(const glm::vec3& rayStart, const glm::vec3& rayDirection,
	aiMaterial** ppImpactMaterial, glm::vec3 *pHitLocation, glm::vec3 *pHitNormal, float *pHitDistance, glm::vec2 *pHitTexCoord, glm::vec3 *pHitColour)
{
	// Doesn't use BVH.

	if(pHitTexCoord)
		*pHitTexCoord = glm::vec3(0,0,0);
	if(pHitNormal)
		*pHitNormal = glm::vec3(0,0,0);
	if(pHitColour) 
		*pHitColour = glm::vec3(0,0,0);
	if(ppImpactMaterial)
		*ppImpactMaterial = nullptr;
	if(pHitLocation)
		*pHitLocation = glm::vec3(0,0,0);
	if(pHitDistance)
		*pHitDistance = 0.f;

	auto temp = 0;
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 v0n;
	glm::vec3 v1n;
	glm::vec3 v2n;
	glm::vec2 v0uv;
	glm::vec2 v1uv;
	glm::vec2 v2uv;

	FLOAT nearestHit = INFINITY;
	FLOAT testHit = 0.f;
	int32_t hitMeshIndex = -1;
	glm::vec3 vNorm;

	/*for (uint32_t x = 0; x < sceneMeshes.size(); ++x)
	{
		aiMesh *mesh = sceneMeshes[x];
		
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
		{
			// Get First Face Indices
			temp = mesh->mFaces[i].mIndices[0];
			v0.x = mesh->mVertices[temp].x;
			v0.y = mesh->mVertices[temp].y;
			v0.z = mesh->mVertices[temp].z;
			v0n.x = mesh->mNormals[temp].x;
			v0n.y = mesh->mNormals[temp].y;
			v0n.z = mesh->mNormals[temp].z;
			if(mesh->HasTextureCoords(0))
			{
				v0uv.x = mesh->mTextureCoords[0][temp].x;
				v0uv.y = mesh->mTextureCoords[0][temp].y;
			}

			// Get Second Face Indices
			temp = mesh->mFaces[i].mIndices[1];
			v1.x = mesh->mVertices[temp].x;
			v1.y = mesh->mVertices[temp].y;
			v1.z = mesh->mVertices[temp].z;
			v1n.x = mesh->mNormals[temp].x;
			v1n.y = mesh->mNormals[temp].y;
			v1n.z = mesh->mNormals[temp].z;
			if(mesh->HasTextureCoords(0))
			{
				v1uv.x = mesh->mTextureCoords[0][temp].x;
				v1uv.y = mesh->mTextureCoords[0][temp].y;
			}

			// Get Third Face Indices
			temp = mesh->mFaces[i].mIndices[2];
			v2.x = mesh->mVertices[temp].x;
			v2.y = mesh->mVertices[temp].y;
			v2.z = mesh->mVertices[temp].z;
			v2n.x = mesh->mNormals[temp].x;
			v2n.y = mesh->mNormals[temp].y;
			v2n.z = mesh->mNormals[temp].z;
			if(mesh->HasTextureCoords(0))
			{
				v2uv.x = mesh->mTextureCoords[0][temp].x;
				v2uv.y = mesh->mTextureCoords[0][temp].y;
			}
			
			glm::vec3 vU = v1 - v0;
			glm::vec3 vV = v2 - v0;
			vNorm.x = (vU.y * vV.z) - (vU.z * vV.y);
			vNorm.y = (vU.z * vV.x) - (vU.x * vV.z);
			vNorm.z = (vU.x * vV.y) - (vU.y * vV.x);

			if (glm::dot(rayDirection, vNorm) > 0.f)
				continue;
			
			testHit = triangleIntersect(rayStart, rayDirection, v0, v1, v2);
			if (testHit > 0.f && testHit < nearestHit) // EpsilonCheck
			{
				nearestHit = testHit;
				// Interpolate Normals
				auto impact3 = rayStart + (rayDirection * nearestHit);
				glm::vec3 f0 = v0 - impact3;
				glm::vec3 f1 = v1 - impact3;
				glm::vec3 f2 = v2 - impact3;

				auto w = glm::length(glm::cross(v0 - v1, v0 - v2));
				float w0 = glm::length(glm::cross(f1,f2)) / w;
				float w1 = glm::length(glm::cross(f2,f0)) / w;
				float w2 = glm::length(glm::cross(f0,f1)) / w;

				auto interpolatedNormal = v0n * w0 + v1n * w1 + v2n * w2;
				auto interpolatedUV = v0uv * w0 + v1uv * w1 + v2uv * w2;

				hitMeshIndex = mesh->mMaterialIndex;
				if(pHitTexCoord)
					*pHitTexCoord = interpolatedUV;
				if(pHitNormal)
					*pHitNormal = interpolatedNormal;
				//break;
			}
		}
	}*/

	Ray r(rayStart,rayDirection);
	IntersectionInfo ii{};

	if(sceneAccelerator->getIntersection(r, &ii, false))
	{
		//printf("%f\n", ii.t);
		nearestHit = ii.t;
		if(pHitNormal)
			*pHitNormal = glm::normalize(ii.object->getNormal(ii, pHitTexCoord));
		hitMeshIndex = 0;//mesh->mMaterialIndex;

	}
	

	// HardCoded Light Test
	//testHit = sphereIntersect(rayStart, rayDirection, glm::vec3(50, 110, 200), 15.f);
	testHit = sphereIntersect(rayStart, rayDirection, glm::vec3(15, 140,25), 3.5f);
	if (testHit > 0.f && testHit < nearestHit) // EpsilonCheck
	{
		nearestHit = testHit;
		// Sphere is closest, it's a light!
		if(pHitColour) 
			*pHitColour = glm::vec3(0,.5,1.0) * 10.f;
		if(pHitNormal)
			*pHitNormal = -glm::normalize(rayStart + (rayDirection * nearestHit) - glm::vec3(-55, 350,-150));

	}

	/*testHit = sphereIntersect(rayStart, rayDirection, glm::vec3(500, 500, 500), 15.f);
	if (testHit > 0.f && testHit < nearestHit) // EpsilonCheck
	{
		nearestHit = testHit;
		// Sphere is closest, it's a light!
		if(pHitColour) 
			*pHitColour = glm::vec3(1.0,1.0,1.0) * 1.f;
	}*/

	testHit = sphereIntersect(rayStart, rayDirection, glm::vec3(500, 800, 1300), 300.f);
	if (testHit > 0.f && testHit < nearestHit) // EpsilonCheck
	{
		nearestHit = testHit;
		// Sphere is closest, it's a light!
		if(pHitColour) 
			*pHitColour = glm::vec3(1.0,1.0,1.0) * 0.5f;
		if(pHitNormal)
				*pHitNormal = -glm::normalize(rayStart + (rayDirection * nearestHit) - glm::vec3(500, 800, 1300));

	}



	{
		auto sizeOfSpheres = 1e5;
		// TopBottom
		testHit = sphereIntersect(rayStart, rayDirection, glm::vec3(0,-sizeOfSpheres + 1000,0), sizeOfSpheres);
		if (testHit > 0.f && testHit < nearestHit) // EpsilonCheck
		{
			nearestHit = testHit;
			if(pHitNormal)
				*pHitNormal = -glm::normalize(rayStart + (rayDirection * nearestHit) - glm::vec3(0,-sizeOfSpheres + 1000,0));
		}

		testHit = sphereIntersect(rayStart, rayDirection, glm::vec3(0,sizeOfSpheres - 0,0), sizeOfSpheres);
		if (testHit > 0.f && testHit < nearestHit) // EpsilonCheck
		{
			nearestHit = testHit;
			if(pHitNormal)
				*pHitNormal = -glm::normalize(rayStart + (rayDirection * nearestHit) - glm::vec3(0,sizeOfSpheres - 0,0));

		}


		// LeftRight
		testHit = sphereIntersect(rayStart, rayDirection, glm::vec3(-sizeOfSpheres + 2000,0,0), sizeOfSpheres);
		if (testHit > 0.f && testHit < nearestHit) // EpsilonCheck
		{
			nearestHit = testHit;
			if(pHitNormal)
				*pHitNormal = -glm::normalize(rayStart + (rayDirection * nearestHit) - glm::vec3(-sizeOfSpheres + 2000,0,0));

		}

		testHit = sphereIntersect(rayStart, rayDirection, glm::vec3(sizeOfSpheres - 2000, 0,0), sizeOfSpheres);
		if (testHit > 0.f && testHit < nearestHit) // EpsilonCheck
		{
			nearestHit = testHit;
			if(pHitNormal)
				*pHitNormal = -glm::normalize(rayStart + (rayDirection * nearestHit) - glm::vec3(sizeOfSpheres - 2000, 0,0));

		}


		// back
		testHit = sphereIntersect(rayStart, rayDirection, glm::vec3(0, 0, -sizeOfSpheres + 2000), sizeOfSpheres);
		if (testHit > 0.f && testHit < nearestHit) // EpsilonCheck
		{
			nearestHit = testHit;
			if(pHitNormal)
				*pHitNormal = -glm::normalize(rayStart + (rayDirection * nearestHit) - glm::vec3(0, 0, -sizeOfSpheres + 2000));

		}

		testHit = sphereIntersect(rayStart, rayDirection, glm::vec3(0, 0, sizeOfSpheres - 2000), sizeOfSpheres);
		if (testHit > 0.f && testHit < nearestHit) // EpsilonCheck
		{
			nearestHit = testHit;
			if(pHitNormal)
				*pHitNormal = -glm::normalize(rayStart + (rayDirection * nearestHit) - glm::vec3(0, 0, sizeOfSpheres - 2000));

		}
	}
	// Setup returns
	if(ppImpactMaterial && hitMeshIndex >= 0)
		*ppImpactMaterial = pScene->mMaterials[hitMeshIndex];
	if(pHitLocation)
		*pHitLocation = rayStart + (rayDirection * nearestHit);
	if(pHitDistance)
		*pHitDistance = nearestHit;
	return nearestHit < INFINITY;
}


void Vermilion::MeshEngine::processUnsupported()
{
	// Report any unsupported features

	// Check animation
	if (pScene->HasAnimations()) logger->logWarn("Vermilion does not support animation at this time");

	// EVERYTHING
}


void Vermilion::MeshEngine::processReserve()
{
	// Reserve space in all of the vectors
	sceneMeshes.reserve(pScene->mNumMeshes);
	sceneVermiMeshes.reserve(pScene->mNumMeshes);

	sceneAnimations.reserve(pScene->mNumAnimations);
	sceneTextures.reserve(pScene->mNumTextures);
	sceneLights.reserve(pScene->mNumLights);
	sceneCameras.reserve(pScene->mNumCameras);
	sceneMaterials.reserve(pScene->mNumMaterials);
}

void Vermilion::MeshEngine::processSceneMeshes()
{
	aiMesh* cMesh;
	uint32_t i, j;
	uint64_t nTris= 0, nVerts = 0, nMeshes = pScene->mNumMeshes;
	aiVector3D vPos;
	aiFace vFace;
	// Load data into the buffers
	for (i = 0; i < pScene->mNumMeshes; i++)
	{
		cMesh = pScene->mMeshes[i];

		// Info
		nTris += cMesh->mNumFaces;
		nVerts += cMesh->mNumVertices;

		// Load into sceneMeshes
		sceneMeshes.push_back(cMesh);

		// Setup vMesh
		VermiMesh vMesh;
		vMesh.vertices.reserve(cMesh->mNumVertices);
		vMesh.indices.reserve(cMesh->mNumFaces * 3); // Triangles

		// setup the VermiMesh vertices
		vMesh.nVerts = cMesh->mNumVertices;
		for (j = 0; j < cMesh->mNumVertices; j++)
		{
			vPos = cMesh->mVertices[j];
			vMesh.vertices.push_back(float3(vPos.x,vPos.y,vPos.z));
		}


		// setup the VermiMesh indices
		vMesh.nTris = cMesh->mNumFaces;
		for (j = 0; j < cMesh->mNumFaces; j++)
		{
			vFace = cMesh->mFaces[j];
			vMesh.indices.push_back(vFace.mIndices[0]);
			vMesh.indices.push_back(vFace.mIndices[1]);
			vMesh.indices.push_back(vFace.mIndices[2]);
		}

		sceneVermiMeshes.push_back(vMesh);
	}

	logger->logInfo(std::string("Mesh Info: ") + std::to_string(nTris) + " Triangles (" + std::to_string(nVerts) + " Points) in " + std::to_string(nMeshes) + " mesh(es)");
}

void Vermilion::MeshEngine::processSceneAnimations()
{
	aiAnimation *cAnimation;
	uint32_t i;
	logger->logInfo("Animation Info: " + std::to_string(pScene->mNumAnimations) + " animations in the scene");
	for (i = 0; i < pScene->mNumAnimations; i++)
	{
		cAnimation = pScene->mAnimations[i];
		sceneAnimations.push_back(cAnimation);
	}
}

void Vermilion::MeshEngine::processSceneCameras()
{
	aiCamera *cCamera;
	uint32_t i;
	logger->logInfo("Camera Info: " + std::to_string(pScene->mNumCameras) + " cameras in the scene");
	for (i = 0; i < pScene->mNumCameras; i++)
	{
		// Push back each camera
		cCamera = pScene->mCameras[i];
		sceneCameras.push_back(cCamera);
	}
}

void Vermilion::MeshEngine::processSceneLights()
{
	aiLight *cLight;
	uint32_t i;
	logger->logInfo("Lighting Info: " + std::to_string(pScene->mNumLights) + " lights in the scene");
	for (i = 0; i < pScene->mNumLights; i++)
	{
		cLight = pScene->mLights[i];
		sceneLights.push_back(cLight);
	}
}

void Vermilion::MeshEngine::processSceneMaterials()
{
	aiMaterial *cMaterial;
	uint32_t i;
	logger->logInfo("Material Info: " + std::to_string(pScene->mNumMaterials) + " materials in the scene");
	for (i = 0; i < pScene->mNumMaterials; i++)
	{
		cMaterial = pScene->mMaterials[i];
		sceneMaterials.push_back(cMaterial);
	}
}

void Vermilion::MeshEngine::processSceneTextures()
{
	aiTexture *cTexture;
	uint32_t i;
	logger->logInfo("Texture Info: " + std::to_string(pScene->mNumTextures) + " textures in the scene");
	for (i = 0; i < pScene->mNumTextures; i++)
	{
		cTexture = pScene->mTextures[i];
		sceneTextures.push_back(cTexture);
	}
}




void Vermilion::MeshEngine::createBVH()
{
	/*for(VermiMesh mesh : sceneVermiMeshes)
	{
		mesh.
	}*/

	auto nFaces = 0;

	std::vector<Object *> tris;

	for (uint32_t x = 0; x < sceneMeshes.size(); ++x)
	{
		aiMesh *mesh = sceneMeshes[x];
		nFaces += mesh->mNumFaces;
		glm::vec3 v0,v1,v2;
		glm::vec3 v0n,v1n,v2n;
		glm::vec2 v0uv,v1uv,v2uv;

		for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
		{
			// Get First Face Indices
			auto temp = mesh->mFaces[i].mIndices[0];
			v0.x = mesh->mVertices[temp].x;
			v0.y = mesh->mVertices[temp].y;
			v0.z = mesh->mVertices[temp].z;
			v0n.x = mesh->mNormals[temp].x;
			v0n.y = mesh->mNormals[temp].y;
			v0n.z = mesh->mNormals[temp].z;
			if(mesh->HasTextureCoords(0))
			{
				v0uv.x = mesh->mTextureCoords[0][temp].x;
				v0uv.y = mesh->mTextureCoords[0][temp].y;
			}

			// Get Second Face Indices
			temp = mesh->mFaces[i].mIndices[1];
			v1.x = mesh->mVertices[temp].x;
			v1.y = mesh->mVertices[temp].y;
			v1.z = mesh->mVertices[temp].z;
			v1n.x = mesh->mNormals[temp].x;
			v1n.y = mesh->mNormals[temp].y;
			v1n.z = mesh->mNormals[temp].z;
			if(mesh->HasTextureCoords(0))
			{
				v1uv.x = mesh->mTextureCoords[0][temp].x;
				v1uv.y = mesh->mTextureCoords[0][temp].y;
			}

			// Get Third Face Indices
			temp = mesh->mFaces[i].mIndices[2];
			v2.x = mesh->mVertices[temp].x;
			v2.y = mesh->mVertices[temp].y;
			v2.z = mesh->mVertices[temp].z;
			v2n.x = mesh->mNormals[temp].x;
			v2n.y = mesh->mNormals[temp].y;
			v2n.z = mesh->mNormals[temp].z;
			if(mesh->HasTextureCoords(0))
			{
				v2uv.x = mesh->mTextureCoords[0][temp].x;
				v2uv.y = mesh->mTextureCoords[0][temp].y;
			}

			tris.push_back(new Triangle(
				v0,v1,v2,
				v0n,v1n,v2n,
				v0uv, v1uv, v2uv
			));
		}
	}

	printf("Faces: %d\n", nFaces);

	if(sceneAccelerator) delete sceneAccelerator;
	sceneAccelerator = new BVH(tris);
}

bool Vermilion::MeshEngine::processScene()
{
	// Don't run on an empty scene, that would be silly
	if (!pScene->HasMeshes())
	{
		logger->logError("Scene contains no geometry");
		return false;
	}

	processUnsupported();
	processReserve();

	// process

	processSceneMeshes();
	processSceneAnimations();
	processSceneTextures();
	processSceneLights();
	processSceneCameras();
	processSceneMaterials();

	createBVH();

	return true;
}


