uniform samplerCube u_skybox;
uniform vec3      	u_freshnelParam;
uniform vec3    	u_eyePosition;
uniform vec4    	u_waterColor;
uniform float       u_waterRatio;

varying vec3 		v_normal;
varying vec2 		v_fragCoord;
varying vec3        v_position;
void main()
{
	vec3 _normal = normalize(v_normal);
	vec3 _eye_direct = normalize(v_position - u_eyePosition);

	vec3 _reflectVec = reflect(_eye_direct,_normal);
	vec3 _refractVec = refract(_eye_direct,_normal,u_waterRatio);

	vec4 _reflectColor = textureCube(u_skybox,_reflectVec);
	vec4 _refractColor = textureCube(u_skybox,_refractVec);

	float _fresnelFactor = u_freshnelParam.x + u_freshnelParam.y * pow(1.0 - dot(-_eye_direct,_normal),u_freshnelParam.z);

	vec4  _color = _reflectColor * _fresnelFactor + _refractColor *(1.0 - _fresnelFactor);
	gl_FragColor.rgb = _color.rgb;
	gl_FragColor.a = 0.5 + 0.5* _fresnelFactor;
}