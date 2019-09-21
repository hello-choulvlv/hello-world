#version 330 core
layout(location=0)out	vec4	outColor;
uniform sampler2D g_OcclusionTexture;
uniform sampler2D g_ColorTexture;
uniform sampler2D g_PositionTexture;
uniform sampler2D g_NormalTexture;
uniform vec3      g_LightPosition;
uniform vec3      g_LightColor;

in	vec2	v_fragCoord;

void	main()
{
	vec3 position = texture(g_PositionTexture,v_fragCoord).xyz;
	vec3 normal   = texture(g_NormalTexture,v_fragCoord).xyz;
	vec3 lightVec = normalize(g_LightPosition-position);

	float diffuse = max(0.0,dot(normal,lightVec));
	outColor = texture(g_ColorTexture,v_fragCoord);
	float efficient = mix(texture(g_OcclusionTexture,v_fragCoord).r , diffuse ,0.5);

	outColor.rgb *= efficient * g_LightColor  ;
}