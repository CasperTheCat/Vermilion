//
// Created by 2018
//

#include "integrators.h"
#include <random>
#include "../../extern/glm/glm/gtx/euler_angles.hpp"
#include "../../extern/glm/glm/gtx/string_cast.hpp"
#include "../../extern/glm/glm/gtc/matrix_transform.hpp"

inline float glmmax(const glm::vec3 &a)
{
	return std::max(a.x, std::max(a.y,a.z));
}

inline float glmmax(const glm::highp_vec4 &a)
{
	return std::max(a.x, std::max(a.y,a.z));
}

glm::vec4 Radiance(Vermilion::MeshEngine *mEng, glm::vec3 rStart, glm::vec3 rDir, std::mt19937 mtRanEngine)
{
	std::uniform_real_distribution<float> distrib(0, 1);
	aiMaterial *hitMaterial;
	glm::vec3 hitLocation;
	float hitDistance;
	glm::vec3 hitNormal;
	glm::vec2 hitUV;
	glm::vec4 accumColour = glm::vec4(0,0,0,-100);
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
		accumColour += accumRadiance * glm::vec4(hitColour, 0.f);
		if(depth == 0)
		{
			accumColour.w = hitDistance;
		}

		//accumColour = glm::vec4(hitNormal, 1.f);
		//return accumColour;

		if(glm::length(hitColour) > 1.f) return accumColour; // Hit a major illuminator

		if(++depth > 5 && distrib(mtRanEngine) > 0.9f || depth > 1000)
		{
			return accumColour;
		}

		// Tex Lookup
		glm::vec4 sampleColour = glm::vec4();
		if (hitMaterial && mEng->boundTextures.size() > 0)
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

			sampleColour = glm::vec4(
			1.f,
			1.f,
			1.f,
			1.0);
		}

		auto p = glmmax(sampleColour);

		/*if(++depth > 5)
		{
            if (distrib(mtRanEngine) < p && depth < 100)
                sampleColour *= (1 / p);
            else
				return accumColour;
		}*/


		accumRadiance *= sampleColour;//glm::vec4(sampleColour.x, sampleColour.y, sampleColour.z, 1.f);

		// Shading Models
		//if(true || distrib(mtRanEngine) >= 0.5)
		//if (hitMaterial->mProperties[ma]->mSemantic == aiTextureType_DIFFUSE);
		if(hitMaterial && distrib(mtRanEngine) >= 0.75)
		{
			// Perform a Specular sample
			Vermilion::FLOAT gx = sin(2 * M_PI * distrib(mtRanEngine));
        	Vermilion::FLOAT gy = sin(2 * M_PI * distrib(mtRanEngine));
        	Vermilion::FLOAT gz = sin(2 * M_PI * distrib(mtRanEngine));
	        glm::vec3 noise = glm::vec3(gx,gy,gz) * 0.04f;
			//rStart = hitLocation;
			//rDir = glm::normalize(rDir - hitNormal * 2.f * glm::dot(hitNormal, rDir)) + noise;

			rStart = hitLocation - rDir * 0.001f;
			rDir = glm::normalize(rDir - hitNormal * 2.f * glm::dot(hitNormal, rDir));


			//continue;

			/*double nc = 1;
			double nt = 1.55;
			bool into = glm::dot(hitNormal, rDir) < 0.f;
			double nnt = into ? nc / nt : nt / nc;
			double ddn = glm::dot(hitNormal, rDir);
			double cos2t = 1 - nnt * nnt * (1 - ddn * ddn);
			if(cos2t < 0.f)
			{
				rDir = glm::normalize(rDir - hitNormal * 2.f * glm::dot(hitNormal, rDir));
				continue;
			}

			glm::vec3 tdir = glm::normalize(rDir * float(nnt) - hitNormal * float((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t))));
			double a = nt - nc;
			double b = nt + nc;
			double r0 = a * a / (b*b);
			double c = 1 - (into ? -ddn : glm::dot(tdir, hitNormal));
			double rE = r0 + (1 - r0) * c * c * c * c * c;
			double tR = 1 - rE;
			double pCull = 0.25 + 0.5 * rE;
			double rP = rE / pCull;
			double tP = tR / (1 - pCull);

			if(distrib(mtRanEngine) < pCull)
			{
				accumRadiance *= rP;
				rDir = glm::normalize(rDir - hitNormal * 2.f * glm::dot(hitNormal, rDir));
			}
			else
			{
				accumRadiance *= tP;
				rDir = tdir;//glm::normalize(rDir - hitNormal * 2.f * glm::dot(hitNormal, rDir));
			}*/



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
			rStart = hitLocation - rDir * 0.001f;
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

		// Rotate the camera
		//glm::mat4 cameraTransform = glm::eulerAngleYXZ(cam->mRotation.y, cam->mRotation.x, cam->mRotation.z);
		glm::mat4 cameraTransform(1);

		// Rotations
		cameraTransform = glm::rotate(cameraTransform, cam->mRotation.y, glm::vec3(0,1,0));
		cameraTransform = glm::rotate(cameraTransform, cam->mRotation.x, glm::vec3(1,0,0));
		cameraTransform = glm::rotate(cameraTransform, cam->mRotation.z, glm::vec3(0,0,1));

		printf("%s\n", glm::to_string(cameraTransform).c_str());
		

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
					for(uint32_t sampleZ = 0; sampleZ < (cam->uSamplesPerPixel / 4); ++sampleZ)
					{
						++nTotalSamples;
	
						//float homogenousX = ((float(p % cam->uImageU) + distrib(mtRanEngine) - 0.25) / cam->uImageU) * 2 - 1;
						//float homogenousX = ((float(p % cam->uImageU) + distrib(mtRanEngine) - 0.25) / cam->uImageU) * 2 - 1;
						//float homogenousY = ((float(p / cam->uImageU) + distrib(mtRanEngine) - 0.25) / cam->uImageV) * 2 - 1;
						
						
						float homogenousX = ((float(p % cam->uImageU) / cam->uImageU) * 2 - 1) + ((distrib(mtRanEngine) - 0.5) * 0.001f);
						float homogenousY = ((float(p / cam->uImageU) / cam->uImageV) * 2 - 1) + ((distrib(mtRanEngine) - 0.5) * 0.001f);
						
	
						float cameraBackXCm = homogenousX * cam->sensorSizeX * 0.5;
						float cameraBackYCm = homogenousY * cam->sensorSizeY * 0.5;
	
						//accum += Radiance(mEng, cam->mPosition, glm::vec3(cameraBackXCm, -cameraBackYCm, -cam->mDistToFilm), mtRanEngine);// * 0.01f;
						//accum += Radiance(mEng, cam->mPosition, glm::vec3(cameraBackYCm, -cameraBackXCm, -cam->mDistToFilm), mtRanEngine);// * 0.01f;
	
						//float fOffX = (((p % cam->uImageU) - (cam->uImageU / 2.f))/* + distrib(mtRanEngine) - 0.5*/) * cam->sensorSizeX;
						//float fOffY = (((p / cam->uImageU) - (cam->uImageV / 2.f))/* + distrib(mtRanEngine) - 0.5*/) * 0.01;
	
						auto gridPane = glm::vec4(cameraBackXCm, -cameraBackYCm, -cam->mDistToFilm, 1.0f); // Flip from VK to GL space
						//printf("GRIDPANE: %s\n", glm::to_string(gridPane).c_str());		
						//printf("GRIDPANE: %s\n", glm::to_string(glm::normalize(gridPane)).c_str());			
						auto rayDirection = cameraTransform * gridPane;
						//printf("%s\n", glm::to_string(rayDirection).c_str());
						//printf("%s\n", glm::to_string(glm::normalize(rayDirection)).c_str());
						accum += Radiance(mEng, cam->mPosition, glm::normalize(glm::vec3(rayDirection) / rayDirection.w),  mtRanEngine);// * 0.01f;
					}
				}
			}

			nTotalSamples = cam->uSamplesPerPixel;

			newPixelCarrier.pixel = p;
			newPixelCarrier.red = std::max(std::min(accum.x / nTotalSamples, 1.f), 0.f); // nTotalSamples * (1 / 255.f);
			newPixelCarrier.green = std::max(std::min(accum.y / nTotalSamples, 1.f), 0.f);
			newPixelCarrier.blue = std::max(std::min(accum.z / nTotalSamples, 1.f), 0.f);
			newPixelCarrier.alpha = 1.f;//std::max(std::min(accum.w, 1.f), 0.f);
			newPixelCarrier.depth = accum.w;
			cam->setPixelValue(newPixelCarrier);
		}
	}
}
