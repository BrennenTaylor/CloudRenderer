#include "World.h"

namespace Farlor
{
    World::World()
        : m_upSceneRoot{ std::make_unique<SceneGraphNode>() }
        , m_meshes{}
        , m_areaLights{}
        , m_uid{ 1 }
    {
    }

    uint32_t World::AddMesh(std::unique_ptr<Mesh> upMesh)
    {
        uint32_t meshId = m_uid;
        ++m_uid;
        m_meshes.insert(std::make_pair(meshId, std::move(upMesh)));
        return meshId;
    }

    uint32_t World::AddAreaLight(std::unique_ptr<AreaLight> upAreaLight)
    {
        uint32_t areaLightId = m_uid;
        ++areaLightId;
        m_areaLights.insert(std::make_pair(areaLightId, std::move(upAreaLight)));
        return areaLightId;
    }

    SceneGraphNode* World::GetRootNode()
    {
        return m_upSceneRoot.get();
    }
}