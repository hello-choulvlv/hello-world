#version 330 core
layout(location=0)out		vec4		outColor;
uniform		sampler2D		g_BaseMap;

in 		vec2	v_fragCoord;

void	main()
{
	outColor = texture(g_BaseMap,v_fragCoord);
}