#version 330 core
layout(location=0)in	vec4	a_position;
layout(location=1)in	vec2	a_fragCoord;

uniform		mat4	g_ModelMatrix;
uniform		mat4	g_ViewProjMatrix;
uniform		vec3	g_CameraPosition;
out		vec2	v_fragCoord;

void	main()
{
	vec4	position = g_ModelMatrix * a_position;
	gl_Position = g_ViewProjMatrix * vec4(g_CameraPosition + position.xyz,position.w);
	v_fragCoord = a_fragCoord;
	gl_Position.z = gl_Position.w;
}

