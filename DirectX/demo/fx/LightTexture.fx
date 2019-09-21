/*
  *2018/3/13
  *@author:xiaohuaxiong
 */
cbuffer GlobalVariableConstant
{
	float4x4   g_ModelMatrix;
	float4x4  g_ViewProjMatrix;
	float4x4  g_NormalMatrix;
	float3      g_LightDirection;
};

Texture2D    g_DiffuseMap;
//Texture2D    g_SpecularMap;
SamplerState   LinearSamplerObject
{
	Filter = MIN_MAG_MIP_LINEAR;
};


struct   VS_STREAM_IN
{
	float4    i_position : POSITION;
	float3    i_normal   : NORMAL;
	float2    i_fragCoord : TEXCOORD;
};

struct PS_STREAM_OUT
{
	float4  v_position : SV_POSITION;
	float3  v_normal   : NORMAL;
	float2  v_fragCoord : TEXCOORD;
};

PS_STREAM_OUT vs_main(VS_STREAM_IN vsIn)
{
	PS_STREAM_OUT  psOut;

	float4   position = mul(vsIn.i_position,g_ModelMatrix);
	psOut.v_position = mul(position,g_ViewProjMatrix);

	psOut.v_normal = mul(float4(vsIn.i_normal, 0.0f),g_NormalMatrix);

	psOut.v_fragCoord = vsIn.i_fragCoord;

	return psOut;
}

float4 ps_main(PS_STREAM_OUT psIn) : SV_TARGET
{
	float3  normal = normalize(psIn.v_normal);

	float4  baseColor = g_DiffuseMap.Sample(LinearSamplerObject,psIn.v_fragCoord);

	float diffuse = max(0.0f,dot(normal,g_LightDirection));

	baseColor.rgb *= 0.3f + diffuse * 0.7f;

	return baseColor;
}

technique10 LightTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0,vs_main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0,ps_main()));
	}
}

