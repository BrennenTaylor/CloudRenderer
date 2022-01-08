struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TexCoord;
};

VSOutput VSMain(uint id : SV_VertexID)
{
    VSOutput output;

    // Generate clip space position
    output.pos.x = (float)(id / 2) * 4.0 - 1.0;
    output.pos.y = (float)(id % 2) * 4.0 - 1.0;
    output.pos.z = 0.0;
    output.pos.w = 1.0;

    // Texture coords
    output.uv.x = (float)(id / 2) * 2.0;
    output.uv.y = 1.0 - (float)(id % 2) * 2.0;

    return output;
}