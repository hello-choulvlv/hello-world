#version 330 core
uniform     sampler2D         u_baseMap;
uniform     sampler2DShadow   u_shadowMap;
uniform     vec3              u_lightVector;
layout(location=0)out     vec4    outColor;

in       vec4     v_shadowCoord;
in       vec3     v_normal;
in       vec2     v_texCoord;
void    main()
{
	    float    diffuse=max(0.0,dot(u_lightVector,v_normal));
	    vec3     lightColor=vec3(1.0);
	    vec3     ambientColor=vec3(0.2);
	    float    shadow_factor=textureProj(u_shadowMap,v_shadowCoord);
	    vec3     mixColor=clamp(shadow_factor*lightColor*diffuse+ambientColor,vec3(0.0),vec3(1.0));
	    outColor=texture(u_baseMap,v_texCoord)*vec4(mixColor,1.0);
}