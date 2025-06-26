#include "LightingCommon.hlsli"
#include "LightingFunction.hlsli"

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


struct Output
{
    float4 positionCS : SV_Position;
    float4 positionWS : POSITION0;
    float3 normalWS : NORMAL;
};

float4 PS_Main(Output input) : SV_Target
{
    input.normalWS = normalize(input.normalWS);

    float3 toEyeDirection = normalize(eyePosition - input.positionWS);
    float4 outAmbient, outDiffuse, specular;
    
    ComputeDirectionalLight(material, directionalLight, input.normalWS, toEyeDirection, outAmbient, outDiffuse, specular);

    float4 litColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    litColor += outAmbient + outDiffuse + specular;
    litColor.a = 1.0f;
    
    return litColor;
}