#version 330 core
precision     highp           float;
uniform      sampler2D        u_baseMap;
uniform      sampler2D        u_normalMap;
uniform      vec3             u_lightVector;
layout(location=0)out   vec4  outColor;

in     vec2      v_texCoord;
in     mat3      v_tbnMatrix;

void   main()
{
	 vec3      normalVector=v_tbnMatrix *( (texture(u_normalMap,v_texCoord).xyz-0.5)*2.0);
     normalVector = normalize(normalVector);

	 float     factor=max(0.0,dot(u_lightVector,normalVector));
	 vec3      ambientColor=vec3(0.2);
	 vec3      u_lightColor=vec3(10.0,10.0,12.0);

	 outColor=texture(u_baseMap,v_texCoord);
	 outColor.xyz=outColor.xyz*(u_lightColor*factor+ambientColor);

     outColor.xyz = vec3(1.0) - exp(-outColor.xyz * 0.5 );
}
