#version 410
//precision highp  float;
layout(location=0)in      vec4      a_position;
layout(location=1)in      vec2      a_texCoord;
layout(location=2)in      vec3      a_normal;

out    VS_GS_VERTEX
{
     vec2    v_texCoord;
     vec3    v_normal;
}vVertex;
void     main()
{
   vVertex.v_texCoord=a_texCoord;
   vVertex.v_normal=a_normal;
   gl_Position=a_position;
}