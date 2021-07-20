#include "IGraphicsBackend.h"

namespace Farlor
{
    IGraphicsBackend::IGraphicsBackend(const Renderer& renderer)
        : m_renderer{renderer}
    {
    }
}