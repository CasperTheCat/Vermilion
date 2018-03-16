//
// Created by canderson on 5/5/16.
//

#include "renderEngine.h"
#include "../camera/camera.h"
// Since this is a render engine. We will have own logger.
Vermilion::RenderEngine::RenderEngine() :
        mMeshEngine(nullptr),
        bHasMeshEng(false),
        bHasInternalLogEng(true)
{
    // The lack of a mesh engine is problem here but we can make a logger
    // Logger needs override change settings
    this->mLogEngine = new LogEngine("Renderer.log", VermiLogBoth, VermiLogLevelAll);
}

Vermilion::RenderEngine::RenderEngine(MeshEngine *mEng) :

        mMeshEngine(mEng),
        bHasMeshEng(true),
        mLogEngine(nullptr),
        bHasInternalLogEng(true)
{
    // Same as above, creats a log engine
    this->mLogEngine = new LogEngine("Renderer.log", VermiLogBoth, VermiLogLevelAll);
}

Vermilion::RenderEngine::RenderEngine(LogEngine *lEng) :
        mMeshEngine(nullptr),
        bHasMeshEng(false),
        mLogEngine(lEng),
        bHasInternalLogEng(false)
{
}

Vermilion::RenderEngine::RenderEngine(MeshEngine *mEng, LogEngine *lEng) :
    bHasMeshEng(true),
    bHasInternalLogEng(false),
    mMeshEngine(mEng),
    mLogEngine(lEng)
{
}

Vermilion::RenderEngine::~RenderEngine()
{
    // The only data we have to remove currently is the camera and the log engine if it is internal
    if (bHasInternalLogEng) delete mLogEngine;
	for (uint32_t i = 0; i < mCameras.size(); ++i)
		if (mCameras[i]) delete mCameras[i];
}

void Vermilion::RenderEngine::assignEngine(LogEngine *lEng)
{
    if (bHasInternalLogEng)
    {
        delete mLogEngine;
        bHasInternalLogEng = false;
    }
    mLogEngine = lEng;
}

void Vermilion::RenderEngine::assignEngine(MeshEngine *mEng)
{
    bHasMeshEng = true;
    mMeshEngine = mEng;
}

void Vermilion::RenderEngine::draw()
{
    // Do we have a meshing engine?
    if(!bHasMeshEng)
    {
        mLogEngine->logError("Renderer called without mesh engine");
        return;
    }

	if(!mIntegrator)
	{
		mIntegrator = new BruteForceTracer();
	}

    // Do we have a camera?
    if(mCameras.empty())
    {
        mLogEngine->logWarn("Renderer has no camera... Defaulting");
        // Default vars
        Vermilion::cameraSettings vcs;
        vcs.imageResX = 1920;
        vcs.imageResY = 1080;
        vcs.position = float3(0,90,500);
        vcs.rotation = float3(0,0,0);
        vcs.fBackDistance = 10.f;
        vcs.horAngleOfView = 90.f;
        vcs.raysPerPixel = 5;
        vcs.rayMaxBounces = 5;
        vcs.tileSize = 16;
		vcs.renderMode = vermRenderMode::RGBAZ;
        // Create a default camera
		mCameras.push_back(new Vermilion::Camera(vcs));
    }

    // This probably needs the scene to do the raycasts :P
    //mCamera->renderFrame();
	mIntegrator->Render(mCameras, mMeshEngine);

}

void Vermilion::RenderEngine::saveFrame(std::string name)
{
    for(auto cam : mCameras)
        cam->saveFrame(name);
}


