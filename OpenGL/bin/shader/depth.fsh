uniform     sampler2D     u_baseMap;
uniform     vec4          u_renderColor;
layout(location=0)out     vec4    outColor;
in       vec2     v_texCoord;

void    main()
{
        outColor=texture(u_baseMap,v_texCoord)*u_renderColor;
}