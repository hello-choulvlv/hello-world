#version 330 core
layout(location=0)in	vec4	a_position;
uniform	mat4	g_ModelMatrix;

void main()
{
	gl_Position = g_ModelMatrix * a_position;
}