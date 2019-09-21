#version 330 core
precision highp float;

uniform		samplerCube		u_skyboxTexCube;
uniform		vec3			u_eyePosition;
uniform		vec3			u_freshnelParam;
uniform		float			u_refractRatio;
uniform		vec4			u_waterColor;

layout(location =0)out		vec4		outColor;

in		vec3		v_position;
in		vec3		v_normal;

void  main()
{
		vec3	_normal = v_normal;//normalize(v_normal);
		vec3	_eye_normal = normalize(u_eyePosition - v_position);	
		vec3	_reflectVec = reflect(- _eye_normal,_normal);
		vec3	_refractVec = refract(-_eye_normal,_normal, u_refractRatio);

		float	_dotValue = dot(_eye_normal,_normal);
		float	_freshnel = u_freshnelParam.x + u_freshnelParam.y * pow(1.0 - _dotValue,u_freshnelParam.z );

		// outColor = mix(		texture(u_skyboxTexCube,_reflectVec),
		// 					texture(u_skyboxTexCube,_refractVec),
		// 					_freshnel
		// 				);
		outColor =mix(		u_waterColor,
							texture(u_skyboxTexCube,_reflectVec),
							0.3
					 );

		outColor = mix(		texture(u_skyboxTexCube,_reflectVec)*u_waterColor,
							texture(u_skyboxTexCube,_refractVec),
							0.3
						);
//		outColor = texture(u_skyboxTexCube,_reflectVec);
}