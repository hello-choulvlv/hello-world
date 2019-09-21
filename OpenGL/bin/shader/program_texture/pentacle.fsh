#version 330 core

uniform     vec3       u_halfSpace[5];
uniform     vec3       u_pentacleColor;
uniform     vec3       u_sphereColor;
uniform     vec3       u_lightPosition;
uniform     vec3       u_lightColor;
uniform     vec3       u_ambientColor;
uniform     vec3       u_eyePosition;
uniform     float      u_specularFactor;
uniform     float      u_stripeWidth;
uniform     float      u_borderWidth;

layout(location=0)out      vec4      outColor;

in      vec3        v_position;
in      vec3        v_normal;

void    main()
{
	  vec3      normal_position = normalize(v_normal);
      vec4      distance;
      distance.x = dot(normal_position,u_halfSpace[0])+0.2;
      distance.y = dot(normal_position,u_halfSpace[1])+0.2;
      distance.z = dot(normal_position,u_halfSpace[2])+0.2;
      distance.a = dot(normal_position,u_halfSpace[3])+0.2;

      float     other_distance;
      other_distance = dot(normal_position,u_halfSpace[4])+0.2;

      distance = smoothstep(-u_borderWidth,u_borderWidth,distance);
      other_distance = smoothstep(-u_borderWidth,u_borderWidth,other_distance);

      float    circle =-3.0 + dot(distance,vec4(1.0));
      circle += other_distance;

      circle = smoothstep(0.0,1.0,circle);

      outColor = vec4(mix(u_sphereColor,u_pentacleColor,circle ),1.0);

      vec3     light_normal = normalize( u_lightPosition- v_position );
      float    diffuse = max(0.0, dot(light_normal,normal_position ));

      vec3     light_reflect = reflect(-light_normal,normal_position );
      vec3     eye_normal = normalize(u_eyePosition - v_position );
      float    specular = max(0.0, dot(eye_normal,light_reflect));
      specular = pow(specular,16)*u_specularFactor;

      outColor.xyz = outColor.xyz * u_ambientColor + outColor.xyz * u_lightColor * diffuse + u_lightColor*specular;
}