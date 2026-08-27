#include "Text.hlsli"


VSOutput main(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    GlyphInstance g = gInstances[iid];

    float2 corner = float2(vid & 1, (vid >> 1) & 1);
    float x = g.rect.x + corner.x * g.rect.z;
    float y = g.rect.y - corner.y * g.rect.w;
    
    // 影
    if (gUseShadow)
    {
        float baseY = gShadowBaseY;
        float localY = y - baseY;
        float sx = (x * gShadowScale.x) + (localY * gShadowShear.x) + gShadowOffset.x;
        float sy = (x * gShadowShear.y) + (localY * gShadowScale.y) + gShadowOffset.y + baseY;
        x = sx;
        y = sy;
    }
    
    VSOutput output;
    output.pos = mul(float4(x, y, 0.0f, 1.0f), gWVP);
    output.uv = lerp(g.uv.xy, g.uv.zw, corner);
    
    float4 top = lerp(gColorLT, gColorRT, corner.x);
    float4 bottom = lerp(gColorLB, gColorRB, corner.x);
    output.color = lerp(top, bottom, corner.y);
    
    return output;
}