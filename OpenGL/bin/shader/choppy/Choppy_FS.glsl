#version 330 core
layout(location=0)out	vec4	outColor;
uniform		sampler2D   	g_Texture;
uniform		samplerCube		g_EnvCubeMap;
uniform		samplerCube     g_BottomCubeMap;

in		vec2	v_fragCoord;
//in		vec3	v_normal;
in      vec3	v_reflectVec;
in      vec3    v_refractVec;
in		vec3	v_lightDirection;
in		float   v_freshnel;

void	main()
{
	const vec4 LMd =vec4(0.7,0.78,0.91,1.0);

	vec4	reflectColor = texture(g_EnvCubeMap,v_reflectVec);
	vec4    refractColor = texture(g_BottomCubeMap,v_refractVec);

	vec3	normal = texture(g_Texture,v_fragCoord).xyz;
	vec3    lightDirection = normalize(v_lightDirection);
	float   dotValue = dot(normal,lightDirection);

	reflectColor = reflectColor * 0.75 + 0.25 * dotValue;

	float diffuse = max(0.0,dotValue);
	outColor = mix(refractColor,reflectColor * LMd * (0.9 + diffuse),v_freshnel);
}