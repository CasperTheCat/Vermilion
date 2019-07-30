//
// Created by canderson on 5/5/16.
//

#include "renderEngine.h"
#include "camera/camera.h"
// Since this is a render engine. We will have own logger.
Vermilion::RenderEngine::RenderEngine() :
        mMeshEngine(nullptr),
        bHasMeshEng(false),
        bHasInternalLogEng(true)
{
    // The lack of a mesh engine is problem here but we can make a logger
    // Logger needs override change settings
    this->mLogEngine = new LogEngine("Renderer.log", VermiLogBoth, VermiLogLevelAll);
    this->Initialise();
}

Vermilion::RenderEngine::RenderEngine(MeshEngine *mEng) :

        mMeshEngine(mEng),
        bHasMeshEng(true),
        mLogEngine(nullptr),
        bHasInternalLogEng(true)
{
    // Same as above, creats a log engine
    this->mLogEngine = new LogEngine("Renderer.log", VermiLogBoth, VermiLogLevelAll);
    this->Initialise();
}

Vermilion::RenderEngine::RenderEngine(LogEngine *lEng) :
        mMeshEngine(nullptr),
        bHasMeshEng(false),
        mLogEngine(lEng),
        bHasInternalLogEng(false)
{
    this->Initialise();
}

Vermilion::RenderEngine::RenderEngine(MeshEngine *mEng, LogEngine *lEng) :
    bHasMeshEng(true),
    bHasInternalLogEng(false),
    mMeshEngine(mEng),
    mLogEngine(lEng)
{
    this->Initialise();
}

void Vermilion::RenderEngine::Initialise()
{
    mIntegrator = new BruteForceTracer();
    bHasInternalIntegrator = true;
}

Vermilion::RenderEngine::~RenderEngine()
{
    // FIRST
    // The preview handling
    // signalThread();
    //mPreviewThread.join();

    // The only data we have to remove currently is the camera and the log engine if it is internal
    if (bHasInternalLogEng) delete mLogEngine;
    if (bHasInternalIntegrator) delete mIntegrator;
	for (uint32_t i = 0; i < mCameras.size(); ++i)
		if (mCameras[i]) delete mCameras[i];
    //delete[] mCameras;
}

void Vermilion::RenderEngine::assignIntegrator(Integrator *externalIntegrator)
{
    if (bHasInternalIntegrator)
    {
        delete mIntegrator;
        bHasInternalIntegrator = false;
    }
    mIntegrator = externalIntegrator;
}

void Vermilion::RenderEngine::createVulkanPreview()
{
    // Check the primary camera
    if(mCameras.empty()) CreateInternalDefaultCamera();

    // This call passes execution to the preview
    mRtPreview = new VkRenderer(mCameras[0], this);
    mRtPreview->Render(); // Short Circuit

    //mPreviewThread = std::thread(&Vermilion::RenderEngine::PreviewThread_RT, this);
}

void Vermilion::RenderEngine::shutdownVulkanPreview()
{
    // Careful, this can be called from a few places
    // Including the preview itself
    
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

void Vermilion::RenderEngine::CreateInternalDefaultCamera()
{
    if(!mCameras.empty() && mCameras[0] != nullptr) return;
    // Default vars
    Vermilion::cameraSettings vcs;
    vcs.imageResX = 2592/2;
    vcs.imageResY = 1728/2;
    
    //vcs.position = float3(0,0,200);
    //vcs.position = float3(-900,300,1200);
    //vcs.position = float3(0,250,800);
    //vcs.position = float3(-40,158,80);
    //vcs.position = float3(0,350,-1500);
    vcs.position = float3(-200, 80, 800);
    //vcs.position = float3(-4000, 1600, 8000);
    //vcs.position = float3(0,250,800);
    vcs.position = float3(0,0,800);
    vcs.rotation = float3(15, 180, 0);
    vcs.rotation = float3(5, 45, 0);
    vcs.rotation = float3(0, 25, 0);
    
    vcs.rotation = float3(0, 0, 0);
    vcs.fBackDistance = 3.f * 2;
    vcs.horAngleOfView = 90.f;
    vcs.raysPerPixel = 128;
    vcs.fBackSizeX = 3.6f;
    vcs.fBackSizeY = 2.4f;
    vcs.rayMaxBounces = 5;
    vcs.tileSize = 16;
	vcs.renderMode = vermRenderMode::RGBA;

    mCameras.push_back(new Vermilion::Camera(vcs));
}

void Vermilion::RenderEngine::draw()
{
    // Do we have a meshing engine?
    if(!bHasMeshEng)
    {
        mLogEngine->logError("Renderer called without mesh engine");
        return;
    }

    // Do we have a camera?
    if(mCameras.empty())
    {
        mLogEngine->logWarn("Renderer has no camera... Defaulting");
        CreateInternalDefaultCamera();
    }

    if(mIntegrator)
        mIntegrator->Render(mCameras, mMeshEngine);
}

void Vermilion::RenderEngine::draw(uint32_t nSamples)
{
        // Do we have a meshing engine?
    if(!bHasMeshEng)
    {
        mLogEngine->logError("Renderer called without mesh engine");
        return;
    }

    // Do we have a camera?
    if(mCameras.empty())
    {
        mLogEngine->logWarn("Renderer has no camera... Defaulting");
        CreateInternalDefaultCamera();
    }

    // Let the PT trace nSample rays, then return flow
    mCameras[0]->uSamplesPerPixel = nSamples;

    if(mIntegrator)
        mIntegrator->Render(mCameras, mMeshEngine);
}

void Vermilion::RenderEngine::saveFrame(std::string name)
{
    for(auto cam : mCameras)
        cam->saveFrame(name);
}


void Vermilion::RenderEngine::PreviewThread_RT()
{
    // Message passing, haha again?
    // TODO
    // Just SPIN
    while(true)
    {
        mRtPreview->Render();
    }
}