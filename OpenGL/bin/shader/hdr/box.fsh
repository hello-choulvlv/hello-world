#version 330 core
//precision   highp   float;
uniform       sampler2D      u_baseMap;
uniform       vec3           u_lightPosition;
uniform       vec3           u_eyePosition;
uniform       vec3           u_lightColor;
uniform       vec3           u_ambientColor;
uniform       float          u_specularFactor;

layout(location=0)out      vec4      outColor;
layout(location=1)out      vec4      hdrColor;

in      vec3      v_position;
in      vec3      v_normal;
in      vec2      v_fragCoord;

void     main()
{
	vec3     _new_normal = normalize(v_normal);
	vec3     _light_direction = normalize(u_lightPosition - v_position);
    float    _diffuse = max(0.0,dot(_new_normal,_light_direction));

    vec3     _eye_direction = normalize(u_eyePosition - v_position);
    vec3     _half_vector = normalize( _eye_direction + _light_direction );
    float    _specular = max(0.0,dot(_half_vector,_new_normal));
    _specular = pow(_specular,16.0);

    outColor = texture(u_baseMap,v_fragCoord);
    outColor.rgb = outColor.rgb *u_ambientColor + outColor.rgb * u_lightColor * _diffuse +u_lightColor * _specular *u_specularFactor;
    hdrColor=vec4(0.0);
	if(dot(outColor.rgb,vec3(0.2126, 0.7152, 0.0722))>1.0)
	     hdrColor = outColor;
}