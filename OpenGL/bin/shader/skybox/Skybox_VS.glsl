#version 330 core
layout(location=0)in	vec4	a_position;
//layout(location=1)in	vec3	a_fragCoord;

uniform	mat4 g_MVPMatrix;
uniform vec3 g_CameraPosition;

out		vec3	v_fragCoord;

void main()
{
	gl_Position = g_MVPMatrix * vec4(a_position.xyz + g_CameraPosition,a_position.w);
	gl_Position.z = gl_Position.w;
	v_fragCoord = a_position.xyz;//vec3(a_position.x,-a_position.yz);
}