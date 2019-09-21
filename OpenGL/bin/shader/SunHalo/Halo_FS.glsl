#version 330 core
precision highp float;

layout(location=0)out	vec4	outColor;

uniform	sampler2D	g_BaseMap;
uniform	sampler2D   g_HighSunTexture;
uniform	sampler2D   g_LowSunTexture;
uniform float       g_Dispersal;
uniform float       g_HaloWidth;
uniform float       g_Intensity;
uniform vec2		g_SunProjPosition;
uniform vec3		g_Distortion;

in      vec2		v_fragCoord;

vec3	textureDistorted(sampler2D Texture,vec2	fragCoord,vec2 offset)
{
	return vec3(
		texture(Texture,fragCoord + offset * g_Distortion.r).r,
		texture(Texture,fragCoord + offset * g_Distortion.g).g,
		texture(Texture,fragCoord + offset * g_Distortion.b).b
		);
}

void main()
{
	//radial color blur sampler
	const float samples = 128.0;
	vec2  sampleCoord = v_fragCoord;
	vec2  stepSampleVec = (g_SunProjPosition - sampleCoord)/samples;
	vec3  radialColor = vec3(0.0);
	for(float r = 0.0; r < samples ; r += 1.0)
	{
		radialColor += texture(g_LowSunTexture,sampleCoord).rgb;
		sampleCoord += stepSampleVec;
	}
	radialColor /= samples;
	//flare halo sampler
	vec3	haloColor = vec3(0.0);
	vec2    flareOffset = vec2(0.0);
	sampleCoord = vec2(1.0) - v_fragCoord;
	stepSampleVec = (vec2(0.5) - sampleCoord) * g_Dispersal;

	for(int i=0; i < 5 ; ++i)
	{
		haloColor += textureDistorted(g_HighSunTexture,sampleCoord,flareOffset);
		flareOffset += stepSampleVec;
	}
	haloColor += textureDistorted(g_HighSunTexture,sampleCoord,normalize(stepSampleVec) * g_HaloWidth);
	haloColor /= 6.0;

	vec3	secondaryColor = texture(g_HighSunTexture,v_fragCoord).rgb + (radialColor+haloColor) * texture(g_BaseMap,v_fragCoord).rgb ;
	outColor = vec4(secondaryColor * g_Intensity,1.0);
}