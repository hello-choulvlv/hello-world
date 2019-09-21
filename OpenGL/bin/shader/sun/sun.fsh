#version 330 core
//precision    highp    float;
uniform      sampler2D       u_baseMap;
uniform      vec4            u_lightColor;
layout(location=0)out      vec4     outColor;
in           vec2            v_texCoord;
in           vec3            v_normal;

void     main()
{
	    vec4      baseColor=texture(u_baseMap,v_texCoord);
	    vec3      light_dir=vec3(1.0,0.0,0.3);
	    float     dotL=max(0.0,dot(normalize(v_normal),normalize(light_dir)));
        
        outColor=baseColor*vec4(0.10,0.10,0.10,1.0)+dotL*baseColor*u_lightColor;
        outColor.a;
}