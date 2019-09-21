#version 330 core
precision highp float;

layout(location=0)out 		vec4	   outColor;

uniform		mat4	  g_CropMatrix[4];
uniform		vec4	  g_NormalSegments;
uniform		vec3	  g_LightDirection;
uniform		vec3	  g_EyePosition;
uniform		sampler2DArrayShadow	g_SegmentShadow;
uniform		sampler2D               g_BaseMap;

in    vec4		v_position;
in    vec3		v_normal;
in    vec2		v_fragCoord;

int		getLayerColor(out vec4	layerColor)
{
	int layer=3;
	layerColor = vec4(0.5,0.5,0.5,1.0);
	if(gl_FragCoord.z < g_NormalSegments.x)
	{
		layer = 0;
		layerColor = vec4(1.0,0.5,0.5,1.0);
	}
	else if(gl_FragCoord.z < g_NormalSegments.y)
	{
		layer = 1;
		layerColor = vec4(0.5,1.0,0.5,1.0);
	}
	else if(gl_FragCoord.z < g_NormalSegments.z)
	{
		layer = 2;
		layerColor = vec4(1.0,0.5,1.0,1.0);
	}
	return layer;
}

void	main()
{
	vec4	layerColor;
	int 	layer = getLayerColor(layerColor);

	vec3	normalVec = normalize(v_normal);
	float   diffuse = max(0.0,dot(-g_LightDirection,normalVec));

	vec3	reflectVec = reflect(-g_LightDirection,normalVec);
	vec3	eyeVec = normalize(g_EyePosition - v_position.xyz);
	float   specular = pow(max(0.0,dot(-eyeVec,reflectVec)),32.0);

	outColor = texture(g_BaseMap,v_fragCoord);

	vec3	extraColor = outColor.rgb * diffuse + vec3(1.0) * specular;
	vec4	shadowCoord = g_CropMatrix[layer] * v_position;
	float   shadowFactor = texture(g_SegmentShadow,vec4(shadowCoord.xy,float(layer),shadowCoord.z));

	outColor.rgb = outColor.rgb * vec3(0.2) + extraColor * shadowFactor;
	outColor *= layerColor;
}
