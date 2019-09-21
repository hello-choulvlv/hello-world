#version 330 core
layout(location=0)out vec4 outColor;
uniform sampler2DArrayShadow g_CascadeShadowMap;
uniform vec4	g_Color;
uniform vec3    g_LightPosition;
uniform vec3    g_LightColor;
uniform mat4    g_TextureMatrix[6];

in	vec3 v_position;
in 	vec3 v_normal;

void main()
{
	outColor = g_Color;
	vec3 lightDistanceVec = g_LightPosition - v_position;
	vec3 lightDirection = normalize(lightDistanceVec);
	vec3 normal = normalize(v_normal);

	float selectAxis[6] = float[](-lightDistanceVec.x,lightDistanceVec.x,
		                        -lightDistanceVec.y,lightDistanceVec.y,
		                        -lightDistanceVec.z,lightDistanceVec.z);
	int layer=0;
	for(int k = 0; k < 6 ; ++k)
	{
		if(selectAxis[k] > selectAxis[layer])
			layer = k;
	}
	vec4  texturePosition = g_TextureMatrix[layer] * vec4(v_position,1.0);
	texturePosition.xyz /= texturePosition.w;
	texturePosition.w = texturePosition.z;
	texturePosition.z = float(layer);


	float diffuse = max(0.0,dot(normal,lightDirection)) * texture(g_CascadeShadowMap,texturePosition);

	outColor.rgb *= g_LightColor * 0.3 + 0.7 *diffuse;
}