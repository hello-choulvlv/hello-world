#version 330 core
layout(location=0)in		vec4		a_position;
layout(location=1)in		vec3        a_normal;

uniform		mat4 	g_modelMatrix;
uniform     mat4    g_viewProjMatrix;
uniform     mat3    g_normalMatrix;

out      vec3		v_position;
out      vec3       v_normal;

void    main()
{
	vec4  position = g_modelMatrix * a_position;
	v_position = position.xyz;
	gl_Position = g_viewProjMatrix * position;
	v_normal = g_normalMatrix * a_normal;
}
