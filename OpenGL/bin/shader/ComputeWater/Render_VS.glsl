#version 330 core

layout(location=0)in	vec4	a_position;
layout(location=1)in	vec3	a_normal;

uniform		mat4		g_ViewProjMatrix;
uniform		mat3		g_NormalMatrix;
uniform     mat4        g_ModelMatrix;

out		vec3	v_position;
out		vec3	v_normal;

void    main()
{
	gl_Position = g_ModelMatrix * a_position;
	v_position = gl_Position.xyz;
	gl_Position = g_ViewProjMatrix * gl_Position;
	v_normal = a_normal;
}