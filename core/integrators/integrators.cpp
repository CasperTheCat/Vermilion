//
// Created by canderson on 5/6/16.
//

#include "integrators.h"
#include <random>

void Vermilion::BruteForceTracer::Render(std::vector<Vermilion::Camera*>& cameraList, MeshEngine* mEng)
{
	// random sampler
	

	//const glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, 0.f, 0.5f));
	const glm::vec3 lightLocation = glm::vec3(50, 110, 200);
	//const glm::vec3 lightDirection2 = glm::normalize(glm::vec3(0.5f, 10.f, -0.5f));
	//const float3 lightDirection_f3 = float3(-0.5f,1.f,0.5f);


	for (auto cam : cameraList)
	{

		std::mt19937 mtRanEngine(time(0));

		#pragma omp parallel for schedule(dynamic, 1)
		for (uint64_t p = 0; p < cam->RenderTargetSize; ++p)
		{
			std::uniform_real_distribution<float> distrib(0, 0.5);

			aiMaterial *hitMaterial;
			glm::vec3 hitLocation;
			float hitDistance;
			glm::vec3 hitNormal;
			glm::vec2 hitUV;
			glm::vec4 accum = glm::vec4(0, 0, 0, 0);

			pixelValue newPixelCarrier;

			// Number of times RNG hit
			uint8_t nRecentHits = 0;
			uint8_t nRecentMisses = 0;
			uint32_t nTotalSamples = 0;

			auto lastSampleColour = glm::vec4(0,0,0,0);
			glm::vec4 currentSampleColour = glm::vec4(0,0,0,0);

			//float fOffX = (((p % cam->uImageU) - (cam->uImageU / 2.f))/* + distrib(mtRanEngine) - 0.5*/) * 0.01;
			//float fOffY = (((p / cam->uImageU) - (cam->uImageV / 2.f))/* + distrib(mtRanEngine) - 0.5*/) * 0.01;

			for(uint16_t sample = 0; sample < cam->uSamplesPerPixel; ++sample)
			{
				// 4 samples that all hit. We can exit early
				//if(nRecentHits > 3 || nRecentMisses > 3) break;
				++nTotalSamples;

				float homogenousX = ((float(p % cam->uImageU) + distrib(mtRanEngine) - 0.25) / cam->uImageU) * 2 - 1;
				float homogenousY = ((float(p / cam->uImageU) + distrib(mtRanEngine) - 0.25) / cam->uImageV) * 2 - 1;

				/*
				float homogenousX = ((float(p % cam->uImageU) / cam->uImageU) * 2 - 1) + ((distrib(mtRanEngine) - 0.5) * 0.001f);
				float homogenousY = ((float(p / cam->uImageU) / cam->uImageV) * 2 - 1) + ((distrib(mtRanEngine) - 0.5) * 0.001f);
				*/

				float cameraBackXCm = homogenousX * cam->sensorSizeX * 0.5;
				float cameraBackYCm = homogenousY * cam->sensorSizeY * 0.5;

				if(mEng->RayCast(cam->mPosition, glm::vec3(cameraBackXCm, -cameraBackYCm, -cam->mDistToFilm), &hitMaterial, &hitLocation, &hitNormal, &hitDistance, &hitUV, nullptr))
				{
					glm::vec3 lightDirectionVector = lightLocation - hitLocation;
					glm::vec3 lightDirection = glm::normalize(lightDirectionVector);

					++nRecentHits;
					nRecentMisses = 0;
					float vNDL = glm::dot(lightDirection, glm::normalize(hitNormal)) / glm::length(lightDirectionVector);

					// TEMP ASSIGN HERE
					newPixelCarrier.light = vNDL;

					for (uint32_t ma = 0; ma < hitMaterial->mNumProperties; ++ma)
					{	
						if (hitMaterial->mProperties[ma]->mSemantic == aiTextureType_DIFFUSE);
					}

					if (mEng->boundTextures.size() > 0)
					{
						auto normal = glm::vec4();
						mEng->boundTextures[0].Sample(hitUV, &normal);
						hitNormal += glm::vec3(normal.x, normal.y, normal.z);
						vNDL = glm::dot(lightDirection, glm::normalize(hitNormal));
						// auto secondNDL = glm::dot(lightDirection2, glm::normalize(hitNormal)) * 0.4f;
						// 	vNDL += secondNDL;
					}

					// Shadow
					if(mEng->RayCastCollision(hitLocation, lightDirection))
						vNDL = 0.0f;

					// Do a second bounce
					glm::vec3 specHitLocale;
					glm::vec3 specHitNormal;
					float specHitDist;
					glm::vec2 specHitUV;
					glm::vec4 resp = glm::vec4(1,1,1,1);

					/*if(mEng->RayCast(hitLocation, hitNormal, nullptr, &specHitLocale, &specHitNormal, &specHitDist, &specHitUV))
					{
						if(mEng->boundTextures.size() > 1)
						{
							auto normal = glm::vec4();
							mEng->boundTextures[1].Sample(specHitUV, &resp);
							mEng->boundTextures[0].Sample(specHitUV, &normal);
							specHitNormal += glm::vec3(normal.x, normal.y, normal.z);
							auto secondNDL = glm::dot(glm::normalize(lightLocation - specHitLocale), specHitNormal);
							resp *= secondNDL;
						}
					}
					else
					{
						vNDL *= 0.9f;
						vNDL += 0.1f;
					}*/
					
					if (mEng->boundTextures.size() > 1)
					{
						mEng->boundTextures[1].Sample(hitUV, &currentSampleColour);
						currentSampleColour *= vNDL;// * glm::vec4(resp.x, resp.y, resp.z, 1);
						currentSampleColour.a = 1.0;
						//currentSampleColour = glm::vec4(1,1,1,1) * vNDL * glm::vec4(resp.x, resp.y, resp.z ,1);
					}
					else
					{
						currentSampleColour = glm::vec4(
							0.890196078f * vNDL,
							0.258823529f * vNDL,
							0.203921569f * vNDL,
							1.0
						);
					}

					accum += currentSampleColour;
				}
				else 
				{
					nRecentHits = 0;
					++nRecentMisses;
				}
			
				// Check last after > 2 samples
				if(nTotalSamples > 2)
				{
					lastSampleColour -= (accum / glm::vec4(nTotalSamples));
					float mag = abs(lastSampleColour.x + lastSampleColour.y + lastSampleColour.z + lastSampleColour.w);
					if(mag < 0.001f) break;
				}
				lastSampleColour = (accum / glm::vec4(nTotalSamples));

			}
			newPixelCarrier.pixel = p;
			newPixelCarrier.red = std::max(std::min(accum.x / nTotalSamples, 1.f), 0.f); // nTotalSamples * (1 / 255.f);
			newPixelCarrier.green = std::max(std::min(accum.y / nTotalSamples, 1.f), 0.f);
			newPixelCarrier.blue = std::max(std::min(accum.z / nTotalSamples, 1.f), 0.f);
			newPixelCarrier.alpha = accum.w / nTotalSamples;
			newPixelCarrier.depth = hitDistance;

			cam->setPixelValue(newPixelCarrier);
		}
	}
}
