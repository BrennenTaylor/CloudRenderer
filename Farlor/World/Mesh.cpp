#include "Mesh.h"

namespace Farlor
{
    Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indicies, uint32_t meshFlags)
        : m_vertices{ vertices }
        , m_indicies{ indicies }
        , m_meshFlags{ meshFlags }
    {
    }
}