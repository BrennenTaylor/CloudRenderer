#pragma once

#include <FMath/FMath.h>

#include <cstdint>

namespace Farlor
{
    namespace CBs
    {
        // Must be size 16
        struct cbGeometryDeferredPerObject
        {
            Matrix4x4 WorldMatrix;
            Matrix4x4 InvWorldMatrix;
            uint32_t MeshID;
            float _pad[3];
        };
        static_assert(sizeof(cbGeometryDeferredPerObject) % 16 == 0, "cbGeometryDeferredPerObject is not not multiple of 16");

        struct cbGeometryDeferredPerFrame
        {
            Matrix4x4 ViewMatrix;
            Matrix4x4 ProjMatrix;
            Matrix4x4 PrevViewMatrix;
            Matrix4x4 PrevProjMatrix;
        };
        static_assert(sizeof(cbGeometryDeferredPerFrame) % 16 == 0, "cbGeometryDeferredPerFrame is not not multiple of 16");


        struct cbGeometryDeferredBasicPBRMaterial
        {
            Vector3 Albedo;
            float metalness;
            float roughness;
            float _pad[3];
        };
        static_assert(sizeof(cbGeometryDeferredBasicPBRMaterial) % 16 == 0, "cbGeometryDeferredBasicPBRMaterial is not multiple of 16");

        struct cbPointLight
        {
            Vector3 position;
            float _pad0;
            Vector3 color;
            float _pad1;
        };
        static_assert(sizeof(cbPointLight) % 16 == 0, "cbPointLight is not multiple of 16");

        struct cbDirectionLight
        {
            Vector3 direction;
            float _pad0;
            Vector3 color;
            float _pad1;
        };
        static_assert(sizeof(cbDirectionLight) % 16 == 0, "cbDirectionLight is not multiple of 16");

        struct cbCamera
        {
            Vector3 position;
            float _pad[1];
        };
        static_assert(sizeof(cbCamera) % 16 == 0, "cbCamera is not multiple of 16");
            
        struct cbWet
        {
            float wetness;
            float _pad[3];
        };
        static_assert(sizeof(cbWet) % 16 == 0, "cbWet is not multiple of 16");
    }
}