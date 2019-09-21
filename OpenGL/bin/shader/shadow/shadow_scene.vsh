#version 330 core
uniform   mat4     u_mvpMatrix;
uniform   mat4     u_lightMatrix;
uniform   mat3     u_normalMatrix;

layout(location=0)in   vec4    a_position;
layout(location=1)in   vec3    a_normal;
layout(location=2)in   vec2    a_texCoord;

out     vec4       v_shadowCoord;
out     vec3       v_normal;
out     vec2       v_texCoord;
void  main()
{
      gl_Position=u_mvpMatrix*a_position;
      v_shadowCoord=u_lightMatrix*a_position;
      v_shadowCoord=v_shadowCoord*0.5+0.5;
	  v_texCoord=a_texCoord;

      v_normal=normalize(u_normalMatrix*a_normal);
}