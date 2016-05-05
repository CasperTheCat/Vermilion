//
// Created by canderson on 5/5/16.
//

#include "renderEngine.h"

// Since this is a render engine. We will have own logger.
Vermilion::RenderEngine::RenderEngine() :
        bHasMeshEng(false),
        bHasInternalLogEng(true),
        mCamera(nullptr),
        mMeshEngine(nullptr)
{
    // The lack of a mesh engine is problem here but we can make a logger
    // Logger needs override change settings
    this->mLogEngine = new LogEngine("Renderer.log", VermiLogBoth, VermiLogLevelAll);
}

Vermilion::RenderEngine::RenderEngine(MeshEngine *mEng) :
        bHasMeshEng(true),
        bHasInternalLogEng(true),
        mCamera(nullptr),
        mLogEngine(nullptr),
        mMeshEngine(mEng)
{
    // Same as above, creats a log engine
    this->mLogEngine = new LogEngine("Renderer.log", VermiLogBoth, VermiLogLevelAll);
}

Vermilion::RenderEngine::RenderEngine(LogEngine *lEng) :
        bHasMeshEng(false),
        bHasInternalLogEng(false),
        mCamera(nullptr),
        mMeshEngine(nullptr),
        mLogEngine(lEng)
{
}

Vermilion::RenderEngine::RenderEngine(MeshEngine *mEng, LogEngine *lEng) :
        bHasMeshEng(true),
        bHasInternalLogEng(false),
        mCamera(nullptr),
        mMeshEngine(mEng),
        mLogEngine(lEng),
{ }

Vermilion::RenderEngine::~RenderEngine()
{
    // The only data we have to remove currently is the camera and the log engine if it is internal
    if (bHasInternalLogEng) delete mLogEngine;
    if (mCamera) delete mCamera;
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