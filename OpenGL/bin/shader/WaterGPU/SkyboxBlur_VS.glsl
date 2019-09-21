#version 330 core
layout(location=0)in	vec4	a_position;
layout(location=1)in	vec3	a_fragCoord;

out		vec3	v_fragCoord;

void	main()
{
	gl_Position	= a_position;//vec4(a_position.x,-a_position.y,a_position.z,a_position.w);
	v_fragCoord = a_fragCoord;// vec3(a_fragCoord.x,-a_fragCoord.yz);
}