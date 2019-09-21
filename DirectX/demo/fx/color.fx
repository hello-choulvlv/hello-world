/*
  *2018/3/6
  *@author:xiaohuaxiong
 */
cbuffer BufferContant
{
	float4x4 g_MVPMatrix;
};

void vs_main(float4 i_position : POSITION,
						  float4 i_color       : COLOR,
						  out     float4    o_position : SV_POSITION,
						  out     float4    o_color       :  COLOR
						)
{
	o_position = mul(i_position, g_MVPMatrix);
	o_color = i_color;
}

float4 ps_main(float4 i_position :SV_POSITION,
	float4  i_color : COLOR ) : SV_TARGET
{
	return i_color;
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
