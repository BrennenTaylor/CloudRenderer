#ifndef COOKTORRENCE_HLSL
#define COOKTORRENCE_HLSL

#include "snippits/Fresnel.hlsl"
#include "snippits/ShadowMasking.hlsl"
#include "snippits/Distribution.hlsl"

float3 CookTorrence(in float3 lightIntensity, in float3 L, in float3 V, in float3 H, in float3 N, float3 surfaceAlbedo, float metalness, float roughness)
{
    // This is an average lambertonian diffuse value. It represents dielectric surfaces
    float3 F0 = float3(0.04f, 0.04f, 0.04f);

    // Metal = 0.0 => Using F0
    // Metal = 1.0 => Using albedo of material
    // We want the albdeo of the material to store the reflectance of metalic materials
    F0 = lerp(F0, surfaceAlbedo, metalness);
    // We now have a F0 that stores the reflectance of the surface with light shone at the normal

    // Cook Torrance BDRF
    // Handles specular calculation

    float NDF = TrowbridgeReitzGGX(N, H, roughness);
    float G = SmithsShadowMethod(N, V, L, roughness);
    float3 F = FresnelSchlickRoughness(N, V, F0);

    // Once we have our specular value, we want to calculate our diffuse based of this.
    // This keeps us constrained to ks + kd <= 1.0
    // kS = F represents the amount of energy reflected
    float3 kS = F;
    // The rest of the light is refracted
    // NOTE: Assumes that there is no transmittence
    // Enforce ks + kd = 1.0f
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    
    // Then, remove refracted light if the surface is metal (thus only reflectance)
    kD *= (1.0f - metalness);

    //  Cook-Torrance calculation
    // DFG / (4 * (N . L) * (N . V))
    float3 numerator = NDF * G * F;
    float denom = 4 * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.001; // Add small value to prevent divide by zero.

    float3 specular = numerator / denom;
    float3 diffuse = (surfaceAlbedo / F_PI) * kD;

    float NdotL = max(dot(N, L), 0.0f);

    float3 diffuseAndSpec = (diffuse + specular) * lightIntensity * NdotL;

    return diffuseAndSpec;
}

#endif