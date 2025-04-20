struct Output
{
    float4 positionCS : SV_Position;
    float4 color : COLOR;
};

float4 PS_Main(Output input) : SV_Target
{
    return input.color;
}