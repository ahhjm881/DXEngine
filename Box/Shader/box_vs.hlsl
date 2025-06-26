#include "LightingCommon.hlsli"

cbuffer CBufferPerObject : register(b0)
{
    row_major float4x4 wvpMatrix;
    row_major float4x4 worldInverseTransposeMatrix;
    row_major float4x4 worldMatrix;
    
    Material material;
}
cbuffer CBufferPerFrame : register(b1)
{
    float3 eyePosition;
    DirectionalLight directionalLight;
}

struct Input
{
    float3 positionOS : POSITION;
    float3 normalOS: NORMAL;
};

struct Output
{
    float4 positionCS : SV_Position;
    float4 positionWS : POSITION;
    float3 normalWS : NORMAL;
};

Output VS_Main(Input input)
{
    Output output;
    output.positionCS = mul(float4(input.positionOS, 1.0f), wvpMatrix);
    output.positionWS = mul(float4(input.positionOS, 1.0f), worldMatrix);
    output.normalWS = mul(input.normalOS, (float3x3)worldInverseTransposeMatrix);
    
    return output;
}