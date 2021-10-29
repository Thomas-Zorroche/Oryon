#include "Editor.hpp"

#include "GLFW/glfw3.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "imgui/ImGuizmo.h"

#include "imgui/IconsMaterialDesignIcons.h"
#include <imgui/MaterialDesign.inl>

#include "GLRenderer/Renderer.hpp"
#include "GLRenderer/Scene/Component.hpp"

#include <string>
#include <iostream>

#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// TEMP
#include "GLRenderer/Lighting/PointLight.hpp"
#include "GLRenderer/Lighting/DirectionalLight.hpp"
#include "Events/Input.hpp"

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
    _cameraController = std::make_shared<CameraController>();
    glrenderer::Renderer::init();

    _renderingFramebuffer = Framebuffer::createRenderingBuffer(64, 64); // used for main rendering (viewport)
    _depthFramebuffer = Framebuffer::createDepthBuffer(1024, 1024);     // used for shadow mapping

    {
        auto plan = _scene->createEntity("Base Plan");
        plan.addComponent<glrenderer::MeshComponent>(glrenderer::Mesh::createMesh(glrenderer::MeshShape::Plan));
        auto& transformPlan = plan.getComponent<glrenderer::TransformComponent>();
        transformPlan.scale *= 50;

        auto cube = _scene->createEntity("Cube");
        cube.addComponent<glrenderer::MeshComponent>(glrenderer::Mesh::createMesh(glrenderer::MeshShape::Cube));
        auto& transformCube = cube.getComponent<glrenderer::TransformComponent>();
        transformCube.location.y += 2.0f;

        // Lights
        auto entityLight = _scene->createEntity("Directional Light");
        auto& baseLight = glrenderer::BaseLight::createLight(glrenderer::LightType::Directional);
        entityLight.addComponent<glrenderer::LightComponent>(baseLight);
        entityLight.addComponent<glrenderer::LineComponent>(std::make_shared<glrenderer::Line>());
        auto& transformLight = entityLight.getComponent<glrenderer::TransformComponent>();
        transformLight.location = { 0.0, 5.7, 6.9 };
        transformLight.rotation = { 48.0, 26.2, -21.7 };

        auto dirLight = std::static_pointer_cast<glrenderer::DirectionalLight>(baseLight);
        _scene->setDirectionalLight(dirLight);

        _entitySelected = entityLight;
        onEntitySelectedChanged();
    }

}

void Editor::onUpdate()
{
    _cameraController->onUpdate();

    //New ImGui Frame
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    static bool demo = false;
    if (demo)
    {
        ImGui::ShowDemoWindow(&demo);
        glrenderer::Renderer::clear();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return;
    }

    setupDockspace();

    // Panels
    drawSettingsPanel();
    drawObjectPanel();
    drawLightPanel();
    drawMaterialPanel();
    drawViewer3DPanel();

    drawWorldOutliner();
    drawMenuBar();
    
    ImGui::End();

    renderFramebuffer();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::drawMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::BeginMenu("Mesh"))
            {
                if (ImGui::MenuItem("Plan"))
                {
                    auto& plan = _scene->createEntity("Plan");
                    plan.addComponent<glrenderer::MeshComponent>(glrenderer::Mesh::createMesh(glrenderer::MeshShape::Plan));
                }
                if (ImGui::MenuItem("Cube"))
                {
                    auto& cube = _scene->createEntity("Cube");
                    cube.addComponent<glrenderer::MeshComponent>(glrenderer::Mesh::createMesh(glrenderer::MeshShape::Cube));
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Light"))
            {
                if (ImGui::MenuItem("Point"))
                {
                    auto& pointLight = _scene->createEntity("Point Light");
                    pointLight.addComponent<glrenderer::LightComponent>(glrenderer::BaseLight::createLight(glrenderer::LightType::Point));
                    auto& transformLight = pointLight.getComponent<glrenderer::TransformComponent>();
                    transformLight.location = { 0.0, 0.0, 0.0 };
                }
                if (ImGui::MenuItem("Directional"))
                {
                    auto& pointLight = _scene->createEntity("Directional Light");
                    pointLight.addComponent<glrenderer::LightComponent>(glrenderer::BaseLight::createLight(glrenderer::LightType::Directional));
                    auto& transformLight = pointLight.getComponent<glrenderer::TransformComponent>();
                    transformLight.location = { 0.0, 0.0, 0.0 };
                    transformLight.rotation = { 1.0, 1.0, 1.0 };
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void Editor::renderFramebuffer()
{
    // TODO move this in rendering 

    // Render to depth map
    _depthFramebuffer->bind(1024, 1024);
    {
        // Set View and Projection matrices
        //glrenderer::Renderer::setCamera(lightSpaceMatrix, glm::vec3(0));

        // Clear Viewport
        glrenderer::Renderer::clear();

        // Render Scene
        _scene->onUpdate(_entitySelected, true /* DEPTH */);
    }
    _depthFramebuffer->unbind();


    // Render scene with shadow mapping
    _renderingFramebuffer->bind(_viewportWidth, _viewportHeight);
    {
        // Set View and Projection matrices
        glrenderer::Renderer::setCamera(_cameraController->getCamera());
        
        // Clear Viewport
        glrenderer::Renderer::clear();

        // Render Scene
        _scene->onUpdate(_entitySelected, false, _depthFramebuffer->getTextureId());
    }
    _renderingFramebuffer->unbind();
}

void Editor::drawWorldOutliner()
{
    if (ImGui::Begin("World Outliner"))
    {
        _scene->forEachEntity([this](glrenderer::Entity entity)
        {
            std::string& label = entity.getComponent<glrenderer::LabelComponent>().label;

            if (ImGui::Selectable((std::string(ICON_MDI_CUBE) + label).c_str(), _entitySelected == entity))
            {
                _entitySelected = entity;
                onEntitySelectedChanged();
            }
        });
    }
    ImGui::End(); // Settings
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
    }
    ImGui::End(); // Settings
}

void Editor::drawObjectPanel()
{
    if (!_entitySelected || !_entitySelected.hasComponent<glrenderer::TransformComponent>())
        return;

    if (ImGui::Begin("Object"))
    {
        // Input Text for Label Mesh
        ImGui::InputText("Label", &_bufferEntitySelectedName);
        ImGui::SameLine();
        if (ImGui::Button("Rename"))
        {
            std::string& labelEntitySelected = _entitySelected.getComponent<glrenderer::LabelComponent>().label;
            labelEntitySelected = _bufferEntitySelectedName;
            _scene->makeUniqueLabel(labelEntitySelected);
        }
        
        // Transform
        if (ImGui::TreeNode("Transform"))
        {
            glm::vec3& location = _entitySelected.getComponent<glrenderer::TransformComponent>().location;
            glm::vec3& rotation = _entitySelected.getComponent<glrenderer::TransformComponent>().rotation;
            glm::vec3& scale = _entitySelected.getComponent<glrenderer::TransformComponent>().scale;

            ImGui::DragFloat3("Location", &location[0], 0.1f);
            ImGui::DragFloat3("Rotation", &rotation[0], 0.1f);
            ImGui::DragFloat3("Scale", &scale[0], 0.01f);

            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    ImGui::End(); // Mesh
}

void Editor::drawMaterialPanel()
{
    if (!_entitySelected || !_entitySelected.hasComponent<glrenderer::MeshComponent>())
        return;

    if (ImGui::Begin("Material"))
    {
        const char* materialTypes[] = { "Flat Color" };
        static int mateiral_current = 0;
        if (ImGui::Combo("Material Type", &mateiral_current, materialTypes, IM_ARRAYSIZE(materialTypes)))
        {

        }

        auto& material = _entitySelected.getComponent<glrenderer::MeshComponent>().mesh->getMaterial();
        if (ImGui::ColorEdit3("Color", &material->getDiffuse()[0]))
        {
            material->updateDiffuse();
        }
        if (ImGui::DragFloat("Roughness", &material->getRoughness(), 0.005f, 0.0f, 1.0f))
        {
            material->updateRoughness();
        }
        ImGui::Text("Shininess: %f", material->getShininess());
    }
    ImGui::End(); // Light

}


void Editor::drawLightPanel()
{
    if (!_entitySelected || !_entitySelected.hasComponent<glrenderer::LightComponent>())
        return;

    if (ImGui::Begin("Light"))
    {
        auto light = _entitySelected.getComponent<glrenderer::LightComponent>().light;
        ImGui::ColorEdit3("Color", &light->getColor()[0]);

        ImGui::DragFloat("intensity", &light->getIntensity(), 0.1f, 0.0f, 10.0f);
        
        glrenderer::PointLight* pointLight = light->isPointLight();
        if (pointLight)
        {
            float radius = pointLight->getRadius();
            if (ImGui::DragFloat("radius", &radius, 0.1f, 7.0f, 600.0f))
            {
                pointLight->setRadius(radius);
            }
            ImGui::Text("Linear: %f", pointLight->getLinear());
            ImGui::Text("Quadratic: %f", pointLight->getQuadratic());
        }

        if (_scene->getDirectionalLight())
        {
            ImGui::Checkbox("Soft Shadow", &_scene->getDirectionalLight()->getSoftShadow());
            ImGui::DragFloat("Size", &_scene->getDirectionalLight()->getSize(), 0.001f, 0.001f, 25.0f);
            ImGui::DragFloat("Far Plane Frustum", &_scene->getDirectionalLight()->getFarPlane(), 1.0f, 5.0f, 500.0f);
            ImGui::DragFloat("Near Plane Frustum", &_scene->getDirectionalLight()->getNearPlane(), 0.1f, -500.0f, 500.0f);
            //ImGui::DragFloat("Frustum Size", &_scene->getDirectionalLight()->getFrustumSize(), 0.1f,-200.0f, 200.0f);
            ImGui::DragFloat("Offset Position", &_scene->getDirectionalLight()->getOffsetPosition(), 0.1f, -15.0f, 15.0f);

            static int textureSizeId = 0;
            if (ImGui::Combo("##Texture Size", &(int&)textureSizeId, "1024\0 2048\0 4096\0\0"))
            {
                switch (textureSizeId)
                {
                case 0: _depthFramebuffer->resize(1024, 1024); break;
                case 1: _depthFramebuffer->resize(2048, 2048); break;
                case 2: _depthFramebuffer->resize(4096, 4096); break;
                }
            }

            float viewerWidth = ImGui::GetWindowContentRegionWidth();
            ImGui::BeginChild("DepthMap Viewer", ImVec2(viewerWidth, viewerWidth));
            ImVec2 wsize = ImGui::GetContentRegionAvail();
            ImGui::Image(
                (ImTextureID)_depthFramebuffer->getTextureId(),
                ImVec2(viewerWidth, viewerWidth),
                ImVec2(0, 1),
                ImVec2(1, 0)
            );
            ImGui::EndChild();
        }

    }
    ImGui::End(); // Light
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
            _renderingFramebuffer->resize(_viewportWidth, _viewportHeight);

            float ratio = _viewportWidth / _viewportHeight;
            auto& camera = _cameraController->getCamera();
            camera->updateAspectRatio(ratio);
        }

        ImGui::Image((ImTextureID)_renderingFramebuffer->getTextureId(), wsize, ImVec2(0, 1), ImVec2(1, 0));


        if (_entitySelected && _guizmoType != -1)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

            const glm::mat4& view = _cameraController->getCamera()->getViewMatrix();
            const glm::mat4& projection = _cameraController->getCamera()->getProjectionMatrix();

            auto& transformComponent = _entitySelected.getComponent<glrenderer::TransformComponent>();
            glm::mat4 transform = transformComponent.getModelMatrix();

            ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), 
                (ImGuizmo::OPERATION)_guizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 translation, rotation, scale;
                
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(translation),
                    glm::value_ptr(rotation), glm::value_ptr(scale));
                
                transformComponent.location = translation; 
                transformComponent.rotation = rotation;
                transformComponent.scale = scale; 
            }
        }

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

void Editor::onEntitySelectedChanged()
{
    _bufferEntitySelectedName = _entitySelected.getComponent<glrenderer::LabelComponent>().label;
}

void Editor::onEvent(Event& e)
{
    KeyEvent* keyEvent = e.isKeyEvent();
    if (keyEvent)
    {
        switch (keyEvent->getKeyCode())
        {
            case Key::Space: nextGuizmoType();
        }
        return;
    }

    MouseScrollEvent* scrollEvent = e.isScrollEvent();
    if (scrollEvent)
    {
        _cameraController->getCamera()->zoom(scrollEvent->getYOffset() * 5.0);
        return;
    }
}

void Editor::nextGuizmoType()
{
    // -1 --> TRANSLATE --> ROTATE --> SCALE --> -1
    switch (_guizmoType)
    {
        case ImGuizmo::OPERATION::TRANSLATE: _guizmoType = ImGuizmo::OPERATION::ROTATE; break;
        case ImGuizmo::OPERATION::ROTATE:    _guizmoType = ImGuizmo::OPERATION::SCALE; break;
        case ImGuizmo::OPERATION::SCALE:     _guizmoType = -1; break;
        case -1:                             _guizmoType = ImGuizmo::OPERATION::TRANSLATE; break;
    }
}



void Editor::free()
{
    //Shutdown ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glrenderer::Renderer::free();
    _renderingFramebuffer->free();
    _depthFramebuffer->free();
}

}