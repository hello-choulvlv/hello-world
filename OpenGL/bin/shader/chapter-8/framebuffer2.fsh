precision  highp   float;
uniform    sampler2D     u_baseMap;
layout(location=0)out    vec4     outColor0;
layout(location=1)out    vec4     outColor1;
in     vec2       v_texCoord;
in     vec3       v_normal;
void   main()
{
       vec3     light_dir=normalize(vec3(1.0,0.0,0.3));
       float    _dotL=max(0.0,dot(light_dir,v_normal));
       vec3      light_color=vec3(0.12,0.12,0.12)+vec3(_dotL);
	   outColor0=texture(u_baseMap,v_texCoord)*vec4(light_color,1.0);
	   outColor1=vec4(0.6,1.0,0.6,1.0);
}