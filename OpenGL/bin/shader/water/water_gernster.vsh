#version 330 core
//precision highp  float;
uniform     mat4         u_mvpMatrix;
uniform     mat4         u_modelViewMatrix;
uniform     vec4         u_DParams;
uniform     vec2         u_QParams;
uniform     vec2         u_AParams;
uniform     vec2         u_omegaWave;
uniform     vec2         u_deltaTime;
uniform     vec2         u_waveFi;
layout(location=0)in      vec4      a_position;
layout(location=1)in      vec2      a_texCoord;

out     vec2      v_texCoord;

vec3     _gerstner_wave(vec3    _position)
{
     vec3    _position_disturb=vec3(0.0);

     vec2      _common_value=u_waveFi *u_deltaTime;

     vec2     _params=u_omegaWave*vec2(dot(_position.xz,u_DParams.xy),dot(_position.xz,u_DParams.zw)) + _common_value;

     vec2     _cos_value = cos(_params);
     vec2     _sin_value = sin(_params);
     _position_disturb.x = u_QParams.x *u_AParams.x * u_DParams.x*  _cos_value.x;
     _position_disturb.z = u_QParams.x *u_AParams.x * u_DParams.y * _cos_value.x;
     _position_disturb.y = u_AParams.x *_sin_value.x;

     _position_disturb.x += u_QParams.y *u_AParams.y * u_DParams.z * _cos_value.y;
     _position_disturb.y += u_QParams.y *u_AParams.y * u_DParams.w * _cos_value.y;
     _position_disturb.z += u_AParams.y *_sin_value.y;

     return  _position_disturb;
}
void   main()
{
     vec4      _position=u_modelViewMatrix * a_position;

     _position.xyz += _gerstner_wave(_position.xyz);
     gl_Position = u_mvpMatrix * _position;
     v_texCoord = a_texCoord;
}