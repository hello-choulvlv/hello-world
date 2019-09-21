/*
  *Water FX
  *2018/3/10
  *@author:xiaohuaxiong
 */
cbuffer BufferConstant
{
	float4x4  g_MVPMatrix;
	float4      g_Color;
};

void vs_main(float4 i_position : POSITION,
						   out float4 o_position : SV_POSITION
	)
{
	o_position = mul(i_position,g_MVPMatrix);
}

float4 ps_main(float4 i_position:SV_POSITION) : SV_TARGET
{
	return g_Color;
}

technique10 ColorTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0,vs_main()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0,ps_main()));
	}
}