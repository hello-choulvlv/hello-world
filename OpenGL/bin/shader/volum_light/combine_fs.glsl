#version 330 core
layout(location=0)out	vec4	outColor;
uniform		sampler2D	g_BaseTexture;
uniform		sampler2D	g_RayTexture;
uniform		vec4		g_RayColor;
uniform		float       g_RayOpacity;

in	vec2	v_fragCoord;

void	main()
{
	vec4	color = texture(g_BaseTexture,v_fragCoord);
	vec4	ray_color = texture(g_RayTexture,v_fragCoord) * g_RayColor * g_RayOpacity;

	outColor.rgb = color.rgb + ray_color.rgb;
	outColor.a = color.a;
}