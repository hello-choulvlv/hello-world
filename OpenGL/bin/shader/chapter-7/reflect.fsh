precision highp  float;
uniform      samplerCube    u_baseMap;
layout(location=0)out    vec4    outColor;

in     vec3      v_normal;

void  main()
{
	    outColor=texture(u_baseMap,v_normal);
}