#version 330 core
//precision  highp  float;
uniform                  mat4      u_mvpMatrix;
uniform                  mat4      u_mvMatrix;
layout(location=0)in     vec4      a_position;
layout(location=1)in     vec2      a_texCoord;
layout(location=2)in     vec3      a_normal;
out     vec2    v_texCoord;
out     vec3    v_normal;
out     vec3    v_specularDir;
out     vec3    v_position;
void    main()
{
	mat3    nMatrix;
	nMatrix[0]=normalize(u_mvMatrix[0].xyz);
	nMatrix[1]=normalize(u_mvMatrix[1].xyz);
	nMatrix[2]=normalize(u_mvMatrix[2].xyz);
	v_normal=normalize(nMatrix*a_normal);

       vec4       _position=u_mvMatrix*a_position;;
       v_position=_position.xyz/_position.w;
       
       v_texCoord=a_texCoord;
       gl_Position=u_mvpMatrix*a_position;
}
