#include "Particle.hlsli"


VSOutput main(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    ParticleInstance p = gInstances[iid];
	
    float2 uvT = float2(vid & 1, (vid >> 1) & 1);
    float2 corner = uvT - 0.5f;
    
    float s = sin(p.roll);
    float c = cos(p.roll);
    float2 r = float2(corner.x * c - corner.y * s, corner.x * s + corner.y * c) * p.size;
	
    float3 worldPos = p.position + gCameraRight.xyz * r.x + gCameraUp.xyz * r.y;
    
    VSOutput output;
    output.pos = mul(float4(worldPos, 1.0f), gViewProj);
    output.uv = lerp(p.uv.xy, p.uv.zw, uvT);
    output.color = p.color;
    
	return output;
}