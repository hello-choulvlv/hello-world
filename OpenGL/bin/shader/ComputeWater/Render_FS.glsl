#version 330 core
layout(location=0)out		vec4		outColor;

uniform		samplerCube		g_TexCubMap;
uniform		vec3			g_CameraPosition;
uniform     vec3			g_FreshnelParam;
uniform		vec4			g_WaterColor;

in		vec3	v_position;
in		vec3	v_normal;

void	main()
{
	vec3	vnormal = normalize(v_normal);//vec3(0.0,1.0,0.0);//
	vec3    cameraDirection = normalize(g_CameraPosition - v_position);
	float   freshnel = g_FreshnelParam.x + g_FreshnelParam.y * pow(1.0 - dot(vnormal,cameraDirection),g_FreshnelParam.z);

	vec3	reflectVec = reflect(-cameraDirection,vnormal);
	vec3	refractVec = refract(-cameraDirection,vnormal,1.0/1.33);

	vec4	reflectColor = texture(g_TexCubMap,reflectVec);
	vec4	refractColor = texture(g_TexCubMap,refractVec);

	outColor = mix( g_WaterColor,mix(refractColor,reflectColor,freshnel),0.63);
	outColor.a *= 0.7 +0.3 * freshnel;
}