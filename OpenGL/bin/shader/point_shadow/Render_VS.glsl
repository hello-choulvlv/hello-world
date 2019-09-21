#version 330 core
layout(location=0)in	vec4	a_position;
layout(location=1)in	vec2	a_fragCoord;
layout(location=2)in	vec3	a_normal;

uniform	mat4 g_MVPMatrix;
uniform mat4 g_ModelMatrix;

out vec2 v_fragCoord;
out vec3 v_position;
out vec3 v_normal;

void main()
{
	vec4 position = g_ModelMatrix * a_position;
	gl_Position  = g_MVPMatrix * position;
	v_position = position.xyz;

	v_fragCoord = a_fragCoord;
	vec4 normal = g_ModelMatrix *vec4(a_normal,0.0);
	v_normal = normal.xyz;
}