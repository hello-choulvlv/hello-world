#version 330 core
precision highp float;

layout(location=0)in	vec4	a_position;
layout(location=1)in	vec3	a_normal;
layout(location=2)in	vec2	a_fragCoord;

uniform		mat4	g_MMatrix;
uniform		mat4	g_ViewProjMatrix;
uniform		mat3	g_NormalMatrix;

out		vec4	v_position;
out		vec3	v_normal;
out		vec2	v_fragCoord;

void 	main()
{
	v_position = g_MMatrix * a_position;
	gl_Position = g_ViewProjMatrix * v_position;
	v_normal = g_NormalMatrix * a_normal;
	v_fragCoord = a_fragCoord;
}