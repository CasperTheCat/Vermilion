////// Vermilion Mesh Engine
#ifndef VERMILION_MESH_ENGINE_H
#define VERMILION_MESH_ENGINE_H

#include <vector>
/*#include "../../extern/assimp/Importer.hpp"
#include "../../extern/assimp/scene.h"
#include "../../extern/assimp/postprocess.h"
#include "../../extern/assimp/cimport.h"*/

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#include "engines/loggingEngine.h"
#include "types/types.h"
#include "camera/camera.h"
#include "common.h"

#include "accelerators/bvh.h"

namespace Vermilion
{
	struct VermiMesh
	{
		uint32_t nTris;
		uint32_t nVerts;
		std::vector<float3> vertices;
		std::vector<int> indices;
	};

	class VermiTexture
	{
		public:
		float* pData;
		uint16_t nWidth;
		uint16_t nHeight;
		uint16_t nChannels;
		uint16_t _padding;
		VermiTexture(uint16_t nWidth, uint16_t nHeight, uint16_t nChannels, float* pData);
		void Sample(const glm::vec2& TexCoord, glm::vec4 *pSample);
	};

    class MeshEngine
    {
    private:
        // Vars

		// MeshEngine's have a logger internally
		// This is able to be overridden however
		LogEngine *logger;
		bool bUsingInternalLogger;

		// ASSIMP Stuff
		Assimp::Importer aiImporter;
		const aiScene* pScene = nullptr;
		//aiVector3D sceneMin, sceneMax, sceneCenter;

	public:
		// TextureBindings
		std::vector<VermiTexture> boundTextures;
	public:
		// Mesh and Bone Data
		std::vector<aiMesh*> sceneMeshes; // Keep the bone + other available for lookup
		std::vector<VermiMesh> sceneVermiMeshes;

		// Animation Data
		std::vector<aiAnimation*> sceneAnimations;

		// Textures Data
		std::vector<aiTexture*> sceneTextures;

		// Material Data
		std::vector<aiMaterial*> sceneMaterials;

		// Light Data
		std::vector<aiLight*> sceneLights;

		// Camera Data
		std::vector<aiCamera*> sceneCameras;

		// Accelerator
		BVH *sceneAccelerator;
        
    private:
        // Methods
		bool processScene();

		// processScene subroutines
		void processUnsupported();
		void processReserve();
		void processSceneMeshes();
		void processSceneAnimations();
		void processSceneTextures();
		void processSceneLights();
		void processSceneCameras();
		void processSceneMaterials();
		void createBVH();
        
    public:
        // Methods    

	    // Constructor
	    MeshEngine();
	    MeshEngine(LogEngine *overrideLogger);
	    ~MeshEngine();

	    // Load Mesh + Overloads
	    bool load(std::string& fName);
	    bool load(std::string& fName, int flags);
		bool bindTexture(std::string& fName);
	    bool RayCast(const glm::vec3& rayStart, const glm::vec3& rayDirection, aiMaterial **ppImpactMaterial, glm::vec3 *pHitLocation, glm::vec3 *pHitNormal, float *pHitDistance, glm::vec2 *pHitTexCoord, glm::vec3 *pHitColour);
		bool RayCastCollision(const glm::vec3& rayStart, const glm::vec3& rayDirection);
    };
}

#endif // End VERMILION_MESH_ENGINE_H
