#ifndef VERTEX_TRANSFORM_HLSL
#define VERTEX_TRANSFORM_HLSL

// NOTE: This MUST be a structured buffer. Should NOT be a RWStructuredBuffer
struct Vertex
{
    float3 position;
    uint matrixIndex;
    float3 normal;
    float _pad;
};

StructuredBuffer<Vertex> LocalVertices : register(t0);
StructuredBuffer<float4x4> LocalToWorld : register(t1);
StructuredBuffer<float4x4> LocalToWorldInv : register(t2);

RWStructuredBuffer<Vertex> WorldVertices : register(u0);

// The number of threads should be exposed as compile time defines
#ifdef USE_DEFAULT_THREAD_COUNTS

#define XThreadCount 256
#define YThreadCount 1
#define ZThreadCount 1

#else

#endif

// Total, Group Size : 256 threads
[numthreads(XThreadCount, YThreadCount, ZThreadCount)]
void CSMain(in uint3 DispatchIdx : SV_DispatchThreadID)
{
    Vertex localVert = LocalVertices[DispatchIdx.x];
    float4x4 localToWorld = LocalToWorld[localVert.matrixIndex];
    float4x4 localToWorldInv = LocalToWorldInv[localVert.matrixIndex];

    float4 worldPos = mul(float4(localVert.position, 1.0f), localToWorld);
    float4 worldNormal = mul(float4(localVert.normal, 1.0f), localToWorldInv);

    Vertex worldVertex = localVert;
    worldVertex.position = worldPos.xyz;
    worldVertex.normal = normalize(worldNormal.xyz);
    WorldVertices[DispatchIdx.x] = worldVertex;
}

#endif