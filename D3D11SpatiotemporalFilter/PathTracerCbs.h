#pragma once

namespace Farlor
{
    namespace CBs
    {

        // Random number initial generation
        struct cbRandomSeedControl
        {
            uint32_t FrameCount;
            uint32_t ScreenWidth;
            uint32_t ScreenHeight;
            float _RSC_PAD;
        };

        // Path Tracing
        struct cbPathTracerControl
        {
            uint32_t screenWidth;
            uint32_t screenHeight;
            Farlor::Vector2 pad;
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
        
        struct cbTimeValues
        {
            float DeltaTime;
            float TotalTime;
            float _pad[2];
        };
        static_assert(sizeof(cbTimeValues) % 16 == 0, "cbTimeValues is not multiple of 16");

        // The order of varialbes is soooooo important here
        struct cbDenoisingGlobalSettings
        {
            float Width;
            float Height;
            float _pad[2];
            float kernel[28];
            int32_t offsetX[28];
            int32_t offsetY[28];
        };
        static_assert(sizeof(cbDenoisingGlobalSettings) % 16 == 0, "cbDenoisingGlobalSettings is not multiple of 16");

        // The order of varialbes is soooooo important here
        struct cbDenoisingPassSettings
        {
            float stepWidth;
            float colorPhi;
            float normalPhi;
            float positionPhi;
        };
        static_assert(sizeof(cbDenoisingPassSettings) % 16 == 0, "cbDenoisingPassSettings is not multiple of 16");

        // The order of varialbes is soooooo important here
        struct cbTonemapPassSettings
        {
            uint32_t ScreenWidth;
            uint32_t ScreenHeight;
            uint32_t PTC_Pad0;
            uint32_t PTC_Pad1;
        };
        static_assert(sizeof(cbTonemapPassSettings) % 16 == 0, "cbTonemapPassSettings is not multiple of 16");
    }
}