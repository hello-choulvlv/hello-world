uniform                vec3    u_offset;
layout(location=0)in   vec4    a_position;
layout(location=1)in   vec2    a_texCoord;

out       vec2       v_texCoord;

void  main()
{

	    gl_Position=vec4(u_offset,0.0)+a_position;
	    v_texCoord=a_texCoord;
}