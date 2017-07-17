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

    std::string fName = argv[1];

    // We will needs to args check TBH
    // Renderer time
    auto mEng = new MeshEngine();
    auto rEng = new RenderEngine(mEng);

    mEng->load(fName); // Get model data
    rEng->draw();
    rEng->saveFrame("output");
    

    return 0;
}
