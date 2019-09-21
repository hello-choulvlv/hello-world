#version 330 core
layout(location=0)out    vec4   outColor;
layout(location=1)out    vec4	outPosition;
layout(location=2)out    vec4   outNormal;

uniform	   vec4     g_LightColor;

in	vec4	v_position;
in	vec3	v_normal;

void	main()
{
	outColor = g_LightColor;
	outPosition = v_position;
	outNormal = vec4(normalize(v_normal),0.0);
}