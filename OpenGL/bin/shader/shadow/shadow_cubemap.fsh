#version 330 core
//precision  highp  float;
uniform    vec3      u_lightPosition;
uniform    float     u_maxDistance;
in       vec4        v_position;

void   main()
{
	gl_FragDepth = length(v_position.xyz - u_lightPosition)/u_maxDistance;
}