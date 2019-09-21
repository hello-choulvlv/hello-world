/*
  *2018/3/19
  *xiaohuaxiong
 */
cbuffer  BufferConstant
{
	float4x4    g_MVPMatrix;
};

Texture2D    g_BaseTexture;

SamplerState  SampleNormal
{
	Filter = MIN_MAG_MIP_LINEAR;
};

struct VS_STREAM_IN
{
	float4    a_position    : POSITION;
	float2   a_fragCoord : TEXCOORD;
};

struct PS_STREAM_IN
{
	float4   v_position     : SV_POSITION;
	float2   v_fragCoord : TEXCOORD;
};

PS_STREAM_IN    vs_main(VS_STREAM_IN  vsIn)
{
	PS_STREAM_IN    psOut;

	psOut.v_position     = mul(vsIn.a_position,g_MVPMatrix);
	psOut.v_fragCoord = vsIn.a_fragCoord;

	return psOut;
}

float4 ps_main(PS_STREAM_IN  psIn) : SV_TARGET
{
	return g_BaseTexture.Sample(SampleNormal,psIn.v_fragCoord);
}

technique10   ColorTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0,vs_main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0,ps_main()));
	}
}