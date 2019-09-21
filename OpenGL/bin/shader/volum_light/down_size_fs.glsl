#version 330 core
layout(location=0)out	vec4	outColor;
uniform		sampler2D	g_BaseTexture;

in	vec2	v_fragCoord;

void  main()
{
	outColor = texture(g_BaseTexture,v_fragCoord);
}