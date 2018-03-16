////// Vermilion Mesh Engine

#include "meshEngine.h"

Vermilion::MeshEngine::MeshEngine()
{
	// MeshEngine loggers go by different rules from the global
	// Namely, they always log everything regardless of global logger level
	this->logger = new LogEngine("MeshEngine.log",VermiLogBoth,VermiLogLevelAll);
	this->bUsingInternalLogger = true;
}

Vermilion::MeshEngine::MeshEngine(LogEngine *overrideLogger) : logger(overrideLogger)
{
	this->bUsingInternalLogger = false;
}


Vermilion::MeshEngine::~MeshEngine()
{
	if (this->bUsingInternalLogger) delete this->logger;
}



bool Vermilion::MeshEngine::load(std::string& fName)
{
	// Use some default flags
	// NOTE: aiProcess_Triangulate is VERY important here. OptiX Prime is only capable of handling tri's
	//int flags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_FlipUVs;// | aiProcess_GenSmoothNormals;

	//int flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs;

	int flags = aiProcess_FlipWindingOrder | 
		aiProcess_CalcTangentSpace |
		aiProcess_PreTransformVertices |
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		//aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |
		//aiProcess_ImproveCacheLocality |
		//aiProcess_LimitBoneWeights |
		//aiProcess_RemoveRedundantMaterials |
		//aiProcess_SplitLargeMeshes |
		//aiProcess_GenUVCoords |
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

	if (fCheck.fail())
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

bool Vermilion::MeshEngine::RayCast(const glm::vec3& rayStart, const glm::vec3& rayDirection,
	aiMaterial** ppImpactMaterial, glm::vec3 *pHitLocation, glm::vec3 *pHitNormal, float *pHitDistance)
{
	// Doesn't use BVH.

	auto temp = 0;
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;

	FLOAT nearestHit = INFINITY;
	FLOAT testHit = 0.f;
	uint32_t hitMeshIndex = 0;

	for (uint32_t x = 0; x < sceneMeshes.size(); ++x)
	{
		aiMesh *mesh = sceneMeshes[x];
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

			glm::vec3 vU = v1 - v0;
			glm::vec3 vV = v2 - v0;
			glm::vec3 vNorm;
			vNorm.x = (vU.y * vV.z) - (vU.z * vV.y);
			vNorm.y = (vU.z * vV.x) - (vU.x * vV.z);
			vNorm.z = (vU.x * vV.y) - (vU.y * vV.x);

			//if(recursive)
			vNorm = vNorm * glm::vec3(-1, -1, -1);

			if (glm::dot(rayDirection, vNorm) > 0.f)
				continue;
			
			testHit = triangleIntersect(rayStart, rayDirection, v0, v1, v2);
			if (testHit > 0.01f && testHit < nearestHit) // EpsilonCheck
			{
				nearestHit = testHit;
				hitMeshIndex = mesh->mMaterialIndex;
				if(pHitNormal)
					*pHitNormal = vNorm;
			}
		}
	}


	// Setup returns
	if(ppImpactMaterial)
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
	for(VermiMesh mesh : sceneVermiMeshes)
	{
		
	}
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


