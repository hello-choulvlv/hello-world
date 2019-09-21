#version 330 core
precision highp float;

layout(location=0)out		vec4		outColor;

uniform		vec4		g_Color;

void	main()
{
	outColor = g_Color;
}