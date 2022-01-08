#ifndef STD_GEOMETRY_DEFERRED_HLSL
#define STD_GEOMETRY_DEFERRED_HLSL

cbuffer PerObject : register(b0)
{
    float4x4 WorldMatrix;
    float4x4 InvTransWorldMatrix;
    uint ObjectID;
    float _PadPerObject0;
    float _PadPerObject1;
    float _PadPerObject2;
};

// Per Camera
cbuffer PerFrame : register(b1)
{
    float4x4 ViewMatrix;
    float4x4 ProjMatrix;
    float4x4 PrevViewMatrix;
    float4x4 PrevProjMatrix;
};

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent : TANGENT;
};

struct VSOutput
{
    float4 PositionCS               : SV_Position;
    float4 CurrentPosCS             : CURRPOSCS;
    float4 PrevPosCS                : PREVPOSCS;
    float3 NormalOS                 : NORMALOS;
    uint ObjectID                   : OBJECT_ID;
    float3 NormalWS                 : NORMALWS;
};

struct PSOutput
{
    float4 NormalOSTarget                 : SV_Target0;
    float4 NormalWSTarget                 : SV_Target1;
    float2 VelocityTarget                 : SV_Target2;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.NormalOS = normalize(input.Normal);
    output.NormalWS = normalize(input.Normal);
    output.NormalWS = mul(float4(output.NormalWS, 1.0f), InvTransWorldMatrix).xyz;

    output.ObjectID = ObjectID;

    output.PositionCS = mul(float4(input.Position, 1.0f), WorldMatrix);
    output.PositionCS = mul(output.PositionCS, ViewMatrix);
    output.PositionCS = mul(output.PositionCS, ProjMatrix);

    output.CurrentPosCS = output.PositionCS;

    output.PrevPosCS = mul(float4(input.Position, 1.0f), WorldMatrix);
    output.PrevPosCS = mul(output.PrevPosCS, PrevViewMatrix);
    output.PrevPosCS = mul(output.PrevPosCS, PrevProjMatrix);

    return output;
}

PSOutput PSMain(VSOutput input)
{
    PSOutput output;
    float meshIDAsFloat = asfloat(input.ObjectID);
    output.NormalOSTarget = float4(normalize(input.NormalOS), meshIDAsFloat);
    output.NormalWSTarget = float4(normalize(input.NormalWS), 1.0f);

    float2 current = (input.CurrentPosCS.xy / input.CurrentPosCS.w) * 0.5f + 0.5f;
    float2 prev = (input.PrevPosCS.xy / input.PrevPosCS.w) * 0.5f + 0.5f;

    output.VelocityTarget = current - prev;
    return output;
}

#endif