#pragma once

#include "FixedUpdate.h"

#include <InputStateManager.h>
#include <Renderer.h>

#include <Timer.h>

#include <map>
#include <memory>
#include <optional>
#include <string>

namespace Farlor
{
    class Geometry;
    class IWindow;
    class Scene;

    class Game
    {
    public:
        Game();
        ~Game();

        bool Initialize(const std::string& resourceDir);
        int Run();

    private:
        std::optional<std::unique_ptr<Scene>> LoadSceneFarlor(const std::string& envFilename);

private:
        bool m_running;

        IWindow* m_pGameWindow;
        std::unique_ptr<InputStateManager> m_upInputStateManager;
        std::unique_ptr<Farlor::Timer> m_upGameTimer;

        std::unique_ptr<Renderer> m_upRenderer;
        std::unique_ptr<IGraphicsBackend> m_upGraphicsBackend;
        std::unique_ptr<CameraManager> m_upCameraManager;

        std::string m_resourceDir;

        uint32_t m_nextGameObjectId;
        std::map<uint32_t, std::string> m_gameObjectNameLookup;

        std::vector<std::unique_ptr<Geometry>> m_geometryCache;
    };
}
