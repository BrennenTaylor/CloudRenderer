#ifndef TONEMAPPING_FUNCTIONS_HLSL
#define TONEMAPPING_FUNCTIONS_HLSL

// Linear tonemapping
float3 LinearTonemapping(float3 baseColor)
{
    // Hardcoded exposure adjustment
    float3 tonemappedColor = baseColor * 16;
    // Gamma correction
    tonemappedColor = pow(tonemappedColor, 1 / 2.2f);
    return tonemappedColor;
}

// Reinhard tonemapping
float3 ReinhardTonemapping(float3 baseColor)
{
    // Hardcoded exposure adjustment
    float3 tonemappedColor = baseColor * 16;
    tonemappedColor = tonemappedColor / (1 + tonemappedColor);
    // Gamma correction
    tonemappedColor = pow(tonemappedColor, 1 / 2.2f);
    return tonemappedColor;
}

// Jim Hejl and Richard Burgess Tonemapping
// Optimized variation of reinhard
float3 HejlBurgessTonemapping(float3 baseColor)
{
    // Hardcoded exposure adjustment
    float3 tonemappedColor = baseColor * 16;
    float3 x = max(0, tonemappedColor - 0.004f);
    tonemappedColor =(x * (6.2f * x + 0.5f)) / (x * (6.2f * x + 1.7f) + 0.06f);
    return tonemappedColor;
}

float3 Uncharted2TonemapPrivateSupport(float3 x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

// Uncharted 2 tonemapping
float3 UnchartedTwoTonemapping(float3 baseColor)
{
    float W = 11.2;

    // Hardcoded exposure adjustment
    float3 tonemappedColor = baseColor * 16;

    float exposureBias = 0.01f;
    float3 current = Uncharted2TonemapPrivateSupport(exposureBias * tonemappedColor);

    float3 whiteScale = 1.0f / Uncharted2TonemapPrivateSupport(W);
    float3 color = current * whiteScale;

    tonemappedColor = pow(color, 1 / 2.2f);
    return tonemappedColor;
}

// Null tonemape
float3 NullTonemapping(float3 color)
{
    return color;
}

#endif