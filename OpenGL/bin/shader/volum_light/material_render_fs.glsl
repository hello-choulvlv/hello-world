#version 330 core
precision highp float;
layout(location=0)out	vec4	outColor;

uniform		vec4	g_Color;
uniform		vec3	g_LightPosition;
uniform		vec3	g_LightColor;
uniform		vec3	g_AmbientColor;

varying		vec3	v_position;
varying		vec3	v_normal;

void	main()
{
	vec3	light_direction = normalize(g_LightPosition - v_position);
	vec3    normal = normalize(v_normal);
	float   diffuse = max(0.0,dot(light_direction,normal));

	outColor.rgb = g_Color.rgb * g_AmbientColor + g_Color.rgb * g_LightColor * diffuse;
	outColor.a = g_Color.a;
}