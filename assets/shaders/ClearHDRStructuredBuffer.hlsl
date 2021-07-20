#ifndef CLEAR_HDR_STRUCTURED_BUFFER_HLSL
#define CLEAR_HDR_STRUCTURED_BUFFER_HLSL

RWStructuredBuffer<float4> imageBuffer : register(u0);

// The number of threads should be exposed as compile time defines
#ifdef USE_DEFAULT_THREAD_COUNTS

#define XThreadCount 256
#define YThreadCount 1
#define ZThreadCount 1

#else

#endif

[numthreads(XThreadCount, YThreadCount, ZThreadCount)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID)
{
    // Seed the random number generator
    // Hash using the thread id, then use wang_hash to make the seed go wide
    imageBuffer[dispatchThreadID.x] = float4(0.0f, 0.0f, 0.0f, 1.0f);
}

#endif