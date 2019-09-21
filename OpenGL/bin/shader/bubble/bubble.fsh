#version 410
//precision highp float;
uniform   float       u_radius;
uniform   sampler2D   u_baseMap;
layout(location=0)out   vec4     outColor;

void   main()
{
	  const  vec4   color1=vec4(0.4,0.6,0.8,1.0);
	  const  vec4   color2=vec4(0.47,0.86,1.0,0.0);

	  vec2   point=gl_PointCoord-vec2(0.5,0.5);
	  float  radius=dot(point,point);
	  if(radius>0.25)
	     discard;
	  outColor=mix(color1,color2,smoothstep(0.1,0.25,radius));
	  outColor = mix(outColor,vec4(0.0),smoothstep(0.0,1.0,radius/0.25)     );
}