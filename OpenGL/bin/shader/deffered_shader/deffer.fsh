#version 330 core
precision  highp   float;

uniform    sampler2D     u_baseMap;
layout(location=0)out       vec3     out_position;
layout(location=1)out       vec3     out_normal;
layout(location=2)out       vec4     out_color;

in         vec3       v_position;
in         vec3       v_normal;
in         vec2       v_fragCoord;

void   main()
{
	  out_color = texture(u_baseMap,v_fragCoord);

	  out_normal = v_normal;

	  out_position = v_position;
}