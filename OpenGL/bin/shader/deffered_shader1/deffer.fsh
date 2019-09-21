#version 330 core
//precision  highp   float;

uniform    sampler2D     u_baseMap;
layout(location=0)out       vec4     out_color;
layout(location=1)out       vec4     out_position;
layout(location=2)out       vec4     out_normal;


in         vec3       v_position;
in         vec3       v_normal;
in         vec2       v_fragCoord;

void   main()
{
	  out_color = texture(u_baseMap,v_fragCoord);

	  out_normal = vec4(v_normal,0.0);

	  out_position =vec4( v_position,0.0);
}