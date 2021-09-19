
struct VSInput
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal: NORMAL;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal: NORMAL;
};

