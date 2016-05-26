//
// Created by canderson on 5/6/16.
//

#ifndef VERMILION_CAMERA_H
#define VERMILION_CAMERA_H

#include <cstdint>
#include <vector>
#include "../types/types.h"
#include "frameTile/frameTile.h"

namespace Vermilion
{

    struct cameraSettings
    {
        // used for camera settings
        float3 position;
        float3 rotation;
        FLOAT fBackDistance;
        FLOAT horAngleOfView;
        uint32_t raysPerPixel;
        uint32_t rayMaxBounces;

    };

    class Camera
    {
        // Camera needs a location
        float3 mPosition;

        // Camera also needs a rotation
        float3 mRotation;

        // it also needs a distance to the pixel grid
        FLOAT mDistToFilm;

        // Frame tileset
        std::vector<frameTile> tileset;

        // settings
        cameraSettings cSettings;


        /// Rays are spawned from camera origin and trace based on rotation
        /// Default camera alignment is along X

    public:
        Camera(cameraSettings &_settings);

        ~Camera();

        void renderFrame(); // Raytracer

    };

}


#endif //VERMILION_CAMERA_H
