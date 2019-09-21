#version 330 core

layout(location=0)out	vec4	outColor;

uniform		vec3		g_Kernel;
uniform		samplerCube	g_CubeMap;

in			vec3		v_fragCoord;

void	main()
{
	outColor = vec4(0.0);
	for(float x = -3.0 ; x <= 3.0 ; x+=1.0)
	{
		outColor.rgb += texture(g_CubeMap,v_fragCoord + g_Kernel * x).rgb *(4.0 - abs(x));
	}
	outColor.rgb /= 16.0;
}