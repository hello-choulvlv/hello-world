#version 330 core
precision highp float;

layout(location=0)out	vec4	outColor;

uniform		sampler2D	g_NormalMap;
uniform		samplerCube g_TexCubeMap;
uniform		vec3		g_CubeMapNormal[6];
uniform		vec3		g_CameraPosition;
uniform		vec3		g_LightPosition;
uniform		float		g_HalfCubeHeight;
uniform		mat4		g_NormalMatrix;

in	vec3	v_position;
in	vec2	v_fragCoord;

vec3	IntersectCubeMap(vec3	position,vec3	normal)
{
	vec3	targetPoint;
	for(int i=0;i<6;++i)
	{
		float angle = -dot(g_CubeMapNormal[i],normal);
		if( angle > 0 )
		{
			float D = dot(g_CubeMapNormal[i],position) + g_HalfCubeHeight;
			D /= angle;

			if(D >= 0.0)
			{
				targetPoint = position + normal * D;
				break;
			}
		}
	}
	return targetPoint;
}

void	main()
{
	vec3	normal = normalize(texture(g_NormalMap,v_fragCoord).rgb);
	vec3	cameraDirection = normalize(v_position - g_CameraPosition );
	//check Camera's position
	if(g_CameraPosition.y > 0)
	{
		vec4	reflectColor = texture(g_TexCubeMap,IntersectCubeMap(v_position,reflect(cameraDirection,normal)));
		vec4	refractColor = texture(g_TexCubeMap,IntersectCubeMap(v_position,refract(cameraDirection,normal,1.0/1.33)));
		vec3	lightDirection = normalize(g_LightPosition - v_position);
		float   dotValue = dot(reflect(-lightDirection,normal),-cameraDirection);
		float   specular = pow(max(dotValue,0.0),128.0);
		outColor = mix(reflectColor,refractColor,-dot(cameraDirection,normal)) + specular;
	}
	else
	{
		normal = -normal;
		//refract vector
		vec3	refractVec = refract(cameraDirection,normal,1.33);
		vec4	reflectColor = texture(g_TexCubeMap,IntersectCubeMap(v_position,reflect(cameraDirection,normal)));
		if( dot(refractVec,refractVec) <= 0.0)
			outColor = reflectColor;
		else
		{
			vec4	refractColor = texture(g_TexCubeMap,IntersectCubeMap(v_position,refractVec));
			outColor = mix(reflectColor,refractColor,-dot(cameraDirection,normal));
		}
	}
}