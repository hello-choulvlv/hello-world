#version 330 core
//precision     highp     float;
uniform       sampler2D      u_baseMap;
uniform       sampler2D      u_hdrMap;
uniform       float          u_exposure;

layout(location=0)out       vec4       outColor;

in      vec2      v_fragCoord;

void    main()
{
	outColor = texture(u_baseMap,v_fragCoord);
    vec4      hdrColor = texture(u_hdrMap,v_fragCoord);

     outColor.rgb += hdrColor.rgb;

     outColor.rgb = vec3(1.0) - exp(-outColor.rgb * u_exposure);
     outColor.rgb = pow(outColor.rgb,vec3(1.0/2.2));
}