#version 330 core
//precision   highp    float;
uniform     mat4     u_modelMatrix;
layout(location=0)in     vec4     a_position;
void    main()
{
	gl_Position = u_modelMatrix * a_position;
}