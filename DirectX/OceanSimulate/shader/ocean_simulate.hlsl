/*
  *generate displacement and gradient vector
  *2018/4/3
 */
cbuffer  Immutable : register(b0)
{
	uint   g_ActualDim;
	uint   g_InputWidth;
	uint   g_InputHeight;
	uint   g_DxOffset;
	uint   g_DyOffset;
	float  g_PixelWidth;
	float  g_PixelHeight;
	float  g_ChoppyScale;
};

Texture2D					g_TextureDxyz : register(t0);
SamplerState			g_SamplerDxyz : register(s0);

struct VS_STRUCT_STREAM
{
	float4	v_position	:	SV_POSITION;
	float2  v_fragCoord :   TEXCOORD0;
};

VS_STRUCT_STREAM   Quad_VS(float4   a_position : POSITION)
{
	VS_STRUCT_STREAM  output;
	output.v_position = a_position;
	output.v_fragCoord = mad(a_position.xy,float2(0.5f,-0.5f),float2(0.5f,0.5f));
	return output;
}

ByteAddressBuffer  g_BufferDxyz : register(t0);

float4 Gen_Displacement_FS(VS_STRUCT_STREAM input) : SV_TARGET
{
	uint   offsetX = (uint)(input.v_fragCoord.x * g_InputWidth);
	uint   offsetY = (uint)(input.v_fragCoord.y * g_InputHeight);
	uint   offset  = g_InputWidth * offsetY + offsetX;

	//float  sign_correct =-((((offsetX+offsetY) & 0x1) <<1) -1);
	int   sign_correct = ((offsetX + offsetY) & 1) ? -1: 1;

	float  dx = asfloat( g_BufferDxyz.Load((g_DxOffset+offset)<<3) ) * sign_correct * g_ChoppyScale;
	float  dy = asfloat (g_BufferDxyz.Load((g_DyOffset+offset)<<3)) * sign_correct * g_ChoppyScale;
	float  dz = asfloat( g_BufferDxyz.Load(offset << 3)) * sign_correct;

	return float4(dx,dy,dz,1);
}

float4 Gen_Gradient_FS(VS_STRUCT_STREAM input) : SV_TARGET
{
	//left/right/bottom/top
	float3  leftPixel  = g_TextureDxyz.Sample(g_SamplerDxyz,input.v_fragCoord - float2(g_PixelWidth,0)).xyz;
	float3  rightPixel = g_TextureDxyz.Sample(g_SamplerDxyz,input.v_fragCoord + float2(g_PixelWidth,0)).xyz;
	float3  bottomPixel= g_TextureDxyz.Sample(g_SamplerDxyz,input.v_fragCoord - float2(0,g_PixelHeight)).xyz;
	float3  topPixel   = g_TextureDxyz.Sample(g_SamplerDxyz,input.v_fragCoord + float2(0,g_PixelHeight)).xyz;

	return float4(leftPixel.z - rightPixel.z, bottomPixel.z - topPixel.z,0,1.0f);
}