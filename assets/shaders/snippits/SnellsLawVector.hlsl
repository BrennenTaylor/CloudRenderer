#ifndef SNELLSLAWVECTOR_HLSL
#define SNELLSLAWVECTOR_HLSL

float3 SnellsLawVector(float n1, float n2, float3 N, float3 s1)
{
    float3 s2 = float3(0.0f, 0.0f, 0.0f);

    float3 left = (n1 / n2) * cross(N, cross(-N, s1));

    float3 right = N * sqrt(1.0f - ((n1 / n2) * (n1 / n2)) * cross(N, s1) * cross(N, s1));

    s2 = left - right;
    return s2;
}

#endif