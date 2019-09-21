#version 410
layout(triangles) in ;
layout(triangle_strip,max_vertices=240) out;
uniform   mat4    u_mvpMatrix;
uniform   int     u_fur_layers=30;
uniform   float     u_fur_depth=5.0;

in    VS_GS_VERTEX
{
	vec2    v_texCoord;
	vec3    v_normal;
}vVertex[];

out   GS_FS_VERTEX
{
    vec3     v_normal;
    vec2     v_texCoord;
    flat float    furFactor;
}outVertex;

void   main()
{
	int    i,layer;
	float  deltaLayer=1.0/float(u_fur_layers);
	float  d=0.0;
	vec4   position;
	for(layer=0;layer<u_fur_layers;++layer)
	{
		for(i=0;i<gl_in.length();++i)
		{
			vec3    normal=vVertex[i].v_normal;
			outVertex.v_normal=normal;
			outVertex.v_texCoord=vVertex[i].v_texCoord;
			outVertex.furFactor=1.0-d;
			position=gl_in[i].gl_Position+vec4(normal*d*u_fur_depth,0.0);
			gl_Position=u_mvpMatrix*position;
			EmitVertex();
		}
		d+=deltaLayer;
		EndPrimitive();
	}
}