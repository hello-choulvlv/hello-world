#version 330 core
precision highp float;

layout(location=0)out	vec4	outColor;

uniform		sampler2D	g_BaseMap;
uniform		float		g_MeshInterval;
in		vec2	v_fragCoord;

void	main()
{
	vec2	pixelSize = 1.0/textureSize(g_BaseMap,0);
	float	r = texture(g_BaseMap,v_fragCoord).r;

	outColor = vec4(
				r - texture(g_BaseMap,v_fragCoord - vec2(pixelSize.x,0.0)).r,
				g_MeshInterval,
				r - texture(g_BaseMap,v_fragCoord - vec2(0.0,pixelSize.y)).r,
				0.0
			);
}