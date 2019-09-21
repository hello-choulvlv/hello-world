#version 410
precision     highp      float;
uniform     sampler2D       u_globleMap;
uniform     sampler2D       u_globlePosition;
uniform     sampler2D       u_globleNormal;
uniform     sampler2D       u_ssaoMap;
uniform     vec3            u_lightPosition;
uniform     vec3            u_lightColor;
uniform     vec3            u_eyePosition;
uniform     vec3            u_ambientColor;
uniform     float           u_specularCoeff;

layout(location=0)out        vec4        outColor;

in       vec2        v_fragCoord;

void     main()
{
	vec3      position = texture(u_globlePosition,v_fragCoord).xyz;
	vec3      normal =normalize( texture(u_globleNormal , v_fragCoord)  ).xyz;
	outColor  = texture(u_globleMap,v_fragCoord);

    vec3      light_direction = normalize(u_lightPosition - position);
    vec3      eye_direction = normalize(u_eyePosition - position);
    vec3      halfWay = normalize(light_direction + eye_direction);

    float     diffuse = max(0.0,dot(normal, light_direction));

    float     specular =pow( max(0.0,dot(halfWay, normal)), 32.0)*u_specularCoeff;
    outColor.xyz  = outColor.xyz * u_ambientColor *texture(u_ssaoMap,v_fragCoord).r + outColor.xyz * u_lightColor * diffuse + u_lightColor * specular;
}