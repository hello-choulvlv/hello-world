#version 330 core
layout(location=0)out	float	outColor;
uniform		sampler2D	g_BaseMap;
uniform		vec2		g_FuzzyVec;
in		    vec2	    v_fragCoord;

void	main()
{
	float color=0.0;
	float totalWeight = 0;
	for(int j = -10; j <= 10; ++j)
	{
		float step = float(j);
		float weight = 11.0 - abs(step);
		totalWeight += weight;
		color += texture(g_BaseMap,v_fragCoord + step*g_FuzzyVec).r*weight;
	}
	outColor=color/totalWeight;
}
