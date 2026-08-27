#include "Particle.hlsli"

SamplerState gSampler : register(s0);


PSOutput main(VSOutput input)
{
    PSOutput output;
	
    float4 texColor = gTex.Sample(gSampler, input.uv);
    output.color = input.color * texColor;
	
	return output;
}