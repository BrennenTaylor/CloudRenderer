#pragma once

#include <FMath/FMath.h>

#include <tinyxml2.h>

namespace Farlor
{
    class TinyXmlUtils
    {
    public:
        static Farlor::Vector2 ParseVector2(const tinyxml2::XMLElement& element);
        static Farlor::Vector3 ParseVector3(const tinyxml2::XMLElement& element);
        static Farlor::Vector4 ParseVector4(const tinyxml2::XMLElement& element);
    };
}