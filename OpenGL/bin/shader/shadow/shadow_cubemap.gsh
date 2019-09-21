#version 330 core
//precision  highp  float;
layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;
uniform    mat4    u_viewProjMatrix[6];
out        vec4    v_position;

void    main()
{
	for(int  face=0;face<6;++face)
	{
		gl_Layer = face;
		for(int  _vertex=0;_vertex<3;++_vertex)
		{
			v_position = gl_in[_vertex].gl_Position;
			gl_Position = u_viewProjMatrix[face] * v_position;
			EmitVertex();
		}
		EndPrimitive();
	}
}