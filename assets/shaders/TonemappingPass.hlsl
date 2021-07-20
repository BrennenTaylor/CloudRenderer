#include "snippits/FullScreenTriVS.hlsl"

// Thus MUST be a structured buffer. Should NOT be a RWStructuredBuffer
StructuredBuffer<float4> HDRTexture : register(t0);

#include "snippits/TonemappingFunctions.hlsl"
#include "snippits/ColorSpaceConversions.hlsl"

cbuffer TonemapParams : register(b0)
{
    uint ScreenWidth;
    uint ScreenHeight;
    uint PTC_Pad0;
    uint PTC_Pad1;
};

float4 PSMain(VSOutput input) : SV_TARGET
{
    uint index = (uint)(input.uv.x * ScreenWidth) + (uint)(input.uv.y * ScreenHeight) * ScreenWidth;
    float3 hdrColor = HDRTexture.Load(index).rgb;
    float3 tonemappedColor = UnchartedTwoTonemapping(hdrColor);
    //float3 tonemappedColor = NullTonemapping(hdrColor);
    return float4(tonemappedColor, 1.0f);

    // Convert back to sRGB
    float3 sRGBColor = LinearToSRGB(tonemappedColor);

    return float4(sRGBColor, 1.0f);
}