#version 330 core

layout(location=0)out	vec4	outColor;

uniform		sampler2D		g_GroundMap;

in		vec2	v_fragCoord;

void	main()
{
	outColor = vec4(texture(g_GroundMap,v_fragCoord).rgb + 1.0,1.0);
}