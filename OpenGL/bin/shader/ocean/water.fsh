#version 330 core
precision highp float;

layout(location=0)out      vec4      outColor;

uniform    float      u_freshnelBias;
uniform    float      u_freshnelScale;
uniform    float      u_freshnelPower;
uniform    float      u_etaRatio;
uniform    vec3       u_lightPosition;
uniform    vec3       u_lightColor;
uniform    vec3       u_eyePosition;
uniform    sampler2D     u_normalMap;
uniform    samplerCube   u_normalCube;
uniform    samplerCube   u_normalCube2;
uniform    samplerCube   u_bottomCube;
uniform    samplerCube   u_envCube;

in       vec3       v_position;
in       vec3       v_normal;
in       vec2       v_fragCoord;

void     main()
{
	
}