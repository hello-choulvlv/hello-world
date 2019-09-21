#version 330 core
//precision    highp     float;
uniform      sampler2D      u_baseMap;
uniform      float          u_exposure;
layout(location=0)out       vec4       outColor;

in      vec2     v_fragCoord;

void     main()
{
	outColor=texture(u_baseMap,v_fragCoord);
	const    float     _gamma = 2.2;

    outColor.rgb = vec3(1.0) - exp(-outColor.rgb * u_exposure );
    outColor.rgb = pow(outColor.rgb,1/_gamma);
}
