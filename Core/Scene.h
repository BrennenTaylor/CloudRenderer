#pragma once

#include <Transform.h>

#include <memory>
#include <vector>

namespace Farlor
{
    class Geometry;

    // A scene file is made of a ton of nodes
    // Basically, there is a root node, and all other nodes are under this root in a tree
    class Scene
    {
    public:
        class Node
        {
        public:
            enum NodeFlags
            {
                HasGeometry = 0x1,
                IsLightSource = 0x2
            };

        public:
            Node(Scene* pScene, uint32_t id);

            void AddChild(std::shared_ptr<Node> spNode);
            void RemoveChild(std::shared_ptr<Node> spNode);

            void SetTransform(const Transform &transform);
            const Transform& GetTransform() const;

            void SetNodeFlags(uint32_t nodeFlags);
            uint32_t GetNodeFlags() const;

            uint32_t GetGameObjectId();

            // Non owning
            void SetGeometry(Geometry* pGeometry);
            Geometry* GetGeometry();

        private:
            Scene* m_pScene;
            std::vector<std::shared_ptr<Node>> m_children;
            Node* m_pParent;
            uint32_t m_nodeFlags;
            Transform m_transform;
            uint32_t m_gameObjectId;

            // Optional members that are not necessarily set
            // Check flags for these
            Geometry* m_pGeometry;
        };

    public:
        Scene();

        const std::vector<uint32_t>& GameObjects() const;
        std::shared_ptr<Node> GetRoot();

    private:
        std::shared_ptr<Node> m_spRoot;
        std::vector<uint32_t> m_gameObjectIds;
    };
}