#version 330 core
layout(location=0)in	vec4	a_position;
layout(location=1)in	vec2	a_fragCoord;

uniform		sampler2D	g_WaterHeightMap;
uniform		sampler2D	g_WaterNormalMap;
uniform		mat4		g_ModelMatrix;
uniform		float		g_GroundHeight;
uniform		float   	g_WaterHeight;
uniform		vec3		g_LightDirection;
uniform		vec2		g_Resolution;

out		vec2	v_fragCoord;

vec3	IntersectGround(vec3	position,vec3	direction)
{
	vec3	point=vec3(10000.0);
	float   sinValue = -direction.y;
	if(sinValue > 0.0)
	{
		float D = (g_WaterHeight - g_GroundHeight)/sinValue;
		point = position + direction * D;
	}
	return point;
}

void	main()
{
	vec3	normal = normalize(texture(g_WaterNormalMap,a_fragCoord).rgb);
	vec4    position = g_ModelMatrix * a_position;
	position.y += texture(g_WaterHeightMap,a_fragCoord).r;
	vec3	refractVec = refract(-g_LightDirection,normal,1.0/1.333);
	vec3	intersectPoint = IntersectGround(position.xyz,refractVec);
	//Scale to [-1.0,1.0]
	vec2	coord = vec2(intersectPoint.x,-intersectPoint.z)/g_Resolution ;//x:[-1,1],y:[0,2]
	coord.y -= 1.0;
	gl_Position = vec4(coord,0.0,1.0);
	v_fragCoord = coord *0.5 + 0.5 ;
}