#version 410
//precision highp  float;
uniform    sampler2D     u_baseMap;
uniform    vec4          u_furColor=vec4(0.8,0.8,0.9,1.0);

layout(location=0)out    vec4    outColor;

in    GS_FS_VERTEX
{
    vec3     v_normal;
    vec2     v_texCoord;
    flat float    furFactor;
}iVertex;

void   main()
{
	vec4    baseColor=texture(u_baseMap,iVertex.v_texCoord);
	outColor=u_furColor*vec4(1.0,1.0,1.0,baseColor.r*iVertex.furFactor);
}