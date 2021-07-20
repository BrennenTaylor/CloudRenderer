#include "Scene.h"

namespace Farlor
{
    // Basic node
    Scene::Node::Node(Scene* pScene, uint32_t id)
        : m_pScene(pScene)
        , m_children()
        , m_pParent(nullptr)
        , m_nodeFlags(0)
        , m_transform()
        , m_gameObjectId(id)
        , m_pGeometry(nullptr)
    {
    }

    void Scene::Node::AddChild(std::shared_ptr<Node> spNode)
    {
        spNode->m_pParent = this;
        m_children.push_back(spNode);
    }

    void Scene::Node::RemoveChild(std::shared_ptr<Node> spNode)
    {
        auto itr = std::find(m_children.begin(), m_children.end(), spNode);
        if (itr != m_children.end())
        {
            m_children.erase(itr);
        }
    }

    void Scene::Node::SetTransform(const Transform& transform)
    {
        m_transform = transform;
    }

    const Transform& Scene::Node::GetTransform() const
    {
        return m_transform;
    }

    void Scene::Node::SetNodeFlags(uint32_t nodeFlags)
    {
        m_nodeFlags = nodeFlags;
    }

    uint32_t Scene::Node::GetNodeFlags() const
    {
        return m_nodeFlags;
    }
    
    uint32_t Scene::Node::GetGameObjectId()
    {
        return m_gameObjectId;
    }

    // Non owning
    void Scene::Node::SetGeometry(Geometry *pGeometry)
    {
        m_pGeometry = pGeometry;
    }

    Geometry* Scene::Node::GetGeometry()
    {
        return m_pGeometry;
    }

    // Scene
    Scene::Scene()
        : m_spRoot(std::make_shared<Node>(this, 0))
    {
    }

    const std::vector<uint32_t>& Scene::GameObjects() const
    {
        return m_gameObjectIds;
    }

    std::shared_ptr<Scene::Node> Scene::GetRoot()
    {
        return m_spRoot;
    }
}