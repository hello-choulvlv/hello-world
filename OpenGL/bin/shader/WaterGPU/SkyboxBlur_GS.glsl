#version 330 core
layout(triangles)in;
layout(triangle_strip,max_vertices=3)out;

in	vec3	v_fragCoord[];
out vec3    v_fragCoord3;

void	main()
{
	for(int i=0;i<gl_in.length(); ++i)
	{
		vec4	position = gl_in[i].gl_Position;
		float 	layer[6] = float[](position.x,-position.x,position.y,-position.y,position.z,-position.z);
		int 	slice=0;
		float 	maxCoord=position.x;
		for(int j=1;j<6 ;++j)
		{
			if(layer[j] > maxCoord)
			{
				maxCoord = layer[j];
				slice=j;
			}
		}
		gl_Position = vec4(position.xy,0.0,1.0);
		v_fragCoord3 = v_fragCoord[i];
		gl_Layer = slice;
		EmitVertex();
	}
	EndPrimitive();
}