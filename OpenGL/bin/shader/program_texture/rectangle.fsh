#version 330 core

uniform      vec2    u_abValue;
uniform      vec4      u_shapeColor;
uniform      vec4      u_maskColor;
layout(location=0)out     vec4       outColor;
varying      vec2      v_fragCoord;

void  main()
{
    vec2      _pointCoord = abs((v_fragCoord - vec2(0.5))*u_abValue)*2.0;
   float     _crital_length = length(u_abValue);
	vec2      _critalCosSinValue = u_abValue.xy/_crital_length;
	float     _real_length = length(_pointCoord);
    vec2     _realCosSinValue = _pointCoord.xy/_real_length;

    vec2      _referVec2;
    if(_realCosSinValue.y<_critalCosSinValue.y)
         _referVec2=vec2(u_abValue.x,u_abValue.x*_realCosSinValue.x/_realCosSinValue.y);
    else
         _referVec2=vec2(u_abValue.y*_realCosSinValue.y/_realCosSinValue.x,u_abValue.y);
    float   _refer_length=length(_referVec2);
    float   _refer_fix=_refer_length*_refer_length*_refer_length;
    float   _real_fix=_real_length*_real_length*_real_length;

    float   _mix_factor = smoothstep(_refer_fix*0.8,_refer_fix,_real_fix);
    outColor = mix(u_shapeColor,u_maskColor,_mix_factor);
}