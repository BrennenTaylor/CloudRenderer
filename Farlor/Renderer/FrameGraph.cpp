#include "FrameGraph.h"

#include <functional>

namespace Farlor
{
    namespace Renderer
    {
        FrameGraph::FrameGraph()
            : m_passes{}
        {
        }

        FrameGraph::~FrameGraph()
        {
        }

        bool FrameGraph::Compile()
        {
            return true;
        }

        bool FrameGraph::Execute()
        {
            return true;
        }

        FrameGraph::FrameGraphPass* FrameGraph::AddPass(std::string passName)
        {
            std::unique_ptr<FrameGraphPass> upNewPass = std::make_unique<FrameGraphPass>(passName);

            // Cache the raw value before moving, we want to be able to easily access the raw ptr
            // to return a non-owning access to the ptr
            FrameGraphPass* pGraphPass = upNewPass.get();
            m_passes.push_back(std::move(upNewPass));
            return pGraphPass;
        }
    }
}