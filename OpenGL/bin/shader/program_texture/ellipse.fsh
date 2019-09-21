#version 330 core
uniform      vec2      u_abValue;
uniform      vec4      u_shapeColor;
uniform      vec4      u_maskColor;
layout(location=0)out     vec4       outColor;
varying      vec2      v_fragCoord;

void    main()
{
	vec2      xyCoord = v_fragCoord * u_abValue *2.0;
	vec2      changeCoord = xyCoord - u_abValue;
	float     ellipse_result = dot(changeCoord*changeCoord,1.0/(u_abValue*u_abValue));
	if(ellipse_result>1.0)  discard;

    float    lengthxy = length(changeCoord);
	vec2     cos_sinxy = changeCoord/lengthxy;
	vec2     margin_point = cos_sinxy*u_abValue;
    float    real_length = u_abValue.x*u_abValue.y/sqrt(dot(cos_sinxy*cos_sinxy,u_abValue.yx*u_abValue.yx));

    float    factor = smoothstep(real_length*0.9,real_length,lengthxy);

    outColor = mix(u_shapeColor,u_maskColor,factor);
}