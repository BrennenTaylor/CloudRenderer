#pragma once

#include <IWindow.h>

#include "RenderParams.h"

#include "CameraEntry.h"
#include "Geometry.h"
#include "LightSet.h"
#include "VisibleSet.h"

namespace Farlor
{
    class Renderer;

    class IGraphicsBackend
    {
    public:
        using BackendGraphicsHandle = uint32_t;
        const static BackendGraphicsHandle InvalidBackendGraphicsHandle = -1;

    public:

        IGraphicsBackend(const Renderer& renderer);

        virtual void Initialize(IWindow *pWindow, std::string& resourceDir) = 0;
        virtual void Shutdown() = 0;

        // This is the only render data that is ever passed over to the backend for a render call.
        virtual void Render(const VisibleSet& visibleSet, const LightSet& lightSet, const CameraEntry& currentCameraEntry, float deltaTime, float totalTime) = 0;

        // Register a with the backend and returns a handle to that index.
        // From now on, when we need to tell the backend to render a piece of geometry, we simply hand it the index
        virtual Geometry::GeometryHandle RegisterGeometry(Geometry* geometry) = 0;

    protected:
        const Renderer& m_renderer;
    };
}