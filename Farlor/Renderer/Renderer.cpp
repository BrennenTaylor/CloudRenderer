#include "CameraEntry.h"
#include "Renderer.h"
#include "FrameGraph.h"
#include "LightSet.h"
#include "VisibleSet.h"

#include <FEnvVar.h>

#include <WindowFactory.h>

#include <iostream>

namespace Farlor
{
    Renderer::Renderer(CameraManager& cameraManager, const std::string& resourceDir)
        : m_resourceDir{ resourceDir }
        , m_pGraphicsBackend{ nullptr }
        , m_cameraManager{ cameraManager }
        , m_agnosticGeometry{}
        , m_nextAgnosticGeometryHandle{ Geometry::s_InvalidHandle + 1 }
        , m_rendererGeomIdToBackendGeomId{}
        , m_renderComponents()
    {
    }

    Renderer::~Renderer()
    {
    }

    bool Renderer::Initialize(IWindow* pGameWindow, IGraphicsBackend* pGraphicsBackend)
    {
        m_pGraphicsBackend = pGraphicsBackend;
        m_pGraphicsBackend->Initialize(pGameWindow, m_resourceDir);
        return true;
    }

    void Renderer::RegisterGameComponent(uint32_t id, Geometry* pGeometry, Matrix4x4 worldTransform)
    {
        RenderComponent newComponent(pGeometry, worldTransform);
        m_renderComponents[id] = newComponent;
    }

    Geometry::GeometryHandle Renderer::RegisterGeometry(Geometry* pGeometry)
    {
        if (!pGeometry)
        {
            return Geometry::s_InvalidHandle;
        }

        m_agnosticGeometry.insert(std::make_pair(m_nextAgnosticGeometryHandle, pGeometry));

        // Register the agnositic mesh with the backend so it can create necessary API specific objects
        IGraphicsBackend::BackendGraphicsHandle backendHandle = m_pGraphicsBackend->RegisterGeometry(m_agnosticGeometry[m_nextAgnosticGeometryHandle]);
        if (backendHandle == IGraphicsBackend::InvalidBackendGraphicsHandle)
        {
            // TODO: Ensure this bad ce is logged
        }
        m_rendererGeomIdToBackendGeomId.insert(std::make_pair(m_nextAgnosticGeometryHandle, backendHandle));

        ++m_nextAgnosticGeometryHandle;
        return (m_nextAgnosticGeometryHandle - 1);
    }

    PointLightComponent* Renderer::RegisterPointLightGameObject(uint32_t id)
    {
        auto itr = m_pointLightComponents.find(id);
        if (itr != m_pointLightComponents.end())
        {
            // Log that the id is already registered and return the already associated transform
            return itr->second.get();
        }

        // We want to register a new render component
        std::unique_ptr<PointLightComponent> upPointLightComponent = std::make_unique<PointLightComponent>(id);
        PointLightComponent* pPointLightComponent = upPointLightComponent.get();
        m_pointLightComponents.insert(std::make_pair(id, std::move(upPointLightComponent)));
        return pPointLightComponent;
    }

    PointLightComponent* Renderer::GetPointLightComponent(uint32_t id)
    {
        auto itr = m_pointLightComponents.find(id);
        if (itr == m_pointLightComponents.end())
        {
            // Log that we did not find the id
            return nullptr;
        }

        return itr->second.get();
    }

    DirectionalLightComponent* Renderer::RegisterDirectionalLightGameObject(uint32_t id)
    {
        auto itr = m_directionalLightComponents.find(id);
        if (itr != m_directionalLightComponents.end())
        {
            // Log that the id is already registered and return the already associated transform
            return itr->second.get();
        }

        // We want to register a new render component
        std::unique_ptr<DirectionalLightComponent> upDirectionalLightComponent = std::make_unique<DirectionalLightComponent>(id);
        DirectionalLightComponent* pDirectionalLightComponent = upDirectionalLightComponent.get();
        m_directionalLightComponents.insert(std::make_pair(id, std::move(upDirectionalLightComponent)));
        return pDirectionalLightComponent;
    }

    DirectionalLightComponent* Renderer::GetDirectionalLightComponent(uint32_t id)
    {
        auto itr = m_directionalLightComponents.find(id);
        if (itr == m_directionalLightComponents.end())
        {
            // Log that we did not find the id
            return nullptr;
        }

        return itr->second.get();
    }

    // This is the actual rendering function
    // This utilizes a render frame structure
    void Renderer::RenderFrame(float deltaTime, float totalTime)
    {
        // Grab the visible set of render components
        // At this point, we want to copy out the geometry ids of the meshes to render, as well as the transforms of the meshes.
        // From this point on, the backend will treat this visible set as read only.
        VisibleSet visibleSet;
        for (const auto& renderComponent : m_renderComponents)
        {
            if (!renderComponent.second.IsVisible())
            {
                continue;
            }
            // Temporary transform
            Transform tempTransform;
            visibleSet.AddRenderComponent(renderComponent.second, tempTransform);
        }

        // Now, we must also cache the light components
        LightSet lightSet;
        for (const auto& pointLightComponent : m_pointLightComponents)
        {
            lightSet.AddPointLightComponent(*pointLightComponent.second.get());
        }

        for (const auto& directionalLightComponent : m_directionalLightComponents)
        {
            lightSet.AddDirectionalLightComponent(*directionalLightComponent.second.get());
        }

        CameraEntry currentCameraEntry;
        Camera* pCurrentCamera = m_cameraManager.GetCurrentCamera();

        currentCameraEntry.m_position = pCurrentCamera->GetWorldPosition();
        currentCameraEntry.m_target = pCurrentCamera->GetWorldTarget();
        currentCameraEntry.m_worldUp = pCurrentCamera->GetWorldUp();
        currentCameraEntry.m_fov = pCurrentCamera->GetFOV();
        currentCameraEntry.m_cameraMoved = pCurrentCamera->MovedInFrame();
        currentCameraEntry.m_view = pCurrentCamera->GetView();
        currentCameraEntry.m_proj = pCurrentCamera->GetProj();

        m_pGraphicsBackend->Render(visibleSet, lightSet, currentCameraEntry, deltaTime, totalTime);
    }

    Geometry& Renderer::GetAgnosticGeometry(Geometry::GeometryHandle handle) const
    {
        return *m_agnosticGeometry.at(handle);
    }

    IGraphicsBackend::BackendGraphicsHandle Renderer::GetBackendHandleFromAgnosticHandle(Geometry::GeometryHandle agnosticHandle) const
    {
        return m_rendererGeomIdToBackendGeomId.at(agnosticHandle);
    }
}