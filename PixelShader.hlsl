#include "common.hlsli"

Texture2D shaderTexture;
SamplerState SampleType;

float4 main(PSInput input) : SV_TARGET
{
    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);

    return textureColor;
}