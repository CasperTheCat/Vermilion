//
// Created by 2018
//

#include "integrators.h"
#include <random>

glm::vec4 Radiance(Vermilion::MeshEngine *mEng, glm::vec3 rStart, glm::vec3 rDir, std::mt19937 mtRanEngine)
{
	std::uniform_real_distribution<float> distrib(0, 1);
	aiMaterial *hitMaterial;
	glm::vec3 hitLocation;
	float hitDistance;
	glm::vec3 hitNormal;
	glm::vec2 hitUV;
	glm::vec4 accumColour = glm::vec4(0,0,0,0);
	glm::vec4 accumRadiance = glm::vec4(1,1,1,1);
	glm::vec3 hitColour;

	short depth = 0;			
	while(1)
	{
		if(!mEng->RayCast(rStart, rDir, &hitMaterial, &hitLocation, &hitNormal, &hitDistance, &hitUV, &hitColour))
		{
			// Ray Missed
			// Lets put the sample colour in now!
			return accumColour;// + accumRadiance * glm::dot(rDir, glm::vec3(0, 1, 0));
		}
		accumColour += accumRadiance * glm::vec4(hitColour, 1.f);


		if(++depth > 5)
		{
			return accumColour;
		}

		// Tex Lookup
		glm::vec4 sampleColour = glm::vec4();
		if (mEng->boundTextures.size() > 0)
		{
			mEng->boundTextures[0].Sample(hitUV, &sampleColour);		
		}
		else
		{
			sampleColour = glm::vec4(
			0.890196078f,
			0.258823529f,
			0.203921569f,
			1.0);
		}
		accumRadiance *= sampleColour;//glm::vec4(sampleColour.x, sampleColour.y, sampleColour.z, 1.f);

		// Shading Models
		if(distrib(mtRanEngine) >= 0.5)
		{
			// Perform a Specular sample
			Vermilion::FLOAT gx = sin(2 * M_PI * distrib(mtRanEngine));
        	Vermilion::FLOAT gy = sin(2 * M_PI * distrib(mtRanEngine));
        	Vermilion::FLOAT gz = sin(2 * M_PI * distrib(mtRanEngine));
	        glm::vec3 noise = glm::vec3(gx,gy,gz) * 0.55f;
			rStart = hitLocation;
			rDir = glm::normalize(rDir - hitNormal * 2.f * glm::dot(hitNormal, rDir)) + noise;
		}
		else
		{
			// Diffuse Hit
            float r1 = 2 * M_PI * distrib(mtRanEngine);
			float r2 = distrib(mtRanEngine);
			float r2s = sqrt(r2);

            glm::vec3 w = glm::dot(hitNormal, rDir) < 0.f ? hitNormal : hitNormal * -1.f;
			glm::vec3 u = glm::normalize(fabs(w.x) > .1 ? glm::vec3(0, 1, 0) : glm::cross(glm::vec3(1, 0, 0), w));
			glm::vec3 v = glm::cross(w, u);
            glm::vec3 d = glm::normalize(u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2));
			rStart = hitLocation;
			rDir = d;
		}
	}
}

void Vermilion::PathTracer::Render(std::vector<Vermilion::Camera*>& cameraList, MeshEngine* mEng)
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

			pixelValue newPixelCarrier;
			glm::vec4 accum = glm::vec4(0,0,0,0);

			// Number of times RNG hit
			uint8_t nRecentHits = 0;
			uint8_t nRecentMisses = 0;
			uint32_t nTotalSamples = 0;

			// Sample antialiasing
			for(uint16_t sampleX = 0; sampleX < 2; ++sampleX)
			{
				//float homogenousX = ((float(p % cam->uImageU) + distrib(mtRanEngine) - 0.25) / cam->uImageU) * 2 - 1;
				for(uint16_t sampleY = 0; sampleY < 2; ++sampleY)
				{
					for(uint16_t sampleZ = 0; sampleZ < 2; ++sampleZ)
					{
						++nTotalSamples;
	
						//float homogenousX = ((float(p % cam->uImageU) + distrib(mtRanEngine) - 0.25) / cam->uImageU) * 2 - 1;
						float homogenousX = ((float(p % cam->uImageU) + distrib(mtRanEngine) - 0.25) / cam->uImageU) * 2 - 1;
						float homogenousY = ((float(p / cam->uImageU) + distrib(mtRanEngine) - 0.25) / cam->uImageV) * 2 - 1;
						
						
						//float homogenousX = ((float(p % cam->uImageU) / cam->uImageU) * 2 - 1) + ((distrib(mtRanEngine) - 0.5) * 0.001f);
						//float homogenousY = ((float(p / cam->uImageU) / cam->uImageV) * 2 - 1) + ((distrib(mtRanEngine) - 0.5) * 0.001f);
						
	
						float cameraBackXCm = homogenousX * cam->sensorSizeX * 0.5;
						float cameraBackYCm = homogenousY * cam->sensorSizeY * 0.5;
	
						//accum += Radiance(mEng, cam->mPosition, glm::vec3(cameraBackXCm, -cameraBackYCm, -cam->mDistToFilm), mtRanEngine);// * 0.01f;
						//accum += Radiance(mEng, cam->mPosition, glm::vec3(cameraBackYCm, -cameraBackXCm, -cam->mDistToFilm), mtRanEngine);// * 0.01f;
	
						//float fOffX = (((p % cam->uImageU) - (cam->uImageU / 2.f))/* + distrib(mtRanEngine) - 0.5*/) * cam->sensorSizeX;
						//float fOffY = (((p / cam->uImageU) - (cam->uImageV / 2.f))/* + distrib(mtRanEngine) - 0.5*/) * 0.01;
	
						auto gridPane = glm::vec3(cameraBackXCm, -cameraBackYCm, -cam->mDistToFilm); // Flip from VK to GL space
						accum += Radiance(mEng, cam->mPosition, glm::normalize(gridPane), mtRanEngine);// * 0.01f;
					}
				}
			}

			newPixelCarrier.pixel = p;
			newPixelCarrier.red = std::max(std::min(accum.x / nTotalSamples, 1.f), 0.f); // nTotalSamples * (1 / 255.f);
			newPixelCarrier.green = std::max(std::min(accum.y / nTotalSamples, 1.f), 0.f);
			newPixelCarrier.blue = std::max(std::min(accum.z / nTotalSamples, 1.f), 0.f);
			newPixelCarrier.alpha = std::max(std::min(accum.w, 1.f), 0.f);
			newPixelCarrier.depth = 12.f;
			cam->setPixelValue(newPixelCarrier);
		}
	}
}
