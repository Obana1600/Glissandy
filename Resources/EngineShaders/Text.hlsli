#pragma pack_matrix(row_major)

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};


struct GlyphInstance
{
    float4 rect;
    float4 uv;
};

StructuredBuffer<GlyphInstance> gInstances : register(t0);


cbuffer PassCB : register(b0)
{
    bool gUseOutline;
    bool gUseShadow;
}


cbuffer TextCB : register(b1)
{
    float4x4 gWVP;

    float4 gColorLT;
    float4 gColorRT;
    float4 gColorLB;
    float4 gColorRB;

    float4 gOutlineColor;
    float4 gShadowColor;
    float gOutlineWidth;
    float gShadowBaseY;
    float gShadowSoftness;
    float2 gShadowOffset;
    float2 gShadowScale;
    float2 gShadowShear;
};