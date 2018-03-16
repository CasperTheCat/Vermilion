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

	const glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, 0.f, 0.5f));
	//const float3 lightDirection_f3 = float3(-0.5f,1.f,0.5f);


	for (auto cam : cameraList)
	{


		#pragma omp parallel for
		for (uint64_t p = 0; p < cam->RenderTargetSize; ++p)
		{
			aiMaterial *hitMaterial;
			glm::vec3 hitLocation;
			float hitDistance;
			glm::vec3 hitNormal;
			float4 accum = float4(0, 0, 0, 0);
			pixelValue newPixelCarrier;
			float fOffX = (((p % cam->uImageU) - (cam->uImageU / 2.f))/* + distrib(mtRanEngine) - 0.5*/) * 0.01;
			float fOffY = (((p / cam->uImageU) - (cam->uImageV / 2.f))/* + distrib(mtRanEngine) - 0.5*/) * 0.01;

			if(mEng->RayCast(cam->mPosition, glm::vec3(fOffX, -fOffY, -cam->mDistToFilm), &hitMaterial, &hitLocation, &hitNormal, &hitDistance))
			{
				float vNDL = glm::dot(lightDirection, glm::normalize(hitNormal));

				// TEMP ASSIGN HERE
				newPixelCarrier.light = vNDL;

				for (uint32_t ma = 0; ma < hitMaterial->mNumProperties; ++ma)
				{
					if (hitMaterial->mProperties[ma]->mSemantic == aiTextureType_DIFFUSE);
				}

				// Shadow
				if(mEng->RayCast(hitLocation, lightDirection, nullptr, nullptr, nullptr, nullptr))
					vNDL = 0.f;

				accum = float4(
					0.890196078f * vNDL,
					0.258823529 * vNDL,
					0.203921569 * vNDL,
					1.0
				);
			}
			
			newPixelCarrier.pixel = p;
			newPixelCarrier.red = std::max(std::min(accum.x, 1.f), 0.f);
			newPixelCarrier.green = std::max(std::min(accum.y, 1.f), 0.f);
			newPixelCarrier.blue = std::max(std::min(accum.z, 1.f), 0.f);
			newPixelCarrier.alpha = accum.w;
			newPixelCarrier.depth = hitDistance;

			cam->setPixelValue(newPixelCarrier);
		}
	}
}
