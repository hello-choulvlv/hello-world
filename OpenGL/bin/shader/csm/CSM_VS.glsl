#version 440 core
precision highp float;
layout(location=0)in	vec4	a_position;

uniform		mat4	u_mvpMatrix;

void	main()
{
	gl_Position = u_mvpMatrix * a_position;
}