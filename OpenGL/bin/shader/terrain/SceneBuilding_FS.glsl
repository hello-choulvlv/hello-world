#version 330 core
precision highp float;

layout(location=0)out	vec4	outColor;

uniform	sampler2D g_Texture;
uniform vec3	  g_lightPosition;

in	vec2	v_fragCoord;
in	vec3	v_normal;
in	vec3	v_position;

void main()
{
	outColor = texture(g_Texture,v_fragCoord);

	vec3	lightDirection = normalize(g_lightPosition - v_position);
	vec3    normal = normalize(v_normal);

	float   diffuse = max(0.0,dot(normal,lightDirection));

	outColor.rgb *= 0.5 + 0.5 *diffuse;
}