#version 330 core
//precision highp float;
uniform  mat4   u_mvpMatrix;
layout(location=0)in   vec4     a_position;
layout(location=1)in   vec3     a_texCoord;

out    vec3     v_texCoord;
void   main()
{
	gl_Position=u_mvpMatrix*a_position;
	gl_Position.z=gl_Position.w;
	v_texCoord=a_texCoord;
}