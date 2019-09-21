#version 330 core
//precision  highp  float;
uniform    vec3     u_lightColor;

layout(location=0)out       vec4       outColor;
layout(location=1)out       vec4       hdrColor;

void  main()
{
	outColor = vec4(u_lightColor,1.0);
	hdrColor=vec4(0.0);
	if(dot(u_lightColor,vec3(0.2126, 0.7152, 0.0722))>1.0)
	     hdrColor = outColor;
}