#version 330 core
//precision  highp  float;
uniform    mat4      u_mvpMatrix;
uniform    mat4      u_modelViewMatrix;
uniform    mat3      u_normalMatrix;
layout(location=0)in     vec4      a_position;
layout(location=1)in     vec2      a_texCoord;
layout(location=2)in     vec3      a_normal;
out     vec2    v_texCoord;
out     vec3    v_normal;
void    main()
{
	   v_normal=normalize(u_normalMatrix*a_normal);

       v_texCoord=a_texCoord;
       gl_Position=u_mvpMatrix*a_position;
}
