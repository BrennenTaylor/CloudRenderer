#ifndef DISTRIBUTION_HLSL
#define DISTRIBUTION_HLSL

#include "snippits/Constants.hlsl"

float TrowbridgeReitzGGX(float3 normal, float3 halfVector, float roughness)
{
    float a2 = roughness * roughness;

    float NdotH = max(dot(normal, halfVector), 0.0f);

    float NdotH2 = NdotH * NdotH;

    float numerator = a2;
    float deonominator = NdotH2 * (a2 - 1) + 1;
    deonominator = deonominator * deonominator * F_PI;
    return numerator / deonominator;
}

#endif