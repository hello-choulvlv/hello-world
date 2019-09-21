#version 330 core
layout(location=0)in	vec4	a_position;
layout(location=1)in	vec3	a_normal;
layout(location=2)in	vec2	a_fragCoord;

uniform		mat4	g_ModelMatrix;
uniform		mat4	g_ViewProjMatrix;
uniform     mat3    g_NormalMatrix;
uniform		vec3	g_FreshnelParam;
uniform		vec3	g_LightPosition;
uniform		vec3	g_EyePosition;
uniform		float   g_Ratio;

out		vec2	v_fragCoord;
//out		vec3	v_normal;
out     vec3	v_reflectVec;
out     vec3    v_refractVec;
out		vec3	v_lightDirection;
out		float   v_freshnel;

void	main()
{
	vec4	position = g_ModelMatrix * a_position;
	gl_Position = g_ViewProjMatrix * position;

	v_fragCoord = a_fragCoord;
	v_lightDirection = normalize(g_LightPosition - position.xyz);

	vec3	eyeDirection = position.xyz - g_EyePosition;
	vec3	normal = normalize(g_NormalMatrix * a_normal);
	//v_normal = normal;

	v_reflectVec = reflect(eyeDirection,normal);

	eyeDirection = normalize(eyeDirection);
	v_refractVec = refract(eyeDirection,normal,g_Ratio);

	v_freshnel = g_FreshnelParam.x + g_FreshnelParam.y * pow(1.0 + dot(normal,eyeDirection),g_FreshnelParam.z);
}