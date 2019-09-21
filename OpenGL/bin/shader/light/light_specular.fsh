#version 330 core
//precision highp float;
uniform   sampler2D    u_baseMap;
uniform   vec3         u_lightColor;
uniform   vec3         u_lightVector;
uniform   vec3         u_eyePosition;
uniform   vec3         u_ambientColor;
uniform   float        u_lightPower;
layout(location=0)out        vec4       outColor;

in      vec2      v_texCoord;
in      vec3      v_normal;
in      vec3      v_position;

void   main()
{
	vec4      baseColor = texture(u_baseMap,v_texCoord);
	vec3      light_dir = normalize( u_lightVector - v_position);
	vec3      light_normal=normalize(v_normal);
	float     diffuse=max(0.0,dot(light_dir , light_normal ));

	vec3      reflect_dir = normalize(reflect(-light_dir, light_normal));
	vec3      eye_dir=normalize(u_eyePosition - v_position );
	float     specular=max(0.0, dot(eye_dir , reflect_dir));


	outColor.xyz=baseColor.xyz*u_ambientColor + baseColor.xyz*u_lightColor*diffuse +u_lightColor*pow(specular,32.0)*u_lightPower;
	outColor.a=baseColor.a;
}