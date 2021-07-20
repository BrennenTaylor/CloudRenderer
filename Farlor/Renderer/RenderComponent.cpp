#include "RenderComponent.h"

namespace Farlor
{
    RenderComponent::RenderComponent()
        : m_pGeometry(nullptr)
        , m_worldTransform(Matrix4x4::s_Identity)
        , m_isVisible(false)
        , m_agnosticHandle(Geometry::s_InvalidHandle)
    {
    }

    RenderComponent::RenderComponent(Geometry* pGeometry, const Matrix4x4& transform)
        : m_pGeometry(pGeometry)
        , m_worldTransform(transform)
        , m_isVisible(true)
        , m_agnosticHandle(Geometry::s_InvalidHandle)
    {
    }

    RenderComponent::~RenderComponent()
    {
    }

    void RenderComponent::SetIsVisible(bool visible)
    {
        m_isVisible = visible;
    }
    
    bool RenderComponent::IsVisible() const
    {
        return m_isVisible;
    }
}