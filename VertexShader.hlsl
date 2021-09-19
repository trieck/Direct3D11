#include "common.hlsli"

PSInput main(VSInput input)
{
    PSInput output;

    input.position.w = 1.0f;

    output.position = input.position;
    output.tex = input.tex;
    output.normal = normalize(input.normal);
    
    return output;
}