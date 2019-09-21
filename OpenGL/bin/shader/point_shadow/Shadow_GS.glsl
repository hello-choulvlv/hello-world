#version 330 core
layout(triangles)in;
layout(triangle_strip,max_vertices=18)out;

uniform mat4 g_ViewProjMatrix[6];

void main()
{
	for(int layer = 0; layer < 6; ++ layer)
	{
		gl_Layer = layer;
		for(int k = 0; k < 3; ++k)
		{
			gl_Position = g_ViewProjMatrix[layer] * gl_in[k].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}