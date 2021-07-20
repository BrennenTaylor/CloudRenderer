#include "SceneGraph.h"

namespace Farlor
{
    SceneGraphNode::SceneGraphNode()
        : m_pParent{ nullptr }
        , m_children{}
        , m_meshId{ 0 }
        , m_worldTransform{}
        , m_localTransform{}
    {
    }

    SceneGraphNode::SceneGraphNode(uint32_t meshId)
        : m_pParent{ nullptr }
        , m_children{}
        , m_meshId{ meshId }
        , m_worldTransform{}
        , m_localTransform{}
    {
    }

    void SceneGraphNode::SetLocalTransform(const Transform& localTransform)
    {
        m_localTransform = localTransform;

        // Update current node and all its children
        UpdateAllTransforms();
    }

    void SceneGraphNode::UpdateAllTransforms()
    {
        UpdateTransform();

        for (auto& upChild : m_children)
        {
            upChild->UpdateAllTransforms();
        }
    }

    void SceneGraphNode::UpdateTransform()
    {
        // Update the current node based on its parents world transform
        // as well as the current nodes local transform
        if (!m_pParent)
        {
            m_worldTransform = m_localTransform;
            return;
        }
        const Transform& parentTransform = m_pParent->GetWorldTransform();
    }

    void SceneGraphNode::AddChild(std::unique_ptr<SceneGraphNode> upChildNode)
    {
        upChildNode->m_pParent = this;
        upChildNode->UpdateAllTransforms();
        m_children.push_back(std::move(upChildNode));
    }
}