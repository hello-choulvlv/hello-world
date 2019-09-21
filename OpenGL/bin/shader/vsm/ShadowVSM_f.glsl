#version 330 core
layout(location=0)out	vec2	outColor;
//uniform		vec3	g_LightPosition;
uniform	mat4	g_LightViewMatrix;
in		vec4	v_position;

void	main()
{
	vec4	position = g_LightViewMatrix * v_position;
	float   length_l = -position.z;//length(v_position - g_LightPosition);
	outColor = vec2(length_l,length_l * length_l);
}