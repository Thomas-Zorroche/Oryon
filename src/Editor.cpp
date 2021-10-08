#include "Editor.hpp"

#include "GLFW/glfw3.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "GLRenderer/Renderer.hpp"
// TMP
#include "GLRenderer/Test.hpp"
//#include "GLRenderer/VertexBuffer.hpp"
//#include "GLRenderer/IndexBuffer.hpp"

#include <string>
#include <iostream>

#include "glm/gtc/matrix_transform.hpp"

#include "imgui/IconsMaterialDesignIcons.h"
#include <imgui/MaterialDesign.inl>

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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));

    // Icons Font
    static const ImWchar icons_ranges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphOffset.y = 1.0f;
    icons_config.OversampleH = icons_config.OversampleV = 1;
    icons_config.PixelSnapH = true;
    icons_config.SizePixels = 18.f;
    // Setup fonts                                                                                    // Font Indices
    io.FontDefault = io.Fonts->AddFontFromFileTTF("res/fonts/OpenSans/OpenSans-Regular.ttf", 16.0f);  // 1
    io.Fonts->AddFontFromMemoryCompressedTTF(MaterialDesign_compressed_data, MaterialDesign_compressed_size, 16, &icons_config, icons_ranges);

    // Initialize Renderer Data
    glrenderer::Renderer::init();
    _framebuffer = std::make_unique<Framebuffer>();
    float ratio = _viewportWidth / _viewportHeight;
    _projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 5000.0f);

    // TEMP -- Triangle Data
    {
        _shader = std::make_shared<glrenderer::Shader>("res/shaders/Default.vert", "res/shaders/Default.frag");
        //_vertexArray = std::make_shared<glrenderer::VertexArray>();

        //std::vector<float> vertices = {
        //    -0.5f, -0.5f, -3.0f,
        //     0.5f, -0.5f, -3.0f,
        //     0.0f,  0.5f, -3.0f
        //};
        //const auto& vertexBuffer = std::make_shared<glrenderer::VertexBuffer>(vertices);
        //glrenderer::BufferLayout layout = {
        //    {glrenderer::BufferAttribute()}
        //};
        //vertexBuffer->setLayout(layout);
        //_vertexArray->setVertexBuffer(vertexBuffer);

        //std::vector<uint32_t> indices = { 0, 1, 2 };
        //auto indexBuffer = std::make_shared<glrenderer::IndexBuffer>(indices, 3);
        //_vertexArray->setIndexBuffer(indexBuffer);
    }

    //std::vector<float> vertices = {
    //-0.5f, -0.5f, -3.0f,
    // 0.5f, -0.5f, -3.0f,
    // 0.0f,  0.5f, -3.0f
    //};
    //std::vector<uint32_t> indices = { 0, 1, 2 };
    ////_mesh = std::make_shared<glrenderer::Mesh>(vertices, indices);

    _mesh = glrenderer::Mesh::createMesh(glrenderer::MeshShape::Plan);
}

void Editor::draw()
{
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    //New Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool demo = true;
    if (demo)
    {
        ImGui::ShowDemoWindow(&demo);
        glrenderer::Renderer::clear();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        return;
    }

    setupDockspace();

    drawSettingsPanel();
    drawMeshPanel();
    drawWorldOutliner();
    drawViewer3DPanel();

    ImGui::End(); // Main Window

    renderFramebuffer();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::renderFramebuffer()
{
    _framebuffer->bind(_viewportWidth, _viewportHeight);
    // Draw 3D Scene here
    {
        glrenderer::Renderer::clear();

        _shader->Bind();
        _shader->SetUniformMatrix4fv("uProjectionMatrix", _projection);

        glrenderer::Renderer::draw(_mesh->getVertexArray());
    }
    _framebuffer->unbind();
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

void Editor::drawMeshPanel()
{
    if (ImGui::Begin("Mesh"))
    {
        static char label[64] = "Cube";
        ImGui::InputText("Label", label, IM_ARRAYSIZE(label));

        if (ImGui::TreeNode("Transform"))
        {
            static float location[3] = { 1.0f, 1.0f, 1.0f };
            static float rotation[3] = { 1.0f, 1.0f, 1.0f };
            static float scale[3] = { 1.0f, 1.0f, 1.0f };

            ImGui::DragFloat3("Location", location);
            ImGui::DragFloat3("Rotation", rotation);
            ImGui::DragFloat3("Scale", scale);

            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    ImGui::End(); // Settings
}

void Editor::drawWorldOutliner()
{
    if (ImGui::Begin("World Outliner"))
    {
        static int selected = -1;

        if (ImGui::Selectable(ICON_MDI_CUBE "Plan", selected == 0))
            selected = 0;
        if (ImGui::Selectable(ICON_MDI_CUBE "Plan 001", selected == 1))
            selected = 1;
        if (ImGui::Selectable(ICON_MDI_CUBE "Plan 002", selected == 2))
            selected = 2;
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

            float ratio = _viewportWidth / _viewportHeight;
            _projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 5000.0f);
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