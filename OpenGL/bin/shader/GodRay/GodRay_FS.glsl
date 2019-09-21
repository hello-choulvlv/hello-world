#version 330 core
layout(location=0)out		vec4	outColor;

uniform		vec3	g_EyePosition;
uniform     vec4    g_Color;

in		vec3	v_position;
in		vec3	v_normal;

void main()
{
	vec3 eyeVector = normalize(g_EyePosition - v_position);
	vec3 normal = normalize(v_normal);
	vec3 reflectVector = reflect( -eyeVector,normal);

	outColor = vec4(g_Color.rgb *max(dot(eyeVector,normal),0.0) + pow(max(0.0,dot(eyeVector,reflectVector)),32.0) ,g_Color.a);
}