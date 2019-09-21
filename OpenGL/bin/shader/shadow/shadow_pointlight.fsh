#version 330 core
//precision  highp  float;
uniform    sampler2D      u_baseMap;
uniform    samplerCube    u_depthMap;
uniform    vec3           u_lightColor;
uniform    vec3           u_lightPosition;
uniform    vec3           u_eyePosition;
uniform    vec3           u_ambientColor;
uniform    float          u_specularCoeff;
uniform    float          u_maxDistance;
//uniform    vec2           u_nearFar;
layout(location=0)out       vec4        outColor;

in      vec3      v_position;
in      vec3      v_normal;
in      vec2      v_fragCoord;
//in      vec4      v_lightSpacePosition;

float   lightShadowfactor(vec3  s_position)
{
//	vec3   _lightSpacePosition =v_lightSpacePosition.xyz/v_lightSpacePosition.w * 0.5 +0.5;
   float  _shadow_factor=texture(u_depthMap,s_position).r;
   return   float(length(s_position)<=_shadow_factor);
//    vec2   _texSize = 1.0/textureSize(u_depthMap,0);
//     for(int x=-1;x<=1;++x)
//     {
//     	for(int y=-1;y<=-1;++y)
//     	{
//     		float    _depth=texture(u_depthMap, _lightSpacePosition.xy+vec2(float(x),float(y))*_texSize).r;
// //            _depth =2.0*u_nearFar.x*u_nearFar.y/(_depth*(u_nearFar.y-u_nearFar.x)-(u_nearFar.y-u_nearFar.x));
//     	    _shadow_factor+= float(_lightSpacePosition.z>_depth);
//     	 }
//     }
//    return  1.0-_shadow_factor*0.1111111111;
}                                              
void    main()
{
	vec3     _normal = normalize(v_normal);
	outColor = texture(u_baseMap,v_fragCoord);
    vec3      _sposition = u_lightPosition - v_position;
    vec3      _lightDirection = normalize(_sposition);

	float     _diffuse = max(0.0,dot(_lightDirection,_normal));

	vec3      _reflect = reflect(-_lightDirection,_normal);
	vec3      _eyePosition = normalize(u_eyePosition - v_position);
	float     _specular = max(0.0,dot(_reflect,_eyePosition));

    float     _shadow_factor = lightShadowfactor( -_sposition);
    vec3      _diffuse_specular_color = outColor.rgb *u_lightColor * _diffuse  + u_lightColor*pow(_specular,32.0)*u_specularCoeff;
	outColor.rgb = outColor.rgb * u_ambientColor + _diffuse_specular_color * _shadow_factor;
}