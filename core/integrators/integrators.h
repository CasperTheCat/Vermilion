//
// Created by canderson on 5/6/16.
//

#ifndef VERMILION_INTEGRATORS_H
#define VERMILION_INTEGRATORS_H
#include "../camera/camera.h"

namespace Vermilion
{
    class Integrator{
    public:
	    virtual ~Integrator() = default;

	    virtual void Render(std::vector<Vermilion::Camera *> &cameraList, MeshEngine* mEng) = 0;
    };

	class BruteForceTracer : public Integrator
	{
		virtual void Render(std::vector<Vermilion::Camera*>& cameraList, MeshEngine* mEng) override;
	};

    class PathTracer : public Integrator
    {
        virtual void Render(std::vector<Vermilion::Camera*>& cameraList, MeshEngine* mEng) override;
    };

    class BiDirectionPathTracer : public Integrator {

    };

    class Metropolis : public Integrator {

    };

    class EnergyRedist : public Integrator {

    };
}

#endif //VERMILION_INTEGRATORS_H
