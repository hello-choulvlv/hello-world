#version 330 core
precision	highp float;
layout(location=0)out	vec4	outColor;
uniform		sampler2D	g_BaseTexture;
uniform		vec2		g_PixelStep;
uniform		float		g_Weights[6];

in	vec2	v_fragCoord;

void	main()
{
	vec4	color = texture(g_BaseTexture,v_fragCoord + g_PixelStep * 2.0) * g_Weights[4];
	color += texture(g_BaseTexture,v_fragCoord + g_PixelStep) * g_Weights[3];
	color += texture(g_BaseTexture,v_fragCoord) * g_Weights[2];
	color += texture(g_BaseTexture,v_fragCoord - g_PixelStep) * g_Weights[1];
	color += texture(g_BaseTexture,v_fragCoord - g_PixelStep * 2) * g_Weights[0];

	outColor = color * g_Weights[5];
}