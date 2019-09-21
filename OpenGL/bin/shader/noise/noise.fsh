#version 330 core
//precision highp float;
uniform     sampler3D     u_baseMap;
uniform     float         u_time;
layout(location=0)out     vec4    outColor;
in       vec2     v_texCoord;

void    main()
{
//     float  b=texture(u_baseMap,vec3(v_texCoord,abs(sin(u_time/57.29578)))).b;
       outColor=texture(u_baseMap,vec3(v_texCoord,abs(sin(u_time/57.29578))));
}