#ifndef PBR_LIGHTING_HLSL
#define PBR_LIGHTING_HLSL

float3 CalcLighting(in float3 position, in float roughness, in float3 albedo, in float metallic, in float3 normal, in float3 V, in float3 L)
{
// All the geometry vectors we will need
    float3 N = normalize(normal);
    // float3 V = normalize(CameraPos - position);
    // float3 L = normalize(LightPos - position);
    float3 H = normalize(V + L);

    // Attenuation
    float dist = length(LightPos - position);
    float attenuation = 1.0f / (dist * dist);
    float3 radiance = LightColor * attenuation * 80.0f;

    // This is an average lambertonian diffuse value. It represents dielectric surfaces
    // This is in linear space
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    // Metal = 0.0 => Using F0
    // Metal = 1.0 => Using albedo of material
    // We want the albdeo of the material to store the reflectance of metalic materials
    // Needs to be in linear space!!
    F0 = lerp(F0, albedo, metallic);

    // We now have a F0 that stores the reflectance of the surface with light shone at the normal

    // Cook Torrance BDRF
    // Enforce ks + kd = 1.0f
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
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    // Then, remove refracted light if the surface is metal (thus only reflectance)
    kD *= (1.0f - metallic);

    //  Cook-Torrance calculation
    // DFG / (4 * (N . L) * (N . V))
    float3 numerator = NDF * G * F;
    float denom = 4 * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.001; // Add small value to prevent divide by zero.

    float3 specular = numerator / denom;
    float3 diffuse = (albedo / F_PI) * kD;

    float NdotL = max(dot(N, L), 0.0f);

    float3 diffuseAndSpec = (diffuse + specular) * radiance * NdotL;

    return diffuseAndSpec;
}

#endif