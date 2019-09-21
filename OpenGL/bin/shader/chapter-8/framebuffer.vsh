precision highp float;
uniform            mat4           u_mvMatrix;
layout(location=0)in     vec4     a_position;
layout(location=1)in     vec3     a_normal;
layout(location=2)in     vec2     a_texCoord;

out      vec2     v_texCoord;
out      vec3     v_normal;
void   main()
{
	  v_texCoord=a_texCoord;
      mat3   nMatrix;
      nMatrix[0]=normalize(u_mvMatrix[0].xyz);
      nMatrix[1]=normalize(u_mvMatrix[1].xyz);
      nMatrix[2]=normalize(u_mvMatrix[2].xyz);

      v_normal=normalize(nMatrix*a_normal);

      gl_Position=u_mvMatrix*a_position; 
}