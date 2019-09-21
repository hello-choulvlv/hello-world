/*
  *2018/3/31
  *@author:xiaohuaxiong
 */
cbuffer BufferConstant
{
	float4x4    g_MVPMatrix;
};

void   vs_main(float4   a_position: POSITION,
							float4   a_color :  COLOR,
							out   float4  v_position : SV_POSITION,
							out   float4  v_color : COLOR)
{
	v_position = a_position;
	v_color = a_color;
}

float4 ps_main(float4 v_position : SV_POSITION,
	float4  v_color : COLOR) : SV_TARGET
{
	return v_color;
}