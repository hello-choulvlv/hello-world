#version 430 core

uniform   float    g_MeshUnitSize;

#define GROUP_SIZE  128
#define NOW_GROUP_SIZE 127
#define ARRAY_INDEX(x,y) ((x+1)*GROUP_SIZE+(y+1))
#define ARRAY_INDEX2(x,y) (x+1)*GROUP_SIZE +(y+1)
layout(std430,binding=0)buffer srcHeight{
	float heightField[];
};

layout(std430,binding=1)buffer dstNormal{
	float normal[];
};

// layout(std430,binding=2)buffer dstHeight{
// 	float outHeight[];
// };

layout(local_size_x = 1,local_size_y = 1,local_size_z = 1)in;

void    main()
{
	int  x = int(gl_GlobalInvocationID.x);
	int  y = int(gl_GlobalInvocationID.y);
	//up index
	int index = ARRAY_INDEX(x,y);
	//left index 
	int index0 = ARRAY_INDEX(x,y-1);
	//bottom index
	int index1 = ARRAY_INDEX(x-1,y-1);

	int index_m3 = index *3;
	float   h = heightField[index];

	normal[index_m3] = h - heightField[index0];
	normal[index_m3+1] = g_MeshUnitSize;
	normal[index_m3+2] = h - heightField[index1];

	//outHeight[index] = h;
}