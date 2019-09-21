#version 330 core
precision highp   float;
uniform   mat4    u_mvpMatrix;
uniform   mat3    u_normalMatrix;
uniform   vec3    u_lightVector;
layout(location=0)in      vec4      a_position;
layout(location=1)in      vec2      a_texCoord;
layout(location=2)in      vec3      a_normal;
layout(location=3)in      vec3      a_tangent;

out    vec2      v_texCoord;
smooth out    mat3      v_tbnMatrix;

void   main()
{

	  vec3   vNormal=normalize(u_normalMatrix*a_normal);
	  vec3   vTangent=a_tangent;
	  vTangent=normalize(u_normalMatrix*vTangent);
//binormal
      vec3   vBinormal=cross(vNormal,vTangent);
      
      v_tbnMatrix=mat3(vTangent,vBinormal,vNormal);

      gl_Position=u_mvpMatrix*a_position;
	  v_texCoord=a_texCoord;
}
