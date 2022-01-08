#ifndef FRESNEL_HLSL
#define FRESNEL_HLSL

// Schlick Approximation
// F0 = Spectral Dist of Fresnel factor at normal insidence
float3 FresnelSchlickRoughness(float3 N, float3 V, float3 F0)
{
    N = normalize(N);
    V = normalize(V);

    float NdotV = max(dot(N, V), 0.0f);
    return F0 + (1.0f - F0) * pow(1.0f - NdotV, 5.0f);
}

#endif