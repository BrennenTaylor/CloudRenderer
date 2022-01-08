#ifndef COLOR_SPACE_CONVERSIONS_H
#define COLOR_SPACE_CONVERSIONS_H

float3 SRGBToLinear(float3 srgbColor)
{
    float3 linearColor = srgbColor * (srgbColor * (srgbColor * 0.305306011 + 0.682171111) + 0.012522878);
    return linearColor;
}

float3 LinearToSRGB(float3 linearColor)
{
    float3 S1 = sqrt(linearColor);
    float3 S2 = sqrt(S1);
    float3 S3 = sqrt(S2);
    float3 sRGB = 0.585122381 * S1 + 0.783140355 * S2 - 0.368262736 * S3;
    return sRGB;
}

#endif