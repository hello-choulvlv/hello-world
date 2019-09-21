#version 440 core
precision highp float;

layout(triangles)in;
layout(triangle_strip,max_vertices=12)out;

uniform 	vec4	u_ViewPort[4];
uniform		vec2	u_ShadowMapSize;

vec2	convertWindowCoord(vec4	clipCoord,int viewPortIndex)
{
	vec2	ndcPosition = clipCoord.xy/clipCoord.w;
	vec2    normalPosition = ndcPosition * 0.5 + 0.5;
	return u_ViewPort[viewPortIndex].xy + normalPosition * u_ViewPort[viewPortIndex].zw;
}

bool	checkIntersect(vec4 src1,vec4 src2)
{
	bool flagX = src1.x > src2.z || src2.x > src1.z;
	bool flagY = src1.y > src2.w || src2.y > src1.w;
	return !flagX && !flagY;
}

void    sendPrimitive(int viewPortIndex)
{
	for(int i=0;i<gl_in.length();++i)
	{
		gl_Position = gl_in[i].gl_Position;
		gl_ViewportIndex = viewPortIndex;
		gl_Layer = viewPortIndex;
		EmitVertex();
	}
	EndPrimitive();
}

void    main()
{
	vec4	windowBounding = vec4(0.0,0.0,u_ShadowMapSize);
	//Split frustum in segment
	const   int    FrustumSegmentCount = 4;
	for(int segment = 0;segment < FrustumSegmentCount; ++segment)
	{
		vec2	startPosition = convertWindowCoord(gl_in[0].gl_Position,segment);
		vec4	aabb = vec4(startPosition,startPosition);
		for(int j= 1; j < gl_in.length(); ++j)
		{
			vec2	nowPosition = convertWindowCoord(gl_in[j].gl_Position,segment);
			vec4	otherAABB;
			// otherAABB.x = min(aabb.x,nowPosition.x);
			// otherAABB.y = min(aabb.y,nowPosition.y);
			// otherAABB.z = max(aabb.x,nowPosition.x);
			// otherAABB.w = max(aabb.y,nowPosition.y);
			otherAABB.xy = min(aabb.xy,nowPosition);
			otherAABB.zw = max(aabb.xy,nowPosition);
			aabb = otherAABB;
		}
		if( checkIntersect(aabb,windowBounding))
		{
			sendPrimitive(segment);
		}
	}
}