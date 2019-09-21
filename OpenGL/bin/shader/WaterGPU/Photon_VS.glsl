#version 330 core
precision highp float;

layout(location=0)in	vec4	a_position;
layout(location=1)in	vec2	a_fragCoord;

uniform		mat4		g_PhotonsWorldToTextureMatrix[6];
uniform		vec3		g_CubeMapNormals[6];
uniform		vec3		g_LightPosition;

uniform		sampler2D	g_WaterHeightMap;
uniform		sampler2D	g_WaterNormalMap;
uniform		float		g_HalfCubeMapWidth;


out		vec3	v_fragCoord;

vec3	IntersectCubeMap(vec3	position,vec3	direction)
{
	vec3	point;
	for(int i=0;i<6;++i)
	{
		float dotValue = -dot(direction,g_CubeMapNormals[i]);
		if( dotValue >0.0 )
		{
			float D = ( dot(g_CubeMapNormals[i],position) + g_HalfCubeMapWidth )/dotValue;
			if(D > 0.0)
			{
				point = position + direction * D;
				vec3 absPoint = abs(point);
				if(absPoint.x <= g_HalfCubeMapWidth && absPoint.y <= g_HalfCubeMapWidth && absPoint.z <= g_HalfCubeMapWidth)
					break;
			}
		}
	}
	return point;
}

void	main()
{
	vec4	position = a_position;
	position.y += texture(g_WaterHeightMap,a_fragCoord).r;
	vec3	normal = normalize(texture(g_WaterNormalMap,a_fragCoord).rgb);

	vec3	lightDirection = normalize(position.xyz - g_LightPosition);
	vec3	refractVec = refract(lightDirection,normal,1.0/1.33);

	vec3	targetPoint = IntersectCubeMap(position.xyz,refractVec);
	//judge
	float	axisPoint[6] = float[](targetPoint.x,-targetPoint.x,targetPoint.y,-targetPoint.y,targetPoint.z,-targetPoint.z);
	int     slice=0;
	float   sliceValue = targetPoint.x;
	for(int i=1; i < 6; ++i)
	{
		if(sliceValue < axisPoint[i])
		{
			slice = i;
			sliceValue = axisPoint[i];
		}
	}
	// to texture a_fragCoord
	vec4 tpos = g_PhotonsWorldToTextureMatrix[slice] * vec4(targetPoint,1.0);
	v_fragCoord = tpos.xyz;
	gl_Position = tpos * 2.0 -1.0;
}