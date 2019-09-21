#version 330 core
precision highp float;

layout(location=0)out		vec4		outColor;

uniform		sampler2D		g_BaseMap;
uniform		vec4			g_WaterParam;
uniform		vec2			g_MeshSize;

in		vec2	v_fragCoord;

void	main()
{
	float	velocity = 0.0;
	vec2	pixelSize = 1.0 / textureSize(g_BaseMap,0);
	velocity += texture(g_BaseMap,v_fragCoord + vec2(0.0,-pixelSize.y)).r;//bottom
	velocity += texture(g_BaseMap,v_fragCoord + vec2(pixelSize.x,0.0)).r;//right
	velocity += texture(g_BaseMap,v_fragCoord + vec2(0.0,pixelSize.y)).r;//top
	velocity += texture(g_BaseMap,v_fragCoord + vec2(-pixelSize.x,0.0)).r;//left

	vec2	heightField = texture(g_BaseMap,v_fragCoord).rg;

	heightField.g += velocity * 0.25 - heightField.r;
	heightField.g *= 0.99;
	heightField.r += heightField.g ;
	if(g_WaterParam.w > 0)
	{

		float	S = length(g_MeshSize * v_fragCoord - g_WaterParam.xy);
		heightField.r -= g_WaterParam.w * max(g_WaterParam.z - S,0.0);
	}
	outColor = vec4(heightField,0.0,0.0);
}