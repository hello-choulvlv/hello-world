#version 330 core
//precision highp float;
uniform    mat4     u_mvpMatrix;
uniform    mat4     u_modelMatrix;
uniform    mat3     u_normalMatrix;

layout(location=0)in       vec4       a_position;
layout(location=1)in       vec2       a_fragCoord;
layout(location=2)in       vec3       a_normal;

out        vec3       v_position;
out        vec3       v_normal;
out        vec2       v_fragCoord;

void    main()
{
	gl_Position = u_mvpMatrix * a_position;
	v_position = (u_modelMatrix * a_position).xyz;
	v_normal = u_normalMatrix * a_normal;
	v_fragCoord = a_fragCoord;
}