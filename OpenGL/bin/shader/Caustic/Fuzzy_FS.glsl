#version 330 core
layout(location=0)out	vec4	outColor;
uniform		sampler2D	g_BaseMap;
uniform		vec2        g_PixelStep;

in	vec2	v_fragCoord;

void	main()
{
	outColor = vec4(0.0);
	for(float x= -3.0 ; x<= 3.0 ; x += 1.0)
	{
		outColor.rgb += texture(g_BaseMap,v_fragCoord + x * g_PixelStep).rgb * (4.0 - abs(x));
	}
	outColor.rgb /= 16.0;
}