/*
  *reflect
  *2018/3/16
  *xiaohuaxiong
 */
cbuffer VertexConstant
{
	float4x4  g_ModelMatrix;
	float4x4  g_ViewProjMatrix;
	float3      g_LightDirection;
};

Texture2D   g_BaseTexture;

SamplerState   SampleLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
};

struct   VS_STREAM_IN
{
	float4     a_position     : POSITION;
	float3     a_normal       : NORMAL  ;
	float2     a_fragCoord  : TEXCOORD;
}; 

struct PS_STREAM_IN
{
	float4    v_position : SV_POSITION;
	float4    v_normal   : NORMAL;
	float2    v_fragCoord : TEXCOORD;
};

PS_STREAM_IN   vs_main(VS_STREAM_IN   vsIn)
{
	PS_STREAM_IN    pixelOutput;

	float4  position = mul(vsIn.a_position,g_ModelMatrix);
	pixelOutput.v_position =mul(position,g_ViewProjMatrix) ;

	pixelOutput.v_normal = mul(float4(vsIn.a_normal,0.0f),g_ModelMatrix);
	pixelOutput.v_fragCoord = vsIn.a_fragCoord;
	return pixelOutput;
}

float4 ps_main(PS_STREAM_IN  pixelIn) : SV_TARGET
{
	float3   normal = normalize(pixelIn.v_normal);
	float     diffuse = max(0.0f,dot(normal,g_LightDirection));
	float4   color = g_BaseTexture.Sample(SampleLinear, pixelIn.v_fragCoord);

	color.rgb *=  lerp(1.0f,diffuse,0.7f);
	return color;
}

technique10 ReflectTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0,vs_main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0,ps_main()));
	}
}