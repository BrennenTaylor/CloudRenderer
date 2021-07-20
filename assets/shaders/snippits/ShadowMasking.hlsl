#ifndef SHADOW_MASKING_HLSL
#define SHADOW_MASKING_HLSL

#include "snippits/Constants.hlsl"

// Approximation of the Smith G function
float SchlickGGX(float Ndot, float k)
{
    float num = Ndot;
    float denom = Ndot * (1.0f - k) + k;
    return num / denom;
}

// https://learnopengl.com/#!PBR/Theory
// Approximation of the smith shadow mask formula
float SmithsShadowMethod(float3 normal, float3 V, float3 L, float roughness)
{
    // For now, roughness simply maps to alpha
    // TODO: Not necessarally correct
    float alpha = roughness;

    // Alright, if we are direct lighting, we map k this way
    // TODO: Maybe should do this outside the shadow method function...?
    float k = (alpha + 1) * (alpha + 1) / 8.0f;

    // If IBL, we do it this way
    // float k = alpha * alpha / 2.0f;

    float NdotV = max(dot(normal, V), 0.0f);
    float NdotL = max(dot(normal, L), 0.0f);

    //return SchlickGGX(normal, L, k);
    return SchlickGGX(NdotV, k) * SchlickGGX(NdotL, k);
}

#endif