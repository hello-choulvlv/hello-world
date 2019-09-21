#version 330 core

layout(location=0)out	vec4	outColor;

uniform	sampler2D	g_PhotonMap;

in		vec3	v_fragCoord;

void	main()
{
	vec3    r = texture(g_PhotonMap,v_fragCoord.xy).rgb + 0.0625;
	outColor.rgb = r;
}