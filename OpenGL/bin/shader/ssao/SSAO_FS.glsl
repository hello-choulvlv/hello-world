#version 330 core
layout(location=0)out	float	outEfficient;
uniform		sampler2D	g_PositionTexture;
uniform		sampler2D	g_NormalTexture;
uniform		sampler2D	g_TangentTexture;
uniform		mat4		g_ProjMatrix;
uniform		mat3		g_NormalViewMatrix;
uniform		vec3		g_Kernel[32];

in		vec2	v_fragCoord;
void	main()
{
	vec3	position = texture(g_PositionTexture,v_fragCoord).xyz;
	vec3	normal   = texture(g_NormalTexture,v_fragCoord).xyz;
	vec3	tangent  = texture(g_TangentTexture,v_fragCoord).xyz;
	//
	tangent = g_NormalViewMatrix * normalize(tangent - normal*dot(normal,tangent));
	vec3	binormal = cross(normal,tangent);
	mat3    TBN = mat3(tangent,binormal,normal);
	//
	float   efficient = 0.0;
	const float radius = 4.0;
	for(int i=0; i < 32 ; ++i)
	{
		vec3  newPosition = position + TBN * g_Kernel[i] * radius;
		vec4  projPosition = g_ProjMatrix * vec4(newPosition,1.0);
		float depth = texture(g_PositionTexture,projPosition.xy/projPosition.w*0.5+0.5).z;

		efficient += (depth > newPosition.z?1.0:0.0) * smoothstep(0.0,1.0,radius/abs(depth - position.z)) ;
	}
	outEfficient = 1.0 - efficient/32.0;
}