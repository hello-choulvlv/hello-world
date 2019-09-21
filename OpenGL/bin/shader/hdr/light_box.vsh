#version 330 core
//precision  highp     float;
uniform    mat4      u_mvpMatrix;

layout(location=0)in      vec4       a_position;

void   main()
{
	gl_Position = u_mvpMatrix * a_position;
}
