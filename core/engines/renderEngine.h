//
// Created by canderson on 5/5/16.
//

#ifndef VERMILION_RENDERENGINE_H
#define VERMILION_RENDERENGINE_H

#include "camera/camera.h"
#include "engines/meshEngine.h"
#include "integrators/integrators.h"
#include "renderer/renderer.h"
#include <thread>


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
        bool bHasInternalIntegrator;

        // Logging engine to log directly from here
        LogEngine* mLogEngine;
        bool bHasInternalLogEng;

        // Vulkan Preview Window
        class VkRenderer *mRtPreview;
        std::thread mPreviewThread;


    private:
        void Initialise();
        void CreateInternalDefaultCamera();

        void PreviewThread_RT();

    public:
        RenderEngine();
        RenderEngine(MeshEngine *mEng);
        RenderEngine(LogEngine *lEng);
        RenderEngine(MeshEngine *mEng, LogEngine *lEng);
        ~RenderEngine();

		void draw();

		void saveFrame(std::string name);

        void setPrimaryCamera();

        // Engine Assignment
        void assignEngine(MeshEngine *mEng);
        void assignEngine(LogEngine *lEng);
        void assignIntegrator(Integrator *externalIntegrator);
        void createVulkanPreview();
        void shutdownVulkanPreview();
    };
}


#endif //VERMILION_RENDERENGINE_H
