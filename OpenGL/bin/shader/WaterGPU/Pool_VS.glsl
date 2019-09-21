#version 330 core
precision highp float;

layout(location=0)in	vec4	a_position;
layout(location=1)in	vec3	a_fragCoord;

uniform		mat4	g_MVPMatrix;

out		vec3	v_fragCoord;
void	main()
{
	gl_Position = g_MVPMatrix * a_position;
	v_fragCoord = a_fragCoord;
}	