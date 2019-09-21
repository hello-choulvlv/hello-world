#version 330 core
precision highp float;
layout(location=0)out	vec4	outColor;
uniform		sampler2D	g_RayTexture;
uniform		vec2       	g_RayLengthVec2;
uniform		vec2		g_LightPositionScreen;

in	vec2	v_fragCoord;

void	main()
{
	vec2	ray_vector = g_LightPositionScreen - (v_fragCoord * 2.0 - 1.0) * vec2(g_RayLengthVec2.y,1.0);
	ray_vector *= g_RayLengthVec2.x;

	vec4	color = texture(g_RayTexture,v_fragCoord);
	color += texture(g_RayTexture,v_fragCoord + ray_vector * 0.2);
	color += texture(g_RayTexture,v_fragCoord + ray_vector * 0.4);
	color += texture(g_RayTexture,v_fragCoord + ray_vector * 0.6);
	color += texture(g_RayTexture,v_fragCoord + ray_vector * 0.8);
	color += texture(g_RayTexture,v_fragCoord + ray_vector * 1.0);

	outColor = color * 0.166666666;
}