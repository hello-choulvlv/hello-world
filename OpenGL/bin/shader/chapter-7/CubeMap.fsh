precision highp  float;
uniform    samplerCube            u_cubeMap;
layout(location=0)out    vec4     outColor;
smooth  in        vec3     v_cubeMaptexCoord;

void  main()
{
	     outColor=texture(u_cubeMap,normalize(v_cubeMaptexCoord.stp));
}
