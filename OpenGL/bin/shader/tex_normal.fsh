#version 330 core
precision    highp     float;
uniform      sampler2D       u_baseMap;
layout(location=0)out       vec4          outColor;

in        vec2       v_fragCoord;

void    main()
{
	outColor = texture(u_baseMap,v_fragCoord);
}