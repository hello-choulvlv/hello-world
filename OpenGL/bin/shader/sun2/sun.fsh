#version 330 core
//precision    highp    float;
uniform      sampler2D       u_baseMap;
uniform      vec4                  u_lightColor;
layout(location=0)out      vec4     outColor;
in           vec2            v_texCoord;
in           vec3            v_normal;
in           vec3            v_position;
void     main()
{
	    vec4      baseColor=texture(u_baseMap,v_texCoord);
     vec3       u_lightPosition=vec3(1.0,0.0,-4.0);
     vec3       v_lightDir=normalize(u_lightPosition-v_position);
	    float     dotL=max(0.0,dot(normalize(v_normal),normalize(v_lightDir)));
	     
      vec3     eyeDir=vec3(1.0,0.0,0.0);
      vec3     _reflect=reflect(-normalize(v_lightDir),normalize(v_normal));
      float     specular=max(0.0,dot(eyeDir,_reflect));
      specular=pow(specular,128.0);
      
       outColor=baseColor*vec4(0.01,0.01,0.01,0.01)+dotL*baseColor*u_lightColor;
       outColor.rgb+=vec3(specular,specular,specular);
}