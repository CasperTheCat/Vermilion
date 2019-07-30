//
// Created by canderson on 5/6/16.
//
#include <memory>
#include <cmath>
#include <random>

// Vermilion Headers
#include "camera.h"
#include "../compat.h"

// OIIO
#include "OpenImageIO/imagebuf.h"

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

Vermilion::Camera::Camera(cameraSettings &_settings)
{
	uRaysFired = 0;
	uRaysHit = 0;
	RenderTargetSize = _settings.imageResX * _settings.imageResY;
	mDistToFilm = _settings.fBackDistance;
	fAngleOfView = _settings.horAngleOfView;
	uMaxBounces = _settings.rayMaxBounces;
	mPosition = glm::vec3(_settings.position.x, _settings.position.y, _settings.position.z);
	mRotation = glm::vec3(
		-_settings.rotation.x * 3.1415926535 / 180,
		-_settings.rotation.y * 3.1415926535 / 180,
		_settings.rotation.z * 3.1415926535 / 180
		);
	uSamplesPerPixel = _settings.raysPerPixel;
	uTileSize = _settings.tileSize;
	uImageU = _settings.imageResX;
	uImageV = _settings.imageResY;
	renderMode = _settings.renderMode;

	sensorSizeX = _settings.fBackSizeX;
	sensorSizeY = _settings.fBackSizeY;

	//GenerateTileSet(); // Don't actually do this...
	// Instead have an X by Y image and just render in buckets...

	switch (renderMode)
	{
	case vermRenderMode::RGB:
		mImage = new float[RenderTargetSize * 4];
		memset(mImage, 0, RenderTargetSize * 4);
		break;
	case vermRenderMode::RGBA:
		mImage = new float[RenderTargetSize * 5];
		memset(mImage, 0, RenderTargetSize * 5);
		break;
	case vermRenderMode::RGBAZ:
		mImage = new float[RenderTargetSize * 6];
		memset(mImage, 0, RenderTargetSize * 6);
		break;
	case vermRenderMode::Depth:
		mImage = new float[RenderTargetSize];
		break;
	case vermRenderMode::Depth64:
		CXX17_FALLTHROUGH;
	case vermRenderMode::TOTAL_RENDER_MODES:
		CXX17_FALLTHROUGH;
	default:
		throw std::exception();
	}
}

Vermilion::Camera::~Camera()
{
	delete[] mImage;
}

void Vermilion::Camera::setPixelValue(pixelValue &newPixelValue)
{
	float *temp = nullptr;
	switch (renderMode)
	{
	case vermRenderMode::RGB:
		temp = mImage + newPixelValue.pixel * 4;
		temp[0] += newPixelValue.red;
		temp[1] += newPixelValue.green;
		temp[2] += newPixelValue.blue;
		temp[3] += newPixelValue.samples;
		break;
	case vermRenderMode::RGBA:
		temp = mImage + newPixelValue.pixel * 5;
		temp[0] += newPixelValue.red;
		temp[1] += newPixelValue.green;
		temp[2] += newPixelValue.blue;
		temp[3] += newPixelValue.alpha;
		temp[4] += newPixelValue.samples;
		break;
	case vermRenderMode::RGBAZ:
		temp = mImage + newPixelValue.pixel * 6;
		temp[0] += newPixelValue.red;
		temp[1] += newPixelValue.green;
		temp[2] += newPixelValue.blue;
		temp[3] += newPixelValue.alpha;
		temp[4] += newPixelValue.depth;
		temp[5] += newPixelValue.samples;
		break;
	case vermRenderMode::Depth:
		mImage[newPixelValue.pixel] = newPixelValue.depth;
		break;
	case vermRenderMode::Depth64:
		CXX17_FALLTHROUGH;
	case vermRenderMode::TOTAL_RENDER_MODES:
		CXX17_FALLTHROUGH;
	default:
		throw std::exception();
	}
}

Vermilion::FLOAT Vermilion::Camera::getScaledPixel(uint64_t index, ColourChannel colourChannel)
{
	uint32_t channel = static_cast<std::underlying_type_t<ColourChannel>>(colourChannel);

	//printf("%f / %f = %f\n", mImage[index * 5 + channel], mImage[index * 5 + 4], mImage[index * 5 + channel] / mImage[index * 5 + 4]);

	switch (renderMode)
	{
		case vermRenderMode::RGB:
			return mImage[index * 4 + channel] / mImage[index * 4 + 3];
		case vermRenderMode::RGBA:
			return mImage[index * 5 + channel] / mImage[index * 5 + 4];
		case vermRenderMode::RGBAZ:
			return mImage[index * 6 + channel] / mImage[index * 6 + 5];
		default:
			return 1.f;
	}
}

void Vermilion::Camera::saveFrame(std::string name)
{
	auto outFrame = new unsigned char[uImageU * uImageV * 4];
	float *outDepth = nullptr;
	if (renderMode == vermRenderMode::RGBAZ)
	{
		outDepth = new float[uImageU * uImageV];
	}
	if (renderMode == vermRenderMode::Depth)
	{
		outDepth = mImage;
	}

	// Do 32f to 16u conversion
	for (uint64_t p = 0; p < uImageU * uImageV; ++p)
	{
		// CLAMP
		// SRGB transform
		switch (renderMode)
		{
		case vermRenderMode::RGB:
			outFrame[p * 4 + 0] = static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::RED) * 255));
			outFrame[p * 4 + 1] = static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::GREEN) * 255));
			outFrame[p * 4 + 2] = static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::BLUE) * 255));
			outFrame[p * 4 + 3] = 1;
			break;
		case vermRenderMode::RGBA:
			outFrame[p * 4 + 0] = static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::RED) * 255));
			outFrame[p * 4 + 1] = static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::GREEN) * 255));
			outFrame[p * 4 + 2] = static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::BLUE) * 255));
			outFrame[p * 4 + 3] = 255;//static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::ALPHA) * 255));
			break;
		case vermRenderMode::RGBAZ:
			outFrame[p * 4 + 0] = 1.f;//static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::RED) * 255));
			outFrame[p * 4 + 1] = 0.5f;//static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::GREEN) * 255));
			outFrame[p * 4 + 2] = 0.f;//static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::BLUE) * 255));
			outFrame[p * 4 + 3] = 1.f;//static_cast<unsigned char>(std::floor(getScaledPixel(p, ColourChannel::ALPHA) * 255));
			outDepth[p] = mImage[p * 5 + 4];
			break;
		case vermRenderMode::Depth:
			p = RenderTargetSize; // force exit
			break;
		case vermRenderMode::Depth64:
			CXX17_FALLTHROUGH;
		case vermRenderMode::TOTAL_RENDER_MODES:
			CXX17_FALLTHROUGH;
		default:
			throw std::exception();
		}
	}

	//TODO FILE OUT
	OpenImageIO::ImageSpec spec(uImageU, uImageV, 4);
	auto outBuffer = OpenImageIO::ImageBuf(spec, (void *)outFrame);
	outBuffer.write(name + ".png", "png");

	std::cout << "FileWritten" << std::endl;

	if (renderMode == vermRenderMode::Depth || renderMode == vermRenderMode::RGBAZ)
	{
		OpenImageIO::ImageSpec dImageSpec(uImageU, uImageV, 1, OpenImageIO::TypeDesc::FLOAT);
		auto depthOutBuffer = OpenImageIO::ImageBuf(dImageSpec, (void *)outDepth);
		depthOutBuffer.write(name + "_depth.exr", ".exr");
	}

	delete[] outFrame;
	if (renderMode == vermRenderMode::RGBAZ)
		delete[] outDepth;
}
