#version 330 core
layout(location=0)out	vec4	outColor;
uniform		sampler2DShadow  g_ShadowMap;
uniform		mat4	g_LightViewProjMatrix;
uniform		vec3	g_LightDirection;
uniform		vec3	g_AmbientColor;
uniform		vec4	g_Color;

in	vec3	v_normal;
in	vec3	v_position;

void	main()
{
	vec4  fragCoord = g_LightViewProjMatrix * vec4(v_position,1.0);
	float shadow_f = textureProj(g_ShadowMap,fragCoord * 0.5 + 0.5);
	float diffuse = max(0.0,dot(g_LightDirection,normalize(v_normal)));
	outColor.rgb = g_Color.rgb * (g_AmbientColor + diffuse * shadow_f);
	outColor.a = g_Color.a;
}