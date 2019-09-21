#version 330 core
precision highp float;
layout(location=0)in         vec4        a_position;
layout(location=1)in         vec2        a_fragCoord;
layout(location=2)in         vec3        a_normal;
uniform     mat4     u_modelMatrix;
uniform     mat4     u_viewProjMatrix;
uniform     mat3     u_normalMatrix;

out       vec2       v_fragCoord;
out       vec3       v_normal;
out       vec3       v_position;

void   main()
{
      vec4     _position = u_modelMatrix * vec4(a_position.xy,0.0,1.0);
      gl_Position = u_viewProjMatrix * _position;
      v_position = _position.xyz;
      v_normal = u_normalMatrix * vec3(a_normal.xy,1.0);
      v_fragCoord = a_fragCoord;
}