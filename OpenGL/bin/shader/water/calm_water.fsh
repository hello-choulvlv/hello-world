#version 330 core
precision highp float;
uniform   sampler2D    g_baseMap;
uniform   vec3         g_lightColor;
uniform   vec3         g_lightDirection;
uniform   vec3         g_ambientColor;
uniform   vec3         g_eyePosition;

layout(location=0)out      vec4      outColor;

in       vec2      v_fragCoord;
in       vec3      v_normal;
in       vec3      v_position;
void    main()
{
	outColor = texture(g_baseMap,v_fragCoord);

    vec3    _normal = normalize(v_normal);
    float   _diffuse = max(0.0,dot(g_lightDirection,_normal));

    vec3    _reflect = reflect(-g_lightDirection,_normal);
    float   _specular=max(0.0,dot(normalize(g_eyePosition - v_position),_reflect));

    outColor.rgb = outColor.rgb * g_ambientColor + outColor.rgb*g_lightColor*_diffuse+ g_lightColor*pow(_specular,128.0f)*0.017;
}