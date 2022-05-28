#include "Editor.hpp"

#include "GLFW/glfw3.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "imgui/ImGuizmo.h"

#include "imgui/IconsMaterialDesignIcons.h"
#include <imgui/MaterialDesign.inl>

#include "GLRenderer/Scene/Component.hpp"
#include "GLRenderer/Framebuffer.hpp"
#include "GLRenderer/Renderer/RendererContext.hpp"
#include "GLRenderer/ParticleSystem.hpp"

#include <string>
#include <iostream>

#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// TEMP
#include "GLRenderer/Lighting/PointLight.hpp"
#include "GLRenderer/Lighting/DirectionalLight.hpp"
#include "GLRenderer/Properties/Render/ShadowsProperties.hpp"
#include "Events/Input.hpp"

using namespace glrenderer;

namespace oryon
{

Editor::Editor()
{

}

void Editor::Initialize(GLFWwindow* window,
    const std::shared_ptr<class glrenderer::RendererContext>& rendererContext,
    const std::shared_ptr<class glrenderer::Scene>& scene,
    const std::shared_ptr<class glrenderer::Camera>& camera)
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
    _cameraController = std::make_shared<CameraController>(camera);

    // Assign Callback
    // Scene
    SC_ImportModel = std::bind<bool>(&glrenderer::Scene::ImportModel, scene, std::placeholders::_1, std::placeholders::_2);
    SC_RenameEntity = std::bind<void>(&glrenderer::Scene::RenameEntity, scene, std::placeholders::_1, std::placeholders::_2);
    SC_CreateEntity = std::bind<glrenderer::Entity>(&glrenderer::Scene::CreateBaseEntity, scene, std::placeholders::_1);
    SC_UpdateLight = std::bind<void>(&glrenderer::Scene::UpdateLights, scene, std::placeholders::_1);
    SC_Duplicate = std::bind<glrenderer::Entity>(&glrenderer::Scene::Duplicate, scene, std::placeholders::_1);


    // RendererContext
    RC_ResizeRenderBuffer = std::bind<void>(&glrenderer::RendererContext::Resize, rendererContext, std::placeholders::_1, std::placeholders::_2);
    _renderBufferTextureID = rendererContext->GetRenderBufferTextureID();
}

void Editor::OnUpdate(std::shared_ptr<glrenderer::Scene>& scene)
{
    if (_canDuplicate)
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
        return;
    }

    setupDockspace();

    // Panels
    for (auto& panel : _panels)
    {
        panel.render();
    }

    renderObjectPanel();
    renderLightPanel();
    renderMaterialPanel();

    renderViewer3DPanel();
    renderWorldOutliner(scene);
    renderMenuBar();
    renderPerformancePanel();
    renderParticuleSystemPanel(scene);
  
    ImGui::End();
}

void Editor::Draw()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::renderMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("Import"))
            {
                if (ImGui::MenuItem("glTF"))
                {
                    static const std::string modelPath = "C:/dev/gltf-models/Sponza/Sponza.gltf";
                    _groupLabels.push_back("Sponza");
                    SC_ImportModel(modelPath, _groupLabels.size() - 1);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::BeginMenu("Mesh"))
            {
                if (ImGui::MenuItem("Plan"))
                {
                    _entitySelected = SC_CreateEntity(EBaseEntityType::Plan);
                    onEntitySelectedChanged();
                }
                if (ImGui::MenuItem("Cube"))
                {
                    _entitySelected = SC_CreateEntity(EBaseEntityType::Cube);
                    onEntitySelectedChanged();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Light"))
            {
                if (ImGui::MenuItem("Point"))
                {
                    _entitySelected = SC_CreateEntity(EBaseEntityType::PointLight);
                    onEntitySelectedChanged();
                }
                if (ImGui::MenuItem("Directional"))
                {
                    _entitySelected = SC_CreateEntity(EBaseEntityType::DirectionalLight);
                    onEntitySelectedChanged();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}


void Editor::renderParticuleSystemPanel(std::shared_ptr<glrenderer::Scene>& scene)
{
    if (ImGui::Begin("Particule System"))
    {
        if (ImGui::Button("Add"))
        {
            scene->AddParticuleSystem();
        }

        if (_particuleSystemSelectedID >= 0)
        {
            if (ImGui::Button("Remove"))
            {
                scene->RemoveParticuleSystemAtIndex(_particuleSystemSelectedID);
                _particuleSystemSelectedID = -1;
            }
        }

        ImGui::Separator();

        // List Selection
        int PSIndex = 0;
        for (const auto& particleSystem : scene->GetParticuleSystems())
        {
            if (ImGui::Selectable(particleSystem->GetName().c_str(), _particuleSystemSelectedID == PSIndex))
            {
                _particuleSystemSelectedID = PSIndex;
                _particuleSystemPanel = Panel("Particule System", { { particleSystem->GetName(), particleSystem->GetBridge() } });
            }
            ++PSIndex;
        }

        // Selected Particule System Panel
        if (_particuleSystemSelectedID >= 0)
        {
            _particuleSystemPanel.render();
        }
        
    }
    ImGui::End();
}


void Editor::renderWorldOutliner(std::shared_ptr<glrenderer::Scene>& scene)
{
    if (ImGui::Begin("World Outliner"))
    {
        scene->forEachEntity([this](glrenderer::Entity entity)
        {
            const auto& component = entity.getComponent<glrenderer::LabelComponent>();
            const std::string& label = component.label;
            //const uint32_t& groupId = component.groupId;

            if (ImGui::Selectable((std::string(ICON_MDI_CUBE) + label).c_str(), _entitySelected == entity))
            {
                _entitySelected = entity;
                onEntitySelectedChanged();
            }

        });
    }
    ImGui::End(); // World Outliner
}

void Editor::renderObjectPanel()
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
            SC_RenameEntity(_entitySelected, _bufferEntitySelectedName);
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
    ImGui::End(); // Object
}

void Editor::renderMaterialPanel()
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

void Editor::renderLightPanel()
{
    if (!_pointLightSelected)
        return;

    if (ImGui::Begin("Light"))
    {
        if (ImGui::ColorEdit3("Color", &_pointLightSelected->getColor()[0]))
        {
            _pointLightSelected->UpdateDiffuse();
            SC_UpdateLight({ _pointLightSelected });
        }

        if (ImGui::DragFloat("intensity", &_pointLightSelected->getIntensity(), 0.1f, 0.0f, 10.0f))
        {
            _pointLightSelected->UpdateIntensity();
            SC_UpdateLight({ _pointLightSelected });
        }
        
        glrenderer::PointLight* pointLight = _pointLightSelected->isPointLight();
        if (pointLight)
        {
            float radius = pointLight->getRadius();
            if (ImGui::DragFloat("radius", &radius, 0.1f, 7.0f, 600.0f))
            {
                pointLight->setRadius(radius);
                SC_UpdateLight({ _pointLightSelected });    
            }
            //ImGui::Text("Linear: %f", pointLight->getLinear());
            //ImGui::Text("Quadratic: %f", pointLight->getQuadratic());
        }

        //if (_scene->getDirectionalLight() && light->isDirectionalLight())
        //{
        //    ImGui::DragFloat("Size", &_scene->getDirectionalLight()->getSize(), 0.001f, 0.001f, 25.0f);
        //    ImGui::DragFloat("Far Plane Frustum", &_scene->getDirectionalLight()->getFarPlane(), 1.0f, 5.0f, 500.0f);
        //    ImGui::DragFloat("Near Plane Frustum", &_scene->getDirectionalLight()->getNearPlane(), 0.1f, -500.0f, 500.0f);
        //    //ImGui::DragFloat("Frustum Size", &_scene->getDirectionalLight()->getFrustumSize(), 0.1f,-200.0f, 200.0f);
        //    ImGui::DragFloat("Offset Position", &_scene->getDirectionalLight()->getOffsetPosition(), 0.1f, -15.0f, 15.0f);
        //
        //    static int textureSizeId = 0;
        //    if (ImGui::Combo("##Texture Size", &(int&)textureSizeId, "1024\0 2048\0 4096\0\0"))
        //    {
        //        switch (textureSizeId)
        //        {
        //        case 0: Renderer::getShadowMap()->resize(1024, 1024); break;
        //        case 1: Renderer::getShadowMap()->resize(2048, 2048); break;
        //        case 2: Renderer::getShadowMap()->resize(4096, 4096); break;
        //        }
        //    }
        //
        //    //static int blockerSearchSamples = 0;
        //    //if (ImGui::Combo("##Blocker Search Samples", &(int&)blockerSearchSamples, "32\0 64\0 128\0\0"))
        //    //{
        //    //    switch (blockerSearchSamples)
        //    //    {
        //    //    case 0: glrenderer::Renderer::getShadowSettings()->setBlockerSearchSamples(32); break;
        //    //    case 1: xxx->setBlockerSearchSamples(64); break;
        //    //    case 2: xxx->setBlockerSearchSamples(128); break;
        //    //    }
        //    //}
        //
        //    //static int PCFFilteringSamples = 0;
        //    //if (ImGui::Combo("##Blocker Search Samples", &(int&)PCFFilteringSamples, "32\0 64\0 128\0\0"))
        //    //{
        //    //    switch (PCFFilteringSamples)
        //    //    {
        //    //    case 0: xxx->setPCFFilteringSamples(32);  break;
        //    //    case 1: xxx->setPCFFilteringSamples(64);  break;
        //    //    case 2: xxx->setPCFFilteringSamples(128); break;
        //    //    }
        //    //}
        //
        //    float viewerWidth = ImGui::GetWindowContentRegionWidth();
        //    ImGui::BeginChild("DepthMap Viewer", ImVec2(viewerWidth, viewerWidth));
        //    ImVec2 wsize = ImGui::GetContentRegionAvail();
        //    ImGui::Image(
        //        (ImTextureID)Renderer::getShadowMap()->getTextureId(),
        //        ImVec2(viewerWidth, viewerWidth),
        //        ImVec2(0, 1),
        //        ImVec2(1, 0)
        //    );
        //    ImGui::EndChild();
        //}

    }
    ImGui::End(); // Light
}

void Editor::renderViewer3DPanel()
{
    if (ImGui::Begin("Viewer 3D"))
    {
        ImVec2 wsize = ImGui::GetContentRegionAvail();
        if (_viewportWidth != wsize.x || _viewportHeight != wsize.y)
        {
            _viewportWidth = wsize.x;
            _viewportHeight = wsize.y;

            RC_ResizeRenderBuffer(_viewportWidth, _viewportHeight);

            float ratio = _viewportWidth / _viewportHeight;
            auto& camera = _cameraController->getCamera();
            camera->updateAspectRatio(ratio);
        }

        ImGui::Image((ImTextureID)_renderBufferTextureID, wsize, ImVec2(0, 1), ImVec2(1, 0));

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

            if (&_canDuplicate && !Input::isKeyPressed(Key::LeftAlt))
            {
                _canDuplicate = true;
            }
        
            if (ImGuizmo::IsUsing())
            {
                if (_canDuplicate && Input::isKeyPressed(Key::LeftAlt) && _guizmoType == ImGuizmo::OPERATION::TRANSLATE)
                {
                    _canDuplicate = false;

                    _entitySelected = SC_Duplicate(_entitySelected);
                    onEntitySelectedChanged();

                    transformComponent = _entitySelected.getComponent<glrenderer::TransformComponent>();
                }

                glm::vec3 translation, rotation, scale;
                
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(translation),
                    glm::value_ptr(rotation), glm::value_ptr(scale));
                
                transformComponent.location = translation; 
                transformComponent.rotation = rotation;
                transformComponent.scale = scale; 

                if (_pointLightSelected)
                {
                    _pointLightSelected->UpdateLocation(translation);
                    SC_UpdateLight({ _pointLightSelected });
                }
            }
        }

    }
    ImGui::End();
}

void Editor::renderPerformancePanel()
{
    if (ImGui::Begin("Performance"))
    {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
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

    if (_entitySelected.hasComponent<LightComponent>())
    {
        auto light = _entitySelected.getComponent<LightComponent>();
        _pointLightSelected = std::dynamic_pointer_cast<glrenderer::PointLight>(light.light);
    }
    else
    {
        _pointLightSelected = nullptr;
    }
}

void Editor::OnEvent(Event& e)
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

void Editor::Free()
{
    //Shutdown ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

}