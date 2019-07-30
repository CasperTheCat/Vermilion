//
// Created by canderson on 5/6/16.
//

#ifndef VERMILION_CAMERA_H
#define VERMILION_CAMERA_H

#include <cstdint>
#include <vector>
#include "types/types.h"
#include "camera/frameTile/frameTile.h"
#include <string>
#include "engines/meshEngine.h"
#include <mutex>

#include "common.h"

namespace Vermilion
{

enum class ColourChannel
{
	RED = 0,
	GREEN = 1,
	BLUE = 2,
	ALPHA = 3,

	NONE
};

enum class vermRenderMode
{
	RGB,
	RGBA,
	RGBAZ,
	Depth,
	Depth64,

	TOTAL_RENDER_MODES
};

struct cameraSettings
{
	// used for camera settings
	uint32_t imageResX;
	uint32_t imageResY;
	float3 position;
	float3 rotation;
	FLOAT fBackDistance;
	FLOAT fBackSizeX;
	FLOAT fBackSizeY;
	FLOAT horAngleOfView;
	uint32_t raysPerPixel;
	uint32_t rayMaxBounces;
	uint32_t tileSize;
	vermRenderMode renderMode;
};

struct pixelValue
{
	uint64_t pixel; // 8
	float red;
	float green; // 8
	float blue;
	float alpha; // 8
	float depth;
	float light; // 8
	uint64_t samples;
};

class Camera
{
	std::mutex write_mutex;

  public:
	// Image data
	uint32_t uImageU;
	uint32_t uImageV;

	uint64_t RenderTargetSize;
	// Used for final image post composite
	float *mImage;

	// Camera needs a location
	glm::vec3 mPosition;

	// Camera also needs a rotation
	glm::vec3 mRotation;

	// it also needs a distance to the pixel grid
	FLOAT mDistToFilm;

	FLOAT sensorSizeX;
	FLOAT sensorSizeY;

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

	//Stats
	uint64_t uRaysFired;
	uint64_t uRaysHit;

	vermRenderMode renderMode;

	/// Rays are spawned from camera origin and trace based on rotation
	/// Default camera alignment is along Z
  private:
	// Functions
	void GenerateTileSet();
	void RenderTile(frameTile &rTile);
	bool rayShadowCast(glm::vec3 pos, glm::vec3 dir);
	Vermilion::float4 rayCast(float3 start, float3 rotation, float ofx, float ofy, bool recursive);
	Vermilion::FLOAT getScaledPixel(uint64_t index, ColourChannel colourChannel);

  public:
	Camera(cameraSettings &_settings);

	~Camera();

	void setPixelValue(pixelValue &newPixelValue);
	void saveFrame(std::string name);
};
}

#endif //VERMILION_CAMERA_H
