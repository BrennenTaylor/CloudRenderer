#pragma once

#include "IGraphicsBackend.h"

#include "Geometry.h"
#include "RenderComponent.h"
#include "PointLightComponent.h"
#include "DirectionalLightComponent.h"

#include "CameraManager.h"
#include "Transform.h"

#include <functional>
#include <memory>
#include <map>
#include <string>

namespace Farlor
{
    // Responsible for managing high level rendering tasks, defining rendering passes, and exposing high level rendering functionality
    class Renderer
    {
    public:
        explicit Renderer(CameraManager& cameraManager, const std::string& resourceDir);
        ~Renderer();

        bool Initialize(IWindow* pGameWindow, IGraphicsBackend* pGraphicsBackend);
        void RegisterGameComponent(uint32_t id, Geometry* pGeometry, Matrix4x4 worldTransform);

        PointLightComponent* RegisterPointLightGameObject(uint32_t id);
        PointLightComponent* GetPointLightComponent(uint32_t id);

        DirectionalLightComponent* RegisterDirectionalLightGameObject(uint32_t id);
        DirectionalLightComponent* GetDirectionalLightComponent(uint32_t id);

        void RenderFrame(float deltaTime, float totalTime);

        Geometry::GeometryHandle RegisterGeometry(Geometry* pGeometry);

        Geometry& GetAgnosticGeometry(Geometry::GeometryHandle handle) const;
        IGraphicsBackend::BackendGraphicsHandle GetBackendHandleFromAgnosticHandle(Geometry::GeometryHandle agnosticHandle) const;

    private:
        std::string m_resourceDir;
        IGraphicsBackend* m_pGraphicsBackend;

        CameraManager& m_cameraManager;

        // The renderer is resonsible for managing agnostic meshes and textures
        std::map<Geometry::GeometryHandle, Geometry*> m_agnosticGeometry;
        Geometry::GeometryHandle m_nextAgnosticGeometryHandle;
        std::map<Geometry::GeometryHandle, IGraphicsBackend::BackendGraphicsHandle> m_rendererGeomIdToBackendGeomId;

        std::unordered_map<uint32_t, RenderComponent> m_renderComponents;
        std::unordered_map<uint32_t, std::unique_ptr<PointLightComponent>> m_pointLightComponents;
        std::unordered_map<uint32_t, std::unique_ptr<DirectionalLightComponent>> m_directionalLightComponents;
    };
}