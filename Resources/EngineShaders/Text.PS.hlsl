#include "Text.hlsli"

Texture2D<float4> gAtlas : register(t1);
SamplerState gSampler : register(s0);


float4 main(VSOutput input) : SV_TARGET
{
    float dist = gAtlas.Sample(gSampler, input.uv).r;
    float w = max(fwidth(dist), 0.0001f);
    
    // 影
    if (gUseShadow)
    {
        float e = max(gShadowSoftness, w);
        float a = smoothstep(0.5f - e, 0.5f + e, dist);
        float4 color = float4(gShadowColor.rgb, gShadowColor.a * a);
        clip(color.a - 0.001f);
        return color;
    }
    
    // 本体
    float faceAlpha = smoothstep(0.5f - w, 0.5f + w, dist);
    
    // 縁取りあり
    if (gUseOutline)
    {
        float outlineAlpha = smoothstep((0.5f - gOutlineWidth) - w, (0.5f - gOutlineWidth) + w, dist);
        float4 outlineLayer = float4(gOutlineColor.rgb, gOutlineColor.a * outlineAlpha);
        float4 faceLayer = float4(input.color.rgb, input.color.a * faceAlpha);
        float4 color = lerp(outlineLayer, faceLayer, faceAlpha);
        clip(color.a - 0.001f);
        return color;
    }
    
    // 縁取りなし
    float4 color = float4(input.color.rgb, input.color.a * faceAlpha);
    clip(color.a - 0.001f);
    return color;
}