#version 330 core
layout(location=0)in	vec4	a_position;
layout(location=1)in	vec3	a_normal;

uniform		mat4	g_ModelMatrix;
uniform 	mat4	g_ViewProjMatrix;
uniform		mat3	g_NormalMatrix;

varying		vec3	v_position;
varying		vec3	v_normal;

void	main()
{
	vec4	position = g_ModelMatrix * a_position;
	gl_Position = g_ViewProjMatrix * position;
	v_position = position.xyz;

	v_normal = g_NormalMatrix * a_normal;
}