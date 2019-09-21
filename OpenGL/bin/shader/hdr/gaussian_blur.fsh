#version 330 core
//precision    highp   float;
uniform     sampler2D     u_baseMap;
uniform     vec2          u_hvstep;
layout(location=0)out     vec4      outColor;

in      vec2     v_fragCoord;

void    main()
{
	vec2     _pixel_offset = 1.0/textureSize(u_baseMap,0);
    float    _gaussian_factor[5];
    _gaussian_factor[0]=0.227027;
    _gaussian_factor[1]=0.1945946,
    _gaussian_factor[2]=0.1216216;
    _gaussian_factor[3]=0.054054;
    _gaussian_factor[4]=0.016216;

    outColor = texture(u_baseMap,v_fragCoord);
    outColor.rgb*= _gaussian_factor[0];

    outColor.rgb+= texture(u_baseMap,v_fragCoord + u_hvstep * _pixel_offset).rgb*_gaussian_factor[1];
    outColor.rgb+= texture(u_baseMap,v_fragCoord - u_hvstep * _pixel_offset).rgb*_gaussian_factor[1];
    outColor.rgb+= texture(u_baseMap,v_fragCoord + u_hvstep * _pixel_offset*2).rgb*_gaussian_factor[2];
    outColor.rgb+= texture(u_baseMap,v_fragCoord - u_hvstep * _pixel_offset*2).rgb*_gaussian_factor[2];
    outColor.rgb+= texture(u_baseMap,v_fragCoord + u_hvstep * _pixel_offset*3).rgb*_gaussian_factor[3];
    outColor.rgb+= texture(u_baseMap,v_fragCoord - u_hvstep * _pixel_offset*3).rgb*_gaussian_factor[3];
    outColor.rgb+= texture(u_baseMap,v_fragCoord + u_hvstep * _pixel_offset*4).rgb*_gaussian_factor[4];
    outColor.rgb+= texture(u_baseMap,v_fragCoord - u_hvstep * _pixel_offset*4).rgb*_gaussian_factor[4];
}