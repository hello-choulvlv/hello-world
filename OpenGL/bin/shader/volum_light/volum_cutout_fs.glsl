#version 330 core
precision highp float;
layout(location=0)out	vec4	outColor;
uniform	sampler2D	g_DepthTexture;
uniform	sampler2D	g_ColorTexture;
uniform	vec4		g_OccluderParams;
uniform	vec4		g_NearQFar;
uniform	vec2		g_LightPositionScreen;

in	vec2	v_fragCoord;

void  main()
{
	float  depth = texture(g_DepthTexture,v_fragCoord).r;// * 2.0 - 1.0;

	vec2   screen_coord = v_fragCoord * 2.0 -1.0;
	screen_coord *= vec2(g_NearQFar.z,1.0);
	vec2   ray_direction = g_LightPositionScreen - screen_coord;
	vec4   color = texture(g_ColorTexture,v_fragCoord + ray_direction);

	float  world_depth = g_NearQFar.y/(depth + g_NearQFar.x);
	float  attenuation = smoothstep(g_OccluderParams.x,g_OccluderParams.y,world_depth);

	outColor = color * attenuation;
}