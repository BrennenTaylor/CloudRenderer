#pragma once

#include <FMath/FMath.h>

#include <cstdint>

namespace Farlor
{
    namespace Renderer
    {
        namespace CBs
        {
            // Must be size 16
            struct cbGeometryDeferredPerObject
            {
                Matrix4x4 WorldMatrix;
                Matrix4x4 ViewMatrix;
                Matrix4x4 ProjMatrix;
                Matrix4x4 InvWorldMatrix;
            };
            static_assert(sizeof(cbGeometryDeferredPerObject) % 16 == 0, "cbGeometryDeferredPerObject is not not multiple of 16");

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

            // Path Tracing
            struct cbPathTracerControl
            {
                uint32_t numSamples;
                uint32_t frameCount;
                uint32_t numIndicies;
                float _pad[1];
            };
            static_assert(sizeof(cbPathTracerControl) % 16 == 0, "cbPathTracerControl is not multiple of 16");

            struct cbPathTracerCamera
            {
                Vector3 cameraPos;
                uint32_t screenWidth;
                Vector3 cameraTarget;
                uint32_t screenHeight;
                Vector3 worldUp;
                float FOV_Horizontal;
            };
            static_assert(sizeof(cbPathTracerCamera) % 16 == 0, "cbPathTracerCamera is not multiple of 16");

        }
    }
}