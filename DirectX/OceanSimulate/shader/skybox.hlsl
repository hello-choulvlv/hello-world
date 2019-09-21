/*
  *Skybox
  *2018/3/31
  *@author:xiaohuaxiong
 */
cbuffer ConstantBuffer : register(b0)
{
	float4x4     g_MVPMatrix;
};

TextureCube    g_SkyboxCube : register(t0);
SamplerState   g_SkyboxSamplerState : register(s0);

void vs_main(float4   a_position : POSITION,
	out   float4   v_position : SV_POSITION,
	out    float3   v_fragCoord : TEXCOORD)
{
	v_position = a_position * 200000;
	v_fragCoord = mul(v_position,g_MVPMatrix).xyz;
}

float4  ps_main(float4 v_position : SV_POSITION,
		float3 v_fragCoord : TEXCOORD): SV_TARGET
{
	return g_SkyboxCube.Sample(g_SkyboxSamplerState,v_fragCoord);
}