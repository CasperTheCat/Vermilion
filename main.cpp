#include <iostream>
#include "core/types/types.h"
#include <typeinfo>
#include "core/engines/renderEngine.h"
#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_vulkan.h"
#include "imgui/examples/imgui_impl_sdl.h"
#include "core/renderer/renderer.h"
#include <vulkan/vulkan.h>

using namespace Vermilion;

void immode(RenderEngine *rEng)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Build atlas
    unsigned char* tex_pixels = NULL;
    int tex_w, tex_h;
    io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);
    ImGui::StyleColorsDark();
	io.DisplaySize = ImVec2(550, 680);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    //ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680));//, ImGuiCond_FirstUseEver);


    bool bRun = true;
    while(bRun)
    {
		ImGui::NewFrame();
        ImGui::Begin("My First Tool", &bRun, ImGuiWindowFlags_MenuBar);
        //ImGui::Text("Hi");
        /*if(ImGui::Button("Render"))
        {
            // Render
        }
        printf("NewFrame\n");
        io.DisplaySize = ImVec2(1920, 1080);
        io.DeltaTime = 1.0f / 60.0f;
        ImGui::NewFrame();

        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::ShowDemoWindow(NULL);*/
        ImGui::End();
        ImGui::Render();
    }

    printf("DestroyContext()\n");
    ImGui::DestroyContext();
}

int main(int argc, char **argv) 
{
    auto mEng = new MeshEngine();
    auto integrator = new PathTracer();
    auto rEng = new RenderEngine(mEng);
    rEng->assignIntegrator(integrator);

    if (argc < 2)
    {
        rEng->createVulkanPreview();
    }
    else
    {
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

        // Renderer time
        mEng->load(fName); // Get model data
        if(mEng->bindTexture(normalName))
        {
            std::cout << "Using a texture" << std::endl;
        }
        mEng->bindTexture(textureName);

        rEng->draw();
        rEng->saveFrame("output");
    }
    
    delete mEng;
    delete integrator;
    delete rEng;


    return 0;
}
