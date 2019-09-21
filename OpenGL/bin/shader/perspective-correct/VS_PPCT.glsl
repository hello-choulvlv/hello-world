#version 330 core
precision highp float;

uniform  mat4   g_MVPMatrix;

layout(location=0)vec4		a_position;
layout(location=1)vec4		a_color;

out		vec4	v_color;
out     float   v_ppcCoeffcient;

void    main()
{
	gl_Position = g_MVPMatrix * a_position;
	v_ppcCoeffcient = 1.0/a_position.z;
	v_color = a_color * v_ppcCoeffcient;
}