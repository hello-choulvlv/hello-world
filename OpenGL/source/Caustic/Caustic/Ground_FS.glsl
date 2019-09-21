#version 330 core
layout(location=0)out	vec4	outColor;

uniform		sampler2D	g_BaseMap;
uniform		sampler2D	g_CausticMap;

in	vec2	v_fragCoord;

void	main()
{
	outColor.rgb = texture(g_BaseMap,v_fragCoord).rgb * texture(g_CausticMap,v_fragCoord).rgb;
	outColor.a =1.0;
}