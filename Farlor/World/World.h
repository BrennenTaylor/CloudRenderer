#pragma once

#include "AreaLight.h"
#include "Mesh.h"
#include "SceneGraph.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace Farlor
{
    class World
    {
    public:
        World();

        // We can register lights, game objects, and such here
        uint32_t AddMesh(std::unique_ptr<Mesh> upMesh);
        uint32_t AddAreaLight(std::unique_ptr<AreaLight> upAreaLight);

        // This stores the hierarchical representation of the world
        SceneGraphNode* GetRootNode();

    private:
        std::unique_ptr<SceneGraphNode> m_upSceneRoot;
        std::unordered_map<uint32_t, std::unique_ptr<Mesh>> m_meshes;
        std::unordered_map<uint32_t, std::unique_ptr<AreaLight>> m_areaLights;
        uint32_t m_uid;
    };
}