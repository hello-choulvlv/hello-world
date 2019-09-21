#version 330 core
layout(location=0)out vec4 outColor;
uniform sampler2D g_BaseMap;
uniform vec3      g_LightPosition;
uniform vec3      g_LightColor;

in	vec2  v_fragCoord;
in	vec3  v_position;
in	vec3  v_normal;

void main()
{
	outColor = texture(g_BaseMap,v_fragCoord);
	vec3 lightDirection = normalize(g_LightPosition - v_position);
	vec3 normal = normalize(v_normal);
	float diffuse = max(0.0,dot(normal,lightDirection));
	//no shadow in current version
	outColor.rgb *=g_LightColor *0.3 + 0.7 *diffuse;
}