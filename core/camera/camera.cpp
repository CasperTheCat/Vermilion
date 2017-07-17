//
// Created by canderson on 5/6/16.
//

// Vermilion Headers
#include "camera.h"

// BOOST headers
#include <boost/gil/extension/dynamic_images/any_image.hpp>

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

	GenerateTileSet();
}

void Vermilion::Camera::RenderTile(frameTile& rTile)
{
	// Render Logic :O
	for (auto i = 0; i < uTileSize; i++)
		for (auto j = 0; j < uTileSize; j++)
			rTile.setColor(i, j, float4(i / float(uTileSize), i / float(uTileSize), i / float(uTileSize), 1.f));
}


void Vermilion::Camera::RenderFrame()
{
	// Make each of tiles render. This is serial for now because threading...
	for (auto t = 0; t < vTileSet.size(); t++)
		RenderTile(vTileSet[t]);
}

void Vermilion::Camera::SaveFrame(std::string name)
{
    
}

