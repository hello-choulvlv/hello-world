#version 330 core
layout(location=0)out		vec4		outColor;
uniform		sampler2D		g_BaseMap;
uniform		float			g_Width;
uniform		vec2			g_Direction;

in		vec2		v_fragCoord;

void    main()
{
	float    tipWidth = g_Width +1.0;
	outColor = vec4(0.0);
	float    sum = 0.0;
	for(float x = -g_Width; x <= g_Width; x += 1.0)
	{
		float width = tipWidth - abs(x);
		outColor.rgb += texture(g_BaseMap,v_fragCoord + g_Direction * vec2(x)).rgb * width;
		sum += width;
	}
	outColor.rgb /= sum;
	outColor.a = texture(g_BaseMap,v_fragCoord).a;
}