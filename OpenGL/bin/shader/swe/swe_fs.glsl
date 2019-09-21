#version 330 core
layout(location=0)out	vec4	outColor;
uniform		sampler2D	g_MainTexture;
uniform		sampler2D	g_SecondaryTexture;
uniform     sampler2D	g_NormalTexture;

in	vec2	v_fragCoord;

void	main()
{
	vec4	frag_coord = texture(g_NormalTexture,v_fragCoord);
	vec4	color1 = texture(g_MainTexture,frag_coord.xy);
	vec4	color2 = texture(g_SecondaryTexture,frag_coord.zw);

	outColor = color1 * 0.6 + color2 * 0.6;
}