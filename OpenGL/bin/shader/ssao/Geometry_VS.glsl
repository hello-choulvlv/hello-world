#version 330 core
layout(location=0)in	vec4	a_position;
layout(location=1)in	vec3	a_normal;

uniform	mat4	g_ModelViewMatrix;
uniform mat4    g_ProjMatrix;
uniform mat3    g_NormalViewMatrix;

out vec4	v_position;
out	vec3	v_normal;

void    main()
{
	vec4	position = g_ModelViewMatrix * a_position;
	v_position = position;
	gl_Position = g_ProjMatrix * position;

	v_normal = g_NormalViewMatrix * a_normal;
}