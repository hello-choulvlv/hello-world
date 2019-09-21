#version 330 core
precision highp float;
uniform   mat4   g_modelMatrix;
uniform   mat4   g_projectMatrix;
uniform   vec2   g_waveDirection;
uniform   vec4   g_awfkParam;
uniform   float  g_time;
layout(location=0)in       vec4       a_position;
layout(location=1)in       vec2       a_fragCoord;

out      vec3      v_normal;
out      vec2      v_fragCoord;
out      vec3      v_position;

#define  g_awfkParam_a   g_awfkParam.x
#define  g_awfkParam_w   g_awfkParam.y
#define  g_awfkParam_f   g_awfkParam.z
#define  g_awfkParam_k   g_awfkParam.w

void     main()
{
//solve  dx,dy
     vec4     _position = g_modelMatrix * a_position;
     float    _param = dot(g_waveDirection,_position.xz)*g_awfkParam_w + g_time * g_awfkParam_f;
     float    _sinValue = sin(_param);
     float    _cosValue = cos(_param);

     float    dx = g_awfkParam_k*g_waveDirection.x*g_awfkParam_w*g_awfkParam_a
                   * pow((_sinValue+1.0)*0.5,g_awfkParam_k-1.0) * _cosValue;
     float    dz = g_awfkParam_k*g_waveDirection.y*g_awfkParam_w*g_awfkParam_a
                   * pow((_sinValue+1.0)*0.5,g_awfkParam_k-1.0) * _cosValue;

     v_position = vec3(
     	             _position.x,
     	             2.0 * g_awfkParam_a * pow( (_sinValue+1.0)*0.5,g_awfkParam_k  ),
     	             _position.z
     	           );
     v_normal = vec3(-dx*0.5,1.0,-dz*0.5);
     v_fragCoord = a_fragCoord;
     gl_Position = g_projectMatrix * vec4(v_position.xyz,a_position.w);
}