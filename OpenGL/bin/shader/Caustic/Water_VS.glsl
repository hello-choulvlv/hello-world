#version 330 core
precision highp float;

layout(location=0)in	vec4	a_position;
layout(location=1)in	vec2	a_fragCoord;

uniform		mat4		g_ModelMatrix;
uniform		mat4		g_ViewProjMatrix;
uniform		sampler2D	g_HeightMap;

out		vec3	v_position;
out		vec2	v_fragCoord;

void	main()
{
	vec4	position = g_ModelMatrix * a_position;
	position.y += texture(g_HeightMap,a_fragCoord).r;
	gl_Position = g_ViewProjMatrix * position;

	v_position = position.xyz;
	v_fragCoord = a_fragCoord;
}
