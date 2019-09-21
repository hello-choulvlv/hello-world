precision   highp   float;
uniform     sampler2D        u_baseMap;
layout(location=0)out        vec4      outColor;

in        vec2      v_texCoord;
in        vec3      v_position;
in        vec3      v_normal;

void    main()
{
 	   vec3          _eyePosition=vec3(-0.1,0.0,1.0);
 	   vec3          _reflect=reflect(normalize(v_position-_eyePosition),normalize(v_normal));
 	   vec2          _texCoord=normalize(_reflect.xy);
 	   outColor=texture(u_baseMap,vec2(-v_texCoord.x,v_texCoord.y));
}