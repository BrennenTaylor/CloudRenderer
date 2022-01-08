#ifndef GENERATE_PRIMARY_RAYS_HLSL
#define GENERATE_PRIMARY_RAYS_HLSL

// Assumptions made!
// In addition, we also assume that all computations are in world space. This will be changed later.
cbuffer PathTracerCamera : register(b0)
{
    float3 CameraPos;
    uint ScreenWidth;
    float3 CameraTarget;
    uint ScreenHeight;
    float3 WorldUp;
    float FOV_Horizontal;
};

// All the data necessary to represent a ray
#include "snippits/Geometry.hlsl"

// Ok, we need some input output buffers as well
RWStructuredBuffer<Ray> rayBuffer : register(u0);

// The number of threads should be exposed as compile time defines
#ifdef USE_DEFAULT_THREAD_COUNTS

#define XThreadCount 32
#define YThreadCount 16
#define ZThreadCount 1

#else

#endif

// X : 32
// Y : 16
// Z : 1
// Total, Group Size : 512 threads
[numthreads(XThreadCount, YThreadCount, ZThreadCount)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID)
{
    // Seed the random number generator
    // Hash using the thread id, then use wang_hash to make the seed go wide
    uint index = (dispatchThreadID.x + dispatchThreadID.y * ScreenWidth);

    const float aspectRatio = (float)(ScreenWidth) / (float)(ScreenHeight);

    float3 camForward = normalize(CameraTarget - CameraPos);
    float3 camRight = -1.0f * normalize(cross(camForward, WorldUp));
    float3 camUp = -1.0f * normalize(cross(camForward, camRight));

    float horFov = FOV_Horizontal;
    float vertFov = horFov / aspectRatio;
    float nearDistance = 0.1f;

    float windowTop = tan(vertFov / 2.0f) * nearDistance;
    float windowRight = tan(horFov / 2.0f) * nearDistance;

    uint xVal = dispatchThreadID.x;
    uint yVal = dispatchThreadID.y;

    float u = (float)xVal / float(ScreenWidth);
    float v = (float)yVal / float(ScreenHeight);

    // Transform to [-1, 1] space from [0, 1] space
    u = u * 2.0f - 1.0f;
    v = v * 2.0f - 1.0f;

    float3 primaryRayOrigin = {0.0f, 0.0f, 0.0f};
    float3 primaryRayDirection = {0.0f, 0.0f, 0.0f};

    primaryRayOrigin = CameraPos + camForward * nearDistance;
    primaryRayOrigin = primaryRayOrigin + camRight * windowRight * u;
    primaryRayOrigin = primaryRayOrigin + camUp * windowTop * v;
    primaryRayDirection = normalize(primaryRayOrigin - CameraPos);

    Ray primaryRay;
    primaryRay.m_start = primaryRayOrigin;
    primaryRay.m_direction = primaryRayDirection;
    primaryRay.m_pad0 = 0.0f;
    primaryRay.m_pad1 = 0.0f;
    rayBuffer[index] = primaryRay;
}

#endif