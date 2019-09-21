#version 330 core
precision  highp  float;
layout(location=0)in      vec4        a_position;
layout(location=1)in      vec2        a_texCoord;

out       vec2       v_texCoord;
out       vec2       v_position;
void    main()
{
	    gl_Position=a_position;
	    v_texCoord=a_texCoord;
	    v_position=a_position.xy;
}
