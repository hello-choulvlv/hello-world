#version 330 core
uniform   mat4   u_lightMatrix;

layout(location=0)in     vec4     a_position;

void   main()
{
	  gl_Position=u_lightMatrix*a_position;
}