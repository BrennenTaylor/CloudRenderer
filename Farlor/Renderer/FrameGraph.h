#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Farlor
{
    class FrameGraph
    {
    public:
        struct TextureInfo
        {
            enum class TextureSizeMode
            {
                Scaled,
                Pixel
            };

            TextureSizeMode m_texSizeMode;

            // Stores either a scale factor if in Scaled mode
            // Or number of pixels in Pixel Mode
            float m_width;
            float m_height;
        };

        struct BufferInfo
        {
            uint32_t m_numElements;
        };

        struct FrameGraphPass
        {
            std::string m_name;

            FrameGraphPass(std::string passName)
                : m_name(passName)
            {
            }
        };

    public:
        FrameGraph();
        ~FrameGraph();

        // This performs validation of the frame graph, as well as the culling of nodes from the graph
        bool Compile();

        // This method actually exectutes the frame graph on the GPU
        // This should be the only location where the GPU backened is interacted with!
        bool Execute();

        // Adds the pass to the frame graph, and returns a reference to the pass so that the client code can set the pass up
        // as needed.
        FrameGraphPass* AddPass(std::string passName);

    private:
        std::vector<std::unique_ptr<FrameGraph::FrameGraphPass>> m_passes;
    };
}