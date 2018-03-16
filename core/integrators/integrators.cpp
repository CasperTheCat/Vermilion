//
// Created by canderson on 5/6/16.
//

#include "integrators.h"
#include <random>

void Vermilion::BruteForceTracer::Render(std::vector<Vermilion::Camera*>& cameraList, MeshEngine* mEng)
{
	// random sampler
	std::mt19937 mtRanEngine(time(0));
	std::uniform_real_distribution<float> distrib(0, 1);

	float4 accum = float4(0, 0, 0, 0);
	pixelValue newPixelCarrier;


	for (auto cam : cameraList)
	{
		aiMaterial *hitMaterial;
		glm::vec3 hitLocation;
		float hitDistance;
		glm::vec3 hitNormal;

		#pragma omp parallel for
		for (uint64_t p = 0; p < cam->RenderTargetSize; ++p)
		{
			float fOffX = (((p % cam->uImageU) - (cam->uImageU / 2.f))/* + distrib(mtRanEngine) - 0.5*/) * 0.01;
			float fOffY = (((p / cam->uImageU) - (cam->uImageV / 2.f))/* + distrib(mtRanEngine) - 0.5*/) * 0.01;

			if(mEng->RayCast(cam->mPosition, glm::vec3(fOffX, -fOffY, -cam->mDistToFilm), &hitMaterial, &hitLocation, &hitNormal, &hitDistance))
			{
				float vNDL = glm::dot(glm::vec3(0,0,1), hitNormal);

				// TEMP ASSIGN HERE
				newPixelCarrier.light = vNDL;

				for (uint32_t ma = 0; ma < hitMaterial->mNumProperties; ++ma)
				{
					if (hitMaterial->mProperties[ma]->mSemantic == aiTextureType_DIFFUSE);
				}

				accum = float4(
					0.890196078f * vNDL,
					0.258823529 * vNDL,
					0.203921569 * vNDL,
					1.0
				);
			}
			
			newPixelCarrier.pixel = p;
			newPixelCarrier.red = accum.x;
			newPixelCarrier.green = accum.y;
			newPixelCarrier.blue = accum.z;
			newPixelCarrier.alpha = accum.w;
			newPixelCarrier.depth = hitDistance;

			cam->setPixelValue(newPixelCarrier);
		}
	}
}
