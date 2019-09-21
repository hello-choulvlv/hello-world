#version 330 core
precision highp float;
uniform   samplerCube   u_skyTexCube;
uniform   vec3          u_freshnelParam;
uniform   float  		u_refractRatio;
uniform   vec3    		u_eyePosition;
uniform   vec4    		u_waterColor;

layout(location=0)out       vec4       outColor;

in       vec3     v_position;
in       vec3     v_normal;
in       vec2     v_fragCoord;

void   main()
{
	// vec3     vNormal = normalize(v_normal);
	// vec3     vEyeVecDir = normalize( u_eyePosition - v_position);
	// float   freshnel = u_freshnelParam.x + (u_freshnelParam.y * pow(1.0 -dot(vNormal,vEyeVecDir),u_freshnelParam.z ));

	// vec3    refractVec = refract( -vEyeVecDir,vNormal,u_refractRatio);
	// vec3    reflectVec = reflect(-vEyeVecDir,vNormal);

	// vec4    refractColor = texture(u_skyTexCube,refractVec);
	// vec4    reflectColor = texture(u_skyTexCube,reflectVec);

	// outColor = mix(
	// 	             u_waterColor,
	// 	             mix(refractColor,reflectColor,freshnel),
	// 	             0.775
	// 	           );
	// outColor.a =0.7 + 0.3*freshnel;
	vec3 _normal = normalize(v_normal);
	vec3 _eye_direct = normalize(v_position - u_eyePosition);

	vec3 _reflectVec = reflect(_eye_direct,_normal);
	vec3 _refractVec = refract(_eye_direct,_normal,u_refractRatio);

	vec4 _reflectColor = texture(u_skyTexCube,_reflectVec);
	vec4 _refractColor = texture(u_skyTexCube,_refractVec);

	float _fresnelFactor = u_freshnelParam.x + u_freshnelParam.y * pow(1.0 - dot(-_eye_direct,_normal),u_freshnelParam.z);

	vec4  _color = _reflectColor * _fresnelFactor + _refractColor *(1.0 - _fresnelFactor);
	outColor.rgb = _color.rgb;
	outColor.a = 0.5 + 0.5* _fresnelFactor;
}