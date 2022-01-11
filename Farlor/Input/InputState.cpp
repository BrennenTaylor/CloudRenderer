#include "InputState.h"

namespace Farlor
{
    UnidirectionalAbsoluteAxis::UnidirectionalAbsoluteAxis()
        : m_value(0.0f)
    {
    }

    UnidirectionalAbsoluteAxis::UnidirectionalAbsoluteAxis(float normalizedFloatValue)
        : m_value(normalizedFloatValue)
    {
    }
    
    UnidirectionalAbsoluteAxis::UnidirectionalAbsoluteAxis(float currentValue, float maxValue)
        : m_value(currentValue / maxValue)
    {
    }
    
    UnidirectionalAbsoluteAxis::UnidirectionalAbsoluteAxis(int currentValue, int maxValue)
        : m_value((float)currentValue / (float)maxValue)
    {
    }

    void UnidirectionalAbsoluteAxis::SetValueNormalized(float normalizedValue)
    {
        m_value = normalizedValue;
    }
    
    void UnidirectionalAbsoluteAxis::SetValueUnnormalized(float currentValue, float maxValue)
    {
        m_value = currentValue / maxValue;
    }
    
    void UnidirectionalAbsoluteAxis::SetValueDiscretized(int currentValue, int maxValue)
    {
        m_value = (float)currentValue / (float)maxValue;
    }

    // Wrapper around a float value, values are valid between -1.0 and 1.0
    BidirectionalAbsoluteAxis::BidirectionalAbsoluteAxis()
    {
    }

    BidirectionalAbsoluteAxis::BidirectionalAbsoluteAxis(float normalizedFloatValue)
        : m_value(normalizedFloatValue)
    {
    }

    BidirectionalAbsoluteAxis::BidirectionalAbsoluteAxis(float currentValue, float maxValue)
        : m_value(currentValue / maxValue)
    {
    }

    BidirectionalAbsoluteAxis::BidirectionalAbsoluteAxis(int currentValue, int maxValue)
        : m_value((float)currentValue / (float)maxValue)
    {
    }


    void BidirectionalAbsoluteAxis::SetValueNormalized(float normalizedValue)
    {
        m_value = normalizedValue;
    }

    void BidirectionalAbsoluteAxis::SetValueUnnormalized(float currentValue, float maxValue)
    {
        m_value = currentValue / maxValue;
    }

    void BidirectionalAbsoluteAxis::SetValueDiscretized(int currentValue, int maxValue)
    {
        m_value = (float)currentValue / (float)maxValue;
    }
}