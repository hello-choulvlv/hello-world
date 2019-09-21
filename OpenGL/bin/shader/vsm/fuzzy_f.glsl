#version 330 core
layout(location=0)out	vec2	outColor;
uniform		sampler2D	u_BaseMap;
uniform		vec2		u_StepInterval;
uniform     vec2        u_TextureStep;
uniform		int         u_SampleCount;

in		vec2	v_fragCoord;

void	main()
{
	vec2    stepPixel = u_StepInterval * u_TextureStep;
	vec2    offset = float(u_SampleCount-1)*0.5 * stepPixel;
	vec2    baseFragCoord = v_fragCoord - offset;

	outColor = vec2(0.0);
	for(int k = 0; k < u_SampleCount; ++ k)
	{
		outColor += texture(u_BaseMap,baseFragCoord + float(k)*stepPixel).xy;
	}
	outColor /= float(u_SampleCount);
}