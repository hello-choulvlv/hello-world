#version 330 core
//precision highp float;
uniform   samplerCube    u_cubeMap;
layout(location=0)out    vec4     outColor;

in     vec3     v_texCoord;

void   main()
{
	outColor=texture(u_cubeMap,v_texCoord);
}