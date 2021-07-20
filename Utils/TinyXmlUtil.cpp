#include "TinyXmlUtil.h"

namespace Farlor
{
    Farlor::Vector2 TinyXmlUtils::ParseVector2(const tinyxml2::XMLElement& element)
    {
        float x = element.FloatAttribute("x", 0.0f);
        float y = element.FloatAttribute("y", 0.0f);
        return Farlor::Vector2(x, y);
    }

    Farlor::Vector3 TinyXmlUtils::ParseVector3(const tinyxml2::XMLElement& element)
    {
        float x = element.FloatAttribute("x", 0.0f);
        float y = element.FloatAttribute("y", 0.0f);
        float z = element.FloatAttribute("z", 0.0f);
        return Farlor::Vector3(x, y, z);
    }
    
    Farlor::Vector4 TinyXmlUtils::ParseVector4(const tinyxml2::XMLElement& element)
    {
        float x = element.FloatAttribute("x", 0.0f);
        float y = element.FloatAttribute("y", 0.0f);
        float z = element.FloatAttribute("z", 0.0f);
        float w = element.FloatAttribute("w", 0.0f);
        return Farlor::Vector4(x, y, z, w);
    }
}