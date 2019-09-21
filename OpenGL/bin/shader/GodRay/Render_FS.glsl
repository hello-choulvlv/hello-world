#version 330 core
layout(location=0)out		vec4		outColor;
uniform		sampler2D		g_BaseMap;
in		vec2	v_fragCoord;
void	main()
{
	float	samples = 128.0;
	float   intensity = 0.125;
	float   decay = 0.96875;
	vec2	nowFragCoord = v_fragCoord;
	vec2	direction = (vec2(0.5) - v_fragCoord)/samples;
	outColor=vec4(0.0);
	for(float sample =0;sample <samples; sample+=1.0)
	{
		outColor += texture(g_BaseMap,nowFragCoord) * intensity;
		intensity *= decay;
		nowFragCoord += direction;
	}
	outColor.a = 1.0;
	//outColor = texture(g_BaseMap,v_fragCoord);
}