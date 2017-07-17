//
// Created by canderson on 5/6/16.
//

#include <memory>
#include <cmath>

// Vermilion Headers
#include "camera.h"

// BOOST PNG FIX
//#define png_infopp_NULL (png_infopp)NULL
//#define int_p_NULL (int *)NULL

void Vermilion::Camera::GenerateTileSet()
{
	// Get number of tiles in the dimx
	uBucketsU = uImageU / uTileSize + (1 * (uImageU % uTileSize != 0));
	uBucketsV = uImageV / uTileSize + (1 * (uImageV % uTileSize != 0));

	auto tBuckets = uBucketsU * uBucketsV;

	vTileSet.reserve(tBuckets);


	for (auto i = 0; i < uBucketsU; i++)
		for (auto j = 0; j < uBucketsV; j++)
			vTileSet.push_back(frameTile(uTileSize, uTileSize, uTileSize * i, uTileSize * j));
}


Vermilion::Camera::Camera(cameraSettings& _settings)
{
	mDistToFilm = _settings.fBackDistance;
	fAngleOfView = _settings.horAngleOfView;
	uMaxBounces = _settings.rayMaxBounces;
	mPosition = _settings.position;
	mRotation = _settings.rotation;
	uSamplesPerPixel = _settings.raysPerPixel;
	uTileSize = _settings.tileSize;
	uImageU = _settings.imageResX;
	uImageV = _settings.imageResY;

    //GenerateTileSet(); // Don't actually do this...
    // Instead have an X by Y image and just render in buckets...

    mImage = new float4[uImageU * uImageV];
    
    // Image is currently junk, zero it out?
}

Vermilion::Camera::~Camera()
{
    // TODO
    if(mImage) delete mImage;
}

void Vermilion::Camera::RenderTile(frameTile& rTile)
{
	// Render Logic :O
	for (auto i = 0; i < uTileSize; i++)
		for (auto j = 0; j < uTileSize; j++)
        {
            printf("Indexing %d,%d\n",i,j);
		    rTile.setColor(i, j, float4(0.89f,0.2588f,0.204f,1.f));
        }
}


void Vermilion::Camera::renderFrame()
{
	// Make each of tiles render. This is serial for now because threading...
    // No buckets currently
    for( uint64_t p = 0; p < uImageU * uImageV; ++p)
    {
        //mImage[p] = float4(0.89f,0.2588f,0.204f,0.f);
        mImage[p] = float4(
                p / float(uImageU * uImageV),
                p / float(uImageU * uImageV),
                p / float(uImageU * uImageV),
                0.f
                );

    }
    

	//for (auto t = 0; t < vTileSet.size(); t++)
    //{
    //    printf("Rendering Tile %d of %d\n",t, vTileSet.size());
	//	RenderTile(vTileSet[t]);
    //}
}

void Vermilion::Camera::saveFrame(std::string name)
{
    auto outFrame = new unsigned char[uImageU * uImageV * 3];

    // Do 32f to 16u conversion
    for( uint64_t p = 0; p < uImageU * uImageV; ++p)
    {
        // CLAMP
        outFrame[p * 3 + 2] = static_cast<unsigned char>(std::floor(mImage[p].x * 255));
        outFrame[p * 3 + 1] = static_cast<unsigned char>(std::floor(mImage[p].y * 255));
        outFrame[p * 3    ] = static_cast<unsigned char>(std::floor(mImage[p].z * 255));
    }

    
    //TODO FILE OUT

    delete outFrame;
}

