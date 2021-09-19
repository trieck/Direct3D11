#include "common.hlsli"

Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 diffuseColor;
    float3 lightDirection;
};

float4 main(PSInput input) : SV_TARGET
{
    // Sample the pixel color from the texture using the sampler at this texture coordinate location
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);

    // Invert the light direction for calculations
    float3 lightDir = -lightDirection;

    // Calculate the amount of light on this pixel
    float lightIntensity = saturate(dot(input.normal, lightDir));

    // Determine the final amount of diffuse color based on the diffuse color combined with the light intensity
    float4 color = saturate(diffuseColor * lightIntensity);

    // Multiply the texture pixel and the final diffuse color to get the final pixel color result
    color = color * textureColor;

    return color;
}