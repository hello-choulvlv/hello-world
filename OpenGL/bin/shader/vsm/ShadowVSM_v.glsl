#version 330 core
layout(location=0)in	vec4	a_position;
uniform     mat4    g_ModelMatrix;
uniform     mat4    g_ViewProjMatrix;

out		vec4	v_position;
void    main()
{
	vec4 position = g_ModelMatrix * a_position;
	gl_Position = g_ViewProjMatrix * position;
	v_position = position;
}

