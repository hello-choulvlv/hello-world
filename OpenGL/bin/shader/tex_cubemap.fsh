precision  highp   float;
uniform    samplerCube     u_cubeMap;
layout(location=0)out      vec4       outColor;

in       vec3       v_position;
void     main()
{
     outColor = texture(u_cubeMap,v_position);
}