#version 330 core
precision highp float;

layout(location=0)out	vec4	outColor;

uniform samplerCube	g_TexCubeMap;
uniform samplerCube g_PhotonCubeMap;

in	vec3	v_fragCoord;

void	main()
{
	outColor = texture(g_TexCubeMap,v_fragCoord);
	outColor.rgb += texture(g_PhotonCubeMap,v_fragCoord).rgb;
}