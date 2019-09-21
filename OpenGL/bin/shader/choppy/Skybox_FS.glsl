#version 330 core
layout(location=0)out	vec4	outColor;
uniform		sampler2D	g_SkyboxMap;
in		vec2	v_fragCoord;

void	main()
{
	outColor = texture(g_SkyboxMap,v_fragCoord);
}