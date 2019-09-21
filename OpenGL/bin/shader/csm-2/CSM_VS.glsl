#version 330 core
precision highp float;

layout(location=0)in	vec4	a_position;

uniform		mat4	g_MVPMatrix;

void 	main()
{
	gl_Position = g_MVPMatrix * a_position;
}