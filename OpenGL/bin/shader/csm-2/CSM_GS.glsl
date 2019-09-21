#version 330 core
precision highp float;

layout(triangles)in;
layout(triangle_strip,max_vertices=12)out;

uniform		mat4	g_CropMatrix[4];

void 	main()
{
	const	int		frustumSplitCount = 4;
	for(int idx=0; idx < frustumSplitCount ; ++ idx)
	{
		for(int i=0; i< gl_in.length(); ++i)
		{
			gl_Position = g_CropMatrix[idx] * gl_in[i].gl_Position;
			gl_Layer = idx;

			EmitVertex();
		}
		EndPrimitive();
	}
}