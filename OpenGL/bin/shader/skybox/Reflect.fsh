#version 330 core
//precision highp float;
uniform    samplerCube     u_cubeMap;
uniform    vec3            u_eyePosition;
uniform    vec3            u_lightColor;
uniform    vec3            u_lightPosition;
uniform    vec3            u_ambientColor;
uniform    float           u_specularFactor;
layout(location=0)out   vec4    outColor;

in   vec3   v_position;
in   vec3   v_normal;

void   main()
{
	vec3    _normal = normalize(v_normal);
	vec3    _eye_normal = normalize( v_position-u_eyePosition);
	vec3    _reflect=reflect(_eye_normal,_normal);
	outColor=texture(u_cubeMap,_reflect);

    vec3     light_normal = normalize(u_lightPosition - v_position);
    float    diffuse = max(0.0,dot(light_normal,_normal));

    vec3     light_reflect = reflect(-light_normal,_normal);
    float    specular=max(0.0,dot(light_reflect,_eye_normal));
    specular = pow(specular,16.0)*u_specularFactor;

    outColor.xyz = outColor.xyz*u_ambientColor + outColor.xyz*u_lightColor*diffuse + u_lightColor*specular;
}