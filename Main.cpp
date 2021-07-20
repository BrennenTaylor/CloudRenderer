#include "Core/Game.h"
#include "Core/Threading/JobSystem.h"

#include "Log.h"

#include <iostream>
#include <string>
#include <vector>

#include <filesystem>

int main(int argc, char** argv)
{
    // Main game object. This should likely be a singleton.
    Farlor::Game game;

    const std::filesystem::path currentPath = std::filesystem::current_path();
    const std::filesystem::path resourceDir = currentPath / "assets";

    const uint32_t numFibers = 1;
    const uint32_t maxNumThreads = 0;

    Farlor::FarlorJobs::JobSystem jobSystem(numFibers, maxNumThreads);

    struct MainTaskArg
    {
        Farlor::Game* m_pGame;
        std::string m_resourceDir;
        Farlor::FarlorJobs::JobSystem* m_pJobSystem;
    };

    MainTaskArg mainTaskArg;
    mainTaskArg.m_pGame = &game;
    mainTaskArg.m_resourceDir = resourceDir.string();
    mainTaskArg.m_pJobSystem = nullptr;

    auto MainTask = [](void* pArg) -> void
    {
        MainTaskArg* pFuncArg = static_cast<MainTaskArg*>(pArg);
        ASSERT(pFuncArg->m_pGame->Initialize(pFuncArg->m_resourceDir), "Failed to initialize game object");
        pFuncArg->m_pGame->Run();
    };
    MainTask(&mainTaskArg);

    return 0;
}