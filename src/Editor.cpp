#include "Editor.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "GLRenderer/Test.hpp"
#include "GLRenderer/Renderer.hpp"

#include <string>


namespace oryon
{

Editor::Editor()
{

}

void Editor::initialize(GLFWwindow * window)
{
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));

    _framebuffer = std::make_unique<Framebuffer>();
    glrenderer::Renderer::init();
}

void Editor::draw()
{
    // Draw 3D Scene here
    _framebuffer->bind(_viewportWidth, _viewportHeight);

    glrenderer::Renderer::clear();

    _framebuffer->unbind();


    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    //New Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool demo = false;
    if (demo)
    {
        ImGui::ShowDemoWindow(&demo);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return;
    }

    setupDockspace();

    // Draw ui panels
    drawSettingsPanel();
    drawViewer3DPanel();

    ImGui::End(); // Main Window

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::drawSettingsPanel()
{
    if (ImGui::Begin("Settings"))
    {
        static float color[3] = { 0.0f , 1.0f, 0.0f };

        if (ImGui::ColorEdit3("Color", &color[0]))
        {
            glrenderer::Renderer::setClearColor(glm::vec4(color[0], color[1], color[2], 1.0f));
        }

        static std::string str = "";
        if (ImGui::Button("GLRenderer"))
        {
            str = test::function12();
        }

        ImGui::Text(str.c_str());
    }
    ImGui::End(); // Settings
}

void Editor::drawViewer3DPanel()
{
    if (ImGui::Begin("Viewer 3D"))
    {
        ImVec2 wsize = ImGui::GetContentRegionAvail();
        if (_viewportWidth != wsize.x || _viewportHeight != wsize.y)
        {
            _viewportWidth = wsize.x;
            _viewportHeight = wsize.y;
            _framebuffer->resize(_viewportWidth, _viewportHeight);
        }

        ImGui::Image((ImTextureID)_framebuffer->getTextureId(), wsize, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
}

void Editor::setupDockspace()
{
    ImGuiIO& io = ImGui::GetIO();

    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &_dockspaceOpen, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiStyle& style = ImGui::GetStyle();
    float minWinSize = style.WindowMinSize.x;
    style.WindowMinSize.x = 370.0f;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    style.WindowMinSize.x = minWinSize;
}

void Editor::free()
{
    //Shutdown ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glrenderer::Renderer::free();
    _framebuffer->free();
}

}