#version 330 core
precision highp float;

layout(location=0)out	vec4	outColor;

in		vec4	v_color;
in 		float   v_ppcCoefficient;

void   main()
{
	outColor = v_color/v_ppcCoefficient;
}