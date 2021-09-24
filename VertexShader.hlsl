#include "common.hlsli"

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
};

PSInput main(VSInput input)
{
    PSInput output;

    input.position.w = 1.0f;

    // calculate the position of the vertex against the world, view, and projection matrices
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // calculate the normal vector against the world matrix only
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);
    output.tex = input.tex;

    // calculate the position of the vertex in the world
    float4 worldPosition = mul(input.position, worldMatrix);

    // determine the viewing direction based on the position of the camera and the position of the vertex in the world
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;

    // normalize the viewing direction vector
    output.viewDirection = normalize(output.viewDirection);

    return output;
}