#include "common.hlsli"

Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float specularPower;
    float4 specularColor;
};

float4 main(PSInput input) : SV_TARGET
{
    // sample the pixel color from the texture using the sampler at this texture coordinate location
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);

    // set the default output color to the ambient light value for all pixels
    float4 color = ambientColor;

    // initialize the specular color
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // invert the light direction for calculations
    float3 lightDir = -lightDirection;

    // calculate the amount of light on this pixel
    float lightIntensity = saturate(dot(input.normal, lightDir));

    if (lightIntensity > 0.0f) {
        // determine the final amount of diffuse color based on the diffuse color combined with the light intensity
        color += (diffuseColor * lightIntensity);

        // saturate the final color
        color = saturate(color);

        // calculate the reflection vector based on the light intensity, normal vector, and light direction
        float3 reflection = normalize(2 * lightIntensity * input.normal - lightDir);

        // determine the amount of specular light based on the reflection vector, viewing direction, and specular power
        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
    }

    // multiply the texture pixel and the final diffuse color to get the final pixel color result
    color = color * textureColor;

    // add the specular component last to the output color
    color = saturate(color + specular);

    return color;
}
