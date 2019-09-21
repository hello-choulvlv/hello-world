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
	vec3     vNormal = normalize(v_normal);
	vec3     vEyeVecDir = normalize( u_eyePosition - v_position);
	float   freshnel = u_freshnelParam.x + (u_freshnelParam.y * pow(1.0 -dot(vNormal,vEyeVecDir),u_freshnelParam.z ));

	vec3    refractVec = refract( -vEyeVecDir,vNormal,u_refractRatio);
	vec3    reflectVec = reflect(-vEyeVecDir,vNormal);

	vec4    refractColor = texture(u_skyTexCube,refractVec);
	vec4    reflectColor = texture(u_skyTexCube,reflectVec);

	outColor = mix(
		             u_waterColor,
		             mix(refractColor,reflectColor,freshnel),
		             0.775
		           );
	outColor.a *=0.7 + 0.3*freshnel;
}