precision highp float;
uniform  sampler2D    u_colorMaskMap;
uniform  float        u_percent;
layout(location=0)out    vec4     outColor;
in    vec2   v_texCoord;

void   main()
{
	float  factor=v_texCoord.y-(32.0/569.0);
	if(factor>u_percent)
	   discard;

    outColor=texture(u_colorMaskMap,v_texCoord);
    factor=clamp(0.0,1.0,factor);
//混合
    outColor.a=outColor.a*(1.0- factor/u_percent );//(1.0-smoothstep(0.0,u_percent,v_texCoord.y));
} 