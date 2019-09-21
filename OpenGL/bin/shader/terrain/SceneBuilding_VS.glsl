#version 330 core

layout(location=0)in	vec4	a_position;
layout(location=1)in	vec3	a_normal;
layout(location=2)in	vec3	a_fragCoord;

uniform mat4 g_MVPMatrix;

out		vec2	v_fragCoord;
out     vec3    v_normal;
out     vec3	v_position;

void main()
{
	gl_Position = g_MVPMatrix * a_position;
	v_fragCoord = a_fragCoord.xy;
	v_normal = a_normal;
	v_position = a_position.xyz;
}