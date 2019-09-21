precision  highp   float;
layout(location=0)in   vec4    a_position;

out     vec3      v_cubeMaptexCoord;

void     main()
{
	  gl_Position=a_position;

	  v_cubeMaptexCoord=normalize(a_position.xyz);
}