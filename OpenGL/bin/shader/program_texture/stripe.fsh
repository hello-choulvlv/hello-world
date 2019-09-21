#version 330 core


uniform   vec3          u_lightPosition;
uniform   vec3          u_lightColor;
uniform   vec3          u_ambientColor;
uniform   vec3          u_eyePosition;
uniform   vec3          u_stripeColor;
uniform   vec3          u_intervalColor;
uniform   float         u_specularFactor;
uniform   float         u_stripeCount;
uniform   float         u_stripeWidth;
uniform   float         u_stripeBlurWidth;
 
layout(location=0)out     vec4        outColor;

in      vec2        v_texCoord;
in      vec3        v_position;
in      vec3        v_normal;

void   main()
{
     float    lengthways = fract(v_texCoord.y*u_stripeCount);

     float    bottom_distance = clamp(lengthways/u_stripeBlurWidth,0.0,1.0);
     float    top_distance = clamp((lengthways - u_stripeWidth)/u_stripeBlurWidth,0.0,1.0);

     bottom_distance = bottom_distance * (1.0 - top_distance);
     outColor=vec4(mix(u_intervalColor,u_stripeColor,smoothstep(0.0,1.0,bottom_distance) ),1.0);

     vec3     object_normal = normalize(v_normal);
     vec3     light_normal = normalize(u_lightPosition - v_position);
     float    diffuse=max(0.0, dot(object_normal,light_normal ));

     vec3     light_reflect=reflect(-light_normal,object_normal );
     vec3     eye_normal = normalize(u_eyePosition - v_position);
     float    specular=max(0.0,dot(light_reflect, eye_normal));
     specular = pow(specular,16.0);

     outColor.xyz = outColor.xyz*u_ambientColor+ outColor.xyz*u_lightColor*diffuse + u_lightColor * specular * u_specularFactor;
}