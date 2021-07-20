#include "VisibleSet.h"

namespace Farlor
{
    VisibleSet::VisibleSet()
    {
    }

    VisibleSet::~VisibleSet()
    {
    }

    void VisibleSet::AddRenderComponent(const RenderComponent& renderComponent, const Transform& transform)
    {
        VisibleEntry entry;
        entry.m_agnosticHandle = renderComponent.GetAgnosticHandle();
        entry.m_transformMatrix = transform.GetWorld();
        m_entries.push_back(entry);
    }

    const std::vector<VisibleSet::VisibleEntry>& VisibleSet::GetVisibleEntries() const
    {
        return m_entries;
    }
}