#pragma once

#include "Mesh.h"
#include "Transform.h"

#include <memory>
#include <vector>

namespace Farlor
{
    class SceneGraphNode
    {
    public:
        explicit SceneGraphNode();
        explicit SceneGraphNode(uint32_t meshId);

        void SetLocalTransform(const Transform& localTransform);

        void UpdateTransform();
        void UpdateAllTransforms();

        void AddChild(std::unique_ptr<SceneGraphNode> upChildNode);

        inline const Transform& GetLocalTransform()
        {
            return m_localTransform;
        }

        inline const Transform& GetWorldTransform()
        {
            return m_worldTransform;
        }

        inline uint32_t GetMesh()
        {
            return m_meshId;
        }

        // TODO: Might not need to ever update a mesh...? Probably want for hot reloading though...
        inline void SetMesh(uint32_t meshId)
        {
            m_meshId = meshId;
        }

    private:
        SceneGraphNode* m_pParent;
        std::vector<std::unique_ptr<SceneGraphNode>> m_children;
        uint32_t m_meshId;

        // World position of the node
        Transform m_worldTransform;
        // Local transformation of the current node from it's parent
        Transform m_localTransform;
    };
}