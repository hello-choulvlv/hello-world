#version 330 core
precision highp float;
layout(location=0)out	vec4	outColor;
uniform     sampler2D  g_ShadowMap;
uniform     mat4    g_LightProjMatrix;
uniform     mat4    g_LightViewMatrix;
uniform		vec4	g_Color;
uniform		vec3    g_AmbientColor;
uniform		vec3	g_LightDirection;
uniform     float   g_LightBleeding;

in	vec4	v_position;
in	vec3	v_normal;

void	main()
{
	vec4   position = g_LightViewMatrix * v_position;
	float   length_l = -position.z;

	float   diffuse = max(0.0,dot(g_LightDirection,normalize(v_normal)));

	vec4	fragCoord = g_LightProjMatrix * position;
	vec2	movement = texture(g_ShadowMap,fragCoord.xy/fragCoord.w * 0.5 + 0.5).xy;

	float   f = step(length_l,movement.x);
	float   variance = max(movement.y - movement.x*movement.x,0.0);
	float   d = length_l - movement.x;
	float   e = max(f,variance/(variance + d*d));
	e = smoothstep(g_LightBleeding,1.0,e);

	outColor = vec4(g_Color.rgb* (g_AmbientColor + diffuse * 0.75 * e),g_Color.a);
}