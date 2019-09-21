#version 330 core
uniform     vec3     u_lightPosition;
layout(location=0)out      float      point_distance;

in        vec3        v_position;

void  main()
{
	   point_distance = length(u_lightPosition - v_position);
}