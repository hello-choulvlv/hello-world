#version 330 core
//precision  highp  float;
uniform    sampler2D     u_baseMap;
uniform    vec3          u_lightPosition;
uniform    vec3          u_lightColor;
uniform    vec3          u_lightDirection;
uniform    vec3          u_ambientColor;
uniform    vec3          u_eyePosition;
uniform    float         u_cosTheta;
uniform    float         u_specularFactor;

layout(location=0)out     vec4      outColor;

in       vec3      v_position;
in       vec3      v_normal;
in       vec2      v_texCoord;

void   main()
{
	vec3    normal_light=normalize( v_normal );
    vec3    light_object_dir = u_lightPosition - v_position;
    vec3    light_object_normal = normalize( light_object_dir );

    outColor=texture(u_baseMap,v_texCoord);
    float   cos_theta=dot(light_object_normal,u_lightDirection );
    if(cos_theta>u_cosTheta)
    {
    	 float    diffuse=max(0.0,dot(light_object_normal, v_normal));

    	 vec3     eye_object_dir=u_eyePosition - v_position;
    	 vec3     eye_object_normal=normalize( eye_object_dir);
    	 vec3     light_object_reflect=reflect(-light_object_normal,v_normal );

         float    specular= pow( max(0.0,dot(light_object_reflect,eye_object_normal )), 32 )*u_specularFactor;
         outColor.xyz=outColor.xyz*u_ambientColor+ (outColor.xyz* u_lightColor*diffuse + u_lightColor*specular)*smoothstep(u_cosTheta,1.0,cos_theta);
    }
    else
         outColor.xyz*=u_ambientColor;
}