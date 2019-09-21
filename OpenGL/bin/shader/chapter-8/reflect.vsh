precision highp  float;
uniform   vec3     u_normal;
uniform   mat4     u_mvMatrix;
layout(location=0)in      vec4       a_position;
layout(location=1)in      vec2       a_texCoord;

out         vec2         v_texCoord;
out         vec3         v_position;
out         vec3         v_normal;
 void  main()
 {
 	   mat3          nMatrix;
 	   nMatrix[0]=normalize(u_mvMatrix[0].xyz);
 	   nMatrix[1]=normalize(u_mvMatrix[1].xyz);
 	   nMatrix[2]=normalize(u_mvMatrix[2].xyz);
       v_normal=normalize(nMatrix*u_normal);

       vec4          _position=u_mvMatrix*a_position;
       v_position.xyz=_position.xyz/_position.w;
       v_texCoord=a_texCoord;

       gl_Position=_position;
 }