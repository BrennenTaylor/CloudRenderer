#ifndef WET_SURFACE_FUNCTIONS_HLSL
#define WET_SURFACE_FUNCTIONS_HLSL

#include "snippits/SnellsLawVector.hlsl"

#include "snippits/CookTorrence.hlsl"
#include "snippits/Constants.hlsl"

float3 CalcDryLighting(in float3 position, in float roughness, in float3 albedo, in float metallic, in float3 normal, in float3 L, in float3 lightIntensity)
{
    float3 surfaceN = normalize(normal);
    float3 mainV = normalize(CameraPos - position);
    float3 mainL = normalize(L);
    float3 mainH = normalize(mainL + mainV);

    float3 rM = CookTorrence(lightIntensity, mainL, mainV, mainH, surfaceN, albedo, metallic, roughness);
    return rM;
}

float3 CalcWetnessLighting(in float3 position, in float roughness, in float3 albedo, in float metallic, in float3 normal, in float3 L, in float3 lightIntensity)
{
    // Ok, we are going to model wet surfaces as followed

    /* 
        1. Model reflectance of water surface as fresnel
        2. Model Material reflectance with cook-torrance
        3. Model Total internal reflection as water leaves 
    */

    float3 surfaceN = normalize(normal);
    float3 mainV = normalize(CameraPos - position);
    float3 mainL = normalize(L);

    // NOTE: We model the water surface as a metal.
    // TODO: This looks awful.... we need a new model...
    float3 F0 = float3(0.04f, 0.04f, 0.04f);

    float3 F = FresnelSchlickRoughness(surfaceN, mainV, F0);

    float3 num = F;
    float denom = 4 * max(dot(surfaceN, mainV), 0.0f) * max(dot(surfaceN, mainL), 0.0f) + 0.001;
    float NdotL = max(dot(surfaceN, mainL), 0.0f);

    // F will model the amount of light to be reflected by the water
    // It is modified by the wetness value
    float3 waterCont = (num * NdotL) / denom;
    float3 rW = lightIntensity * waterCont;

    // Amount transmitted
    float3 rT = lightIntensity - rW;

    // The amount transmitted is then fed into cook-torrence
    // We need a modified light and view vector
    // First, the reflection off of the water.
    float airIOR = 1.0f;
    float waterIOR = 1.33f;

    // We hard code the critical angle as this can be precalculated for our substances
    // In degrees
    float criticalAngleDegree = 48.753466631327235;
    float criticalAngleRad = 0.85090851447573789823;

    float3 LPrime = -1.0f * normalize(SnellsLawVector(airIOR, waterIOR, surfaceN, -mainL));
    float3 VPrime = -1.0f * normalize(SnellsLawVector(airIOR, waterIOR, surfaceN, -mainV));
    float3 HPrime = normalize(LPrime + VPrime);

    // We model this by utalizing cook-torrance
    float3 waterAbsorbtionPerCentemeter = float3(0.0035f, 0.0004f, 0.0f);
    float depth = 5; // cm
    float cosLPrimeAngle = dot(LPrime, -normal);
    float cosVPrimeAngle = dot(VPrime, -normal);

    float3 power = -1.0f * waterAbsorbtionPerCentemeter * depth * ((1 / cosLPrimeAngle) + (1 / cosVPrimeAngle));

    float3 absorbtion = exp(power);

    float3 incidentLightOnMaterial = rT * absorbtion;

    float3 rM = CookTorrence(incidentLightOnMaterial, LPrime, VPrime, HPrime, surfaceN, albedo, metallic, roughness);

    float3 incidentLightUnderWater = rM * absorbtion;

    // Finally, we compute the amount of material reflected light that experiences total internal reflection
    float vAngle = acos(dot(-mainV, -normal));

    float amountTransmitted = 0.0f;
    if (vAngle == criticalAngleRad)
    {
        amountTransmitted = 0.5f;
    }
    // Case 1, internal reflection < 0.5
    else if (vAngle < criticalAngleDegree)
    {
        amountTransmitted = 1.0f - ((vAngle / criticalAngleRad) * 0.5f);
    }
    // Case 2, internal reflection > 0.5
    else
    {
        amountTransmitted = ((vAngle - criticalAngleRad) / ((F_PI / 2.0f) - criticalAngleRad)) * 0.5;
    }

    float3 scaledMaterial = incidentLightUnderWater * amountTransmitted;
    float3 totalIntensity = rW + scaledMaterial;
    return totalIntensity;
}

#endif