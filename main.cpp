#include <iostream>
#include "core/types/types.h"
#include <typeinfo>
#include "core/engines/renderEngine.h"

using namespace Vermilion;

int main(int argc, char **argv) 
{
    if (argc < 2)
    {
        return 1;
    }

    std::string normalName;
    std::string textureName;

    if(argc > 2)
    {
        normalName = argv[2];
    }

    if(argc > 3)
    {
        textureName = argv[3];
    }

    std::string fName = argv[1];

    // We will needs to args check TBH
    // Renderer time
    auto mEng = new MeshEngine();
    auto integrator = new PathTracer();
    auto rEng = new RenderEngine(mEng);
    rEng->assignIntegrator(integrator);

    mEng->load(fName); // Get model data
    if(mEng->bindTexture(normalName))
    {
        std::cout << "Using a texture" << std::endl;
    }
    mEng->bindTexture(textureName);

    rEng->draw();
    rEng->saveFrame("output");
    

    return 0;
}
