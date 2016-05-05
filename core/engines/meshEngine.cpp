////// Vermilion Mesh Engine

#include "MeshEngine.h"

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

	return true;
}


