#version 430 core

#define WORK_GROUP_SIZE 128
#define NOW_GROUP_SIZE  126
#define Coefficient     0.99
#define Array_Index(x,y)  ((x+1) * g_MeshSize + (y+1))
uniform vec4  g_WaveParam;
uniform vec2  g_WaveResolutionHalf;
uniform int   g_MeshSize;

layout(std430,binding=0)buffer SrcBuffer{
	float	height[];
};

layout(std430,binding=1)buffer Velocity{
	float   velocity[];
};

layout(std430,binding=2)buffer DstBuffer{
	float   field[];
};

layout(local_size_x = 1,local_size_y = 1,local_size_z = 1)in;

void    main()
{
	int   x = int(gl_GlobalInvocationID.x);
	int   y = int(gl_GlobalInvocationID.y);
	int   index =Array_Index(x,y);

	float  centerHeight = height[index];

	float  arroundHeight = height[Array_Index(x -1,y -1)] + height[Array_Index(x,y+1)] +
						  height[Array_Index(x+1,y+1)] + height[Array_Index(x,y-1)];
	float  velocityValue = velocity[index];
	float  value = arroundHeight * 0.25 - centerHeight + velocityValue;
	velocityValue = value * Coefficient;
	velocity[index] = velocityValue;

	float nowHeight = centerHeight + velocityValue;
	if(g_WaveParam.w > 0.0 )
	{
		float d = distance(	vec2( float(x)+1.0 ,float(y) +1.0 ), g_WaveParam.xy);
		d = d / g_WaveParam.z * 3.0 + 1.0;
		nowHeight -= exp(- d*d ) * g_WaveParam.w;
	}
	field[index] = nowHeight;
}