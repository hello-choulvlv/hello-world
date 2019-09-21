#version 330 core
precision highp float;

layout(location = 0)in		vec4		a_position;
layout(location = 1)in		vec3		a_offset;
layout(location = 2)in		vec3		a_normal;
//layout(location = 2)in		vec2		a_fragCoord;

uniform		mat4		u_modelMatrix;
uniform		mat4		u_viewProjMatrix;
uniform		mat3		u_normalMatrix;

out			vec3		v_position;
out			vec3		v_normal;
//out			vec2		v_fragCoord;

void	main()
{
	vec4	 _position = u_modelMatrix * (a_position +vec4(a_offset,0.0));
	v_position = _position.xyz;

	gl_Position = u_viewProjMatrix * _position;
	v_normal = u_normalMatrix * a_normal;
//	v_fragCoord = a_fragCoord;
}