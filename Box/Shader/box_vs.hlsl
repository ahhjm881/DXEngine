
cbuffer CBufferPerObject : register(b0)
{
    row_major float4x4 wvpMatrix;
}

struct Input
{
    float3 positionOS : POSITION;
    float4 color : COLOR;
};

struct Output
{
    float4 positionCS : SV_Position;
    float4 color : COLOR;
};

Output VS_Main(Input input)
{
    Output output;
    output.positionCS = mul(float4(input.positionOS, 1.0f), wvpMatrix);
    output.color = input.color;
    
    return output;
}