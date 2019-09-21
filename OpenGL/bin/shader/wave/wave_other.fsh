#version 330 core
precision highp float;
uniform   samplerCube   u_skyTexCube;
uniform   sampler2D		u_skyboxMap;
uniform   vec3          u_freshnelParam;
uniform   float  		u_refractRatio;
uniform   vec3    		u_eyePosition;
uniform   vec4    		u_waterColor;

layout(location=0)out       vec4       outColor;

in       vec3     v_position;
in       vec3     v_normal;
in       vec2     v_fragCoord;

vec2    offsetFragCoord()
{
	vec2 stepPixel = 1.0f/textureSize(u_skyboxMap,0);
	return stepPixel * v_normal.xy;
}

void   main()
{
	vec3 _normal = normalize(v_normal);
	float _fresnelFactor = u_freshnelParam.x + u_freshnelParam.y * pow(1.0 - dot(vec3(0.0,0.0,1.0),_normal),u_freshnelParam.z);

	vec4  _reflectColor = texture(u_skyboxMap,clamp(v_fragCoord + offsetFragCoord(),0.0,1.0));
	outColor = _reflectColor;
	outColor.a = outColor.a * 0.7 + 0.3 *_fresnelFactor;
}