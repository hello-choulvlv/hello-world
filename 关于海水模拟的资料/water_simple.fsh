uniform sampler2D	u_skybox;
uniform vec3      	u_freshnelParam;
uniform vec2        u_textureStep;

varying vec3 		v_normal;
varying vec2 		v_fragCoord;

vec2    offsetFragCoord()
{
	vec2 stepPixel = u_textureStep;//1.0/textureSize(u_skybox,0);
	return stepPixel * v_normal.xy;
}

void   main()
{
	vec3 _normal = normalize(v_normal);
	float _fresnelFactor = u_freshnelParam.x + u_freshnelParam.y * pow(1.0 - dot(vec3(0.0,0.0,1.0),_normal),u_freshnelParam.z);
	gl_FragColor = texture2D(u_skybox,clamp(v_fragCoord + offsetFragCoord(),0.0,1.0));
	gl_FragColor.a = gl_FragColor.a * 0.7 + 0.3 * _fresnelFactor;
}