//
// Created by canderson on 5/6/16.
//

#ifndef VERMILION_CAMERA_H
#define VERMILION_CAMERA_H

#include <cstdint>
#include <vector>
#include "../types/types.h"
#include "frameTile/frameTile.h"
#include <string>
#include "../engines/meshEngine.h"
#include <mutex>
// GLM
#include "../../extern/glm/glm/glm.hpp"

namespace Vermilion
{

    struct cameraSettings
    {
        // used for camera settings
		uint32_t imageResX;
		uint32_t imageResY;
        float3 position;
        float3 rotation;
        FLOAT fBackDistance;
        FLOAT horAngleOfView;
        uint32_t raysPerPixel;
        uint32_t rayMaxBounces;
		uint32_t tileSize;
    };

    class Camera
    {
        std::mutex write_mutex;        

	// Image data
	uint32_t uImageU;
	uint32_t uImageV;
        // Used for final image post composite
        float4* mImage; 

        // Camera needs a location
        float3 mPosition;

        // Camera also needs a rotation
        float3 mRotation;

        // it also needs a distance to the pixel grid
        FLOAT mDistToFilm;

		// Theta
		FLOAT fAngleOfView;

		// Ray data
		uint32_t uMaxBounces;
		uint32_t uSamplesPerPixel;

		// Tiling size
		uint32_t uBucketsU;
		uint32_t uBucketsV;
		uint32_t uTileSize;
		
        // Frame tileset for the rendergrid
        std::vector<frameTile> vTileSet;

        MeshEngine *mMeshEngine;

        //Stats
        uint64_t uRaysFired;
        uint64_t uRaysHit;

        /// Rays are spawned from camera origin and trace based on rotation
        /// Default camera alignment is along X

	// Functions
	void GenerateTileSet();
	void RenderTile(frameTile &rTile);
        bool rayShadowCast(glm::vec3 pos, glm::vec3 dir);
        Vermilion::float4 rayCast(float3 start, float3 rotation, float ofx, float ofy, bool recursive);

    public:
        Camera(cameraSettings &_settings, MeshEngine *mEng);

        ~Camera();

        void renderFrame(); // Raytracer

		void saveFrame(std::string name);

    };

}


#endif //VERMILION_CAMERA_H
