#version 410
precision   highp      float;
uniform     sampler2D  u_baseMap;
uniform     float      u_kernelCount;
uniform     vec2       u_textureStep;
layout(location=0)out     vec4     outColor;

in      vec2     v_fragCoord;

void    main()
{
	  int      step=int(u_kernelCount)>>1;
	  outColor=vec4(0.0);
	  for(int  i=-step;i<=step ; ++i)
	  {
	  	  for(int  k=-step ; k<=step ;++k )
             outColor += texture(u_baseMap,v_fragCoord + u_textureStep*vec2(float(i),float(k))  );
	  }
	  outColor/=(u_kernelCount * u_kernelCount);
}        