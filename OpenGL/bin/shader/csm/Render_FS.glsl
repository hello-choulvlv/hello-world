#version 440 core
layout(location=0)out		vec4		outColor;
precision highp float;
uniform		sampler2DArrayShadow	u_shadowMapArray;
uniform		sampler2D               u_baseMap;
uniform		mat4					u_lightVPSBMatrix[4];
uniform		vec4					u_normalSegments;
uniform		vec3					u_lightDirection;
uniform		vec3					u_eyePosition;

in  		vec3		v_position;
in   		vec3		v_normal;
in 			vec2		v_fragCoord;

vec4	computeLayerColor(out float layer)
{
	// float layer0 = step(gl_FragCoord.z,u_normalSegments.x);

	// float layert0 = step(gl_FragCoord.z,u_normalSegments.y);
	// float layer1 = layert0 * (1.0 - layer0);

	// float layert1 = step(gl_FragCoord.z,u_normalSegments.z);
	// float layer2 =  layert1 * (1.0 - layert0);

	// float layer3 = step(gl_FragCoord.z,u_normalSegments.w) *(1.0 - layert1);

	// layer = layer0 + layer1 + layer2 + layer3;
	// return layer0 * vec4(1.0, 0.5, 0.5, 1.0) + layer1 * vec4(0.5, 1.0, 0.5, 1.0) + 
	//        layer2 * vec4(0.5, 0.5, 1.0, 1.0) + layer3 * vec4(1.0, 0.5, 1.0, 1.0);
	 vec4 layerColor = vec4(0.5, 0.5, 0.5, 1.0);
	layer = 3.0;
	if(gl_FragCoord.z <= u_normalSegments.x)
	{
		layer = 0.0;
		layerColor = vec4(1.0, 0.5, 0.5, 1.0);
	}
	else if(gl_FragCoord.z <= u_normalSegments.y)
	{
		layer = 1.0;
		layerColor = vec4(0.5, 1.0, 0.5, 1.0);
	}
	else if(gl_FragCoord.z <= u_normalSegments.z )
	{
		layer = 2.0;
		layerColor = vec4(0.5, 0.5, 1.0, 1.0);
	}
	return layerColor;
}

void    main()
{
	float   layer=0.0;
	vec4	layerColor = computeLayerColor(layer);
	vec4    shadowCoordNoChange = u_lightVPSBMatrix[int(layer)] * vec4(v_position,1.0);	
	vec4	shadowCoord;

	shadowCoord.xyw = shadowCoordNoChange.xyz;
	shadowCoord.z = layer;
	float   shadowFactor = texture(u_shadowMapArray,shadowCoord);
	//calculate light ambient specular color
	vec3	_normal = normalize(v_normal);
	vec3	light_reflect = reflect(u_lightDirection,_normal);
	float   diffuseFactor = max(0.0,dot(-u_lightDirection,_normal));

	vec3	_eyeVec = normalize(u_eyePosition - v_position);
	vec3    _eye_reflect = reflect( - _eyeVec,_normal);
	float   specularFartor = pow(max(0.0,dot(_eye_reflect,light_reflect)),64.0);

	outColor =  texture(u_baseMap,v_fragCoord);
	vec3    shadowColor =  outColor.rgb * diffuseFactor + vec3(1.0) * specularFartor;

	shadowColor *= shadowFactor;
	outColor.rgb = outColor.rgb * vec3(0.2,0.2,0.2) + shadowColor;
	outColor *= layerColor;
}	