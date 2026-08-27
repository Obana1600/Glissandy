#pragma pack_matrix(row_major)

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};


struct PSOutput
{
    float4 color : SV_TARGET;
};


cbuffer Root : register(b0)
{
    float4x4 gViewProj;
    float3 gCameraRight;
    float3 gCameraUp;
}


struct ParticleInstance
{
    float3 position;
    float2 size;
    float roll;
    float4 color;
    float4 uv;
};
StructuredBuffer<ParticleInstance> gInstances : register(t0);

Texture2D gTex : register(t1);