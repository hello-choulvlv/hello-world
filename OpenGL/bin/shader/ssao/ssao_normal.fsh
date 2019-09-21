#version 330 core
precision  highp   float;
uniform    sampler2D     u_baseMap;
uniform    vec2          u_nearFarPlane;
layout(location=0)out       vec4     out_position;
layout(location=1)out       vec3     out_normal;
layout(location=2)out       vec4     out_color;

in         vec3       v_position;
in         vec3       v_normal;
in         vec2       v_fragCoord;

void   main()
{
	  out_color = texture(u_baseMap,v_fragCoord);

	  out_normal = normalize(v_normal);

	  float     _alpha = gl_FragCoord.z*2.0 - 1.0; //back to ndc
	  _alpha = -0.5*((u_nearFarPlane.y - u_nearFarPlane.x)*v_position.z + (u_nearFarPlane.x + u_nearFarPlane.x));
	  out_position = vec4(v_position,_alpha);
}