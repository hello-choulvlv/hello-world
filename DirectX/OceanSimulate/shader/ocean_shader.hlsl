/*
  *2018/4/5
  *@author:xiaohuaxiong
 */
cbuffer cbConstantBuffer : register(b0)
{
	float3        g_SkyColor;
	float          g_TexelLength_x2;
	float3        g_WaterbodyColor;
	float          g_UVScale;
	float3        g_PerlinAmplitude;
	float          g_UVOffset;
	float3        g_PerlinOctave;
	float          g_PerlinSize;
	float3        g_BlendParam;
	float          g_Sunshine;
	float3       g_PerlinGradient;
	float         padding0;
	float3       g_SunDir;
	float         g_padding1;
	float3      g_SunColor;
	float         padding1;
};

cbuffer cbPerFrameBuffer : register(b1)
{
	float4x4    g_ViewProjMatrix;
	float4x4    g_LocalMatrix;
	float3        g_EyeLocation;
	float          unuse;
	float2        g_UVBase;
	float2        g_PerlinMovement;
};

Texture2D       g_TextureDisplace : register(t0);
Texture2D       g_TexturePerlin      : register(t1);
Texture2D       g_TextureGradient : register(t2);
TextureCube   	g_CubeReflect         : register(t3);
Texture1D       g_TextureFreshnel  :register(t4);

SamplerState  g_SamplerDisplace : register(s0);
SamplerState  g_SamplerPerlin      : register(s1);
SamplerState  g_SamplerGradient : register(s2);
SamplerState  g_SamplerReflect    : register(s3);
SamplerState  g_SamplerFreshnel :register(s4);

struct  VS_OUT_STREAM
{
	float4    v_position : SV_POSITION;
	float3    v_localPosition : TEXCOORD0;
	float2    v_fragCoord : TEXCOORD1;
};

VS_OUT_STREAM   vs_main(float4  a_position : POSITION)
{
	VS_OUT_STREAM   output;
	output.v_localPosition = mul(a_position,g_LocalMatrix).xyz;

	output.v_fragCoord = output.v_localPosition.xy * g_UVScale + g_UVOffset;
	//Perlin param
	float2   perlinCoord = output.v_fragCoord * g_PerlinSize + g_UVBase;
	float     x = g_TexturePerlin.SampleLevel(g_SamplerPerlin,perlinCoord * g_PerlinOctave.x + g_PerlinMovement,0).w;
	float     y = g_TexturePerlin.SampleLevel(g_SamplerPerlin,perlinCoord * g_PerlinOctave.y + g_PerlinMovement,0).w;
	float     z = g_TexturePerlin.SampleLevel(g_SamplerPerlin, perlinCoord * g_PerlinOctave.z + g_PerlinMovement,0).w;

	float     perlin_step = dot(g_PerlinAmplitude,float3(x,y,z));

	float3  displace = g_TextureDisplace.SampleLevel(g_SamplerDisplace, output.v_fragCoord,0).xyz;
	float     factor = clamp((20000.0f - length(output.v_localPosition.xy - g_EyeLocation.xy))/19200.0f,0.0f,1.0f);
	displace = lerp(float3(0,0,perlin_step),displace, factor);

	output.v_localPosition += displace;
	output.v_position = mul(float4(output.v_localPosition,1.0f),g_ViewProjMatrix);
	return output;
}

float4    ps_main(VS_OUT_STREAM  input) : SV_TARGET
{
	float2    perlin_coord = input.v_fragCoord * g_PerlinSize + g_UVBase;
	float2    perlin_1 = g_TexturePerlin.Sample(g_SamplerPerlin,perlin_coord*g_PerlinOctave.x + g_PerlinMovement).xy;
	float2    perlin_2 = g_TexturePerlin.Sample(g_SamplerPerlin,perlin_coord*g_PerlinOctave.y+ g_PerlinMovement).xy;
	float2    perlin_3 = g_TexturePerlin.Sample(g_SamplerPerlin,perlin_coord*g_PerlinOctave.z+ g_PerlinMovement).xy;

	float2   perlin_gradient = perlin_1*g_PerlinGradient.x + perlin_2*g_PerlinGradient.y + perlin_3*g_PerlinGradient.z;
	float     blend_coefficient = clamp((20000.0f-length(input.v_localPosition.xy - g_EyeLocation.xy))/19200.0f,0.0f,1.0f);
	blend_coefficient = blend_coefficient*blend_coefficient*blend_coefficient;

	float2  gradient = g_TextureGradient.Sample(g_SamplerGradient,input.v_fragCoord).xy;
	gradient = lerp(perlin_gradient, gradient, blend_coefficient);

	float3 normal = normalize(float3(gradient,g_TexelLength_x2));
	float3 eye_direction = normalize(g_EyeLocation - input.v_localPosition);
	float3 eye_reflect = reflect(-eye_direction,normal);

	float2 raw_freshnel = g_TextureFreshnel.Sample(g_SamplerFreshnel, dot(eye_direction, normal)).xy;
	eye_reflect.z = max(0.0f,eye_reflect.z);

	float3   reflect_color = g_CubeReflect.Sample(g_SamplerReflect,eye_reflect).xyz;
	reflect_color = reflect_color * reflect_color * 2.5f;

	reflect_color = lerp(g_SkyColor,reflect_color,raw_freshnel.y);
	float3 body_color = lerp(g_WaterbodyColor,reflect_color,raw_freshnel.x);

	body_color += g_SunColor * pow(max(0.0f,dot(eye_reflect,g_SunDir)),g_Sunshine);

	return float4(body_color,1.0f);
}