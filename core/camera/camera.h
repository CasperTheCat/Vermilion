//
// Created by canderson on 5/6/16.
//

#ifndef VERMILION_CAMERA_H
#define VERMILION_CAMERA_H

#include <cstdint>
#include <random>
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

    class VermiTriangle : public Object
    {
        uint32_t a;
        uint32_t b;
        uint32_t c;
        VermiTriangle(_a, _b, _c) : a(_a), b(_b), c(_c) {}
          //! All "Objects" must be able to test for intersections with rays.
        virtual bool getIntersection(const glm::vec3 ori, const glm::vec3 dir, IntersectionInfo* intersection) const = 0;

        //! Return an object normal based on an intersection
        virtual glm::vec3 getNormal(const IntersectionInfo& I) const = 0;

        //! Return a bounding box for this object
        virtual BBox getBBox() const = 0;

        //! Return the centroid for this object. (Used in BVH Sorting)
        virtual glm::vec3 getCentroid() const = 0;
    }

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

        // Random
        std::mt19937 mtRanEngine;
        std::uniform_real_distribution<float> distrib;

	    // Image data
	    uint32_t uImageU;
	    uint32_t uImageV;

        // Used for final image post composite
        glm::vec4* mImage; 

        FLOAT mFocalDistance;

        // Camera needs a location
        float3 mPosition;

        // Camera also needs a rotation
        float3 mRotation;

        // it also needs a distance to the pixel grid
        FLOAT mDistToFilm;

		// Theta
		FLOAT fAngleOfView;
        FLOAT fAngleOfViewY;

        float3 mUp;

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

    private:
        // RenderFunctionCPU
        glm::vec4 renderKernel(glm::vec3 oriInWS, glm::vec3 rayInWS);
        //bool intersectBVH(glm::vec3 rayOri, glm::vec3 rayDir, int& hitTriIdx, float& hitDistance, glm::vec3& triN, bool anyHit = false);

    public:
        Camera(cameraSettings &_settings, MeshEngine *mEng);

        ~Camera();

        void renderFrame(); // RayTracer

		void saveFrame(std::string name);

    };

}


#endif //VERMILION_CAMERA_H
