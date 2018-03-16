//
// Created by canderson on 5/5/16.
//

#ifndef VERMILION_RENDERENGINE_H
#define VERMILION_RENDERENGINE_H

#include "../camera/camera.h"
#include "meshEngine.h"
#include "../integrators/integrators.h"


namespace Vermilion
{
    // This class handles scene and camera logic
    class RenderEngine
    {
        // Scene needs a render camera
        std::vector<Camera *> mCameras;

        // MeshEngine for loading meshes
        MeshEngine* mMeshEngine;
        bool bHasMeshEng;

		// Actual Code used to render
		Integrator *mIntegrator;

        // Logging engine to log directly from here
        LogEngine* mLogEngine;
        bool bHasInternalLogEng;


    public:
        RenderEngine();
        RenderEngine(MeshEngine *mEng);
        RenderEngine(LogEngine *lEng);
        RenderEngine(MeshEngine *mEng, LogEngine *lEng);
        ~RenderEngine();

		void draw();

		void saveFrame(std::string name);

        // Engine Assignment
        void assignEngine(MeshEngine *mEng);
        void assignEngine(LogEngine *lEng);
    };
}


#endif //VERMILION_RENDERENGINE_H
