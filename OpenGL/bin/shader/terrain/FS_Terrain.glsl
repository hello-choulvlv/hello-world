#version 330 core
layout(location =0)out       vec4      outColor;
uniform    vec3    g_lightDirection;
uniform    vec3    g_eyePosition;
uniform    vec4    g_color;
uniform    vec4    g_lightColor;

in     vec3     v_position;
in     vec3     v_normal;

void     main()
{
	vec3    normal = normalize(v_normal);
	vec3    reflectVec = normalize(g_eyePosition - v_position);
	float   factor =  dot(reflectVec,normal);
	//float   lightFactor = max(0.0,dot(reflectVec,normal));
	outColor = g_color ;//+ pow(lightFactor,16.0) * g_lightColor.rgb;
	outColor.a = 0.5 + 0.5 *factor;
}
