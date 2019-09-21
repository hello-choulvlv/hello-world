precision highp  float;
layout(location=0)in     vec4     a_position;
layout(location=1)in     vec3     a_normal;

smooth    out      vec3      v_normal;

void   main()
{
	   mat4   mat;
	   mat[0]=vec4(-1.0,0.0,0.0,0.0);
	   mat[1]=vec4(0.0,1.0,0.0,0.0);
	   mat[2]=vec4(0.0,0.0,-1.0,0.0);
	   mat[3]=vec4(0.0,0.0,0.0,1.0);
	   vec4  _position=mat*a_position;

       v_normal=reflect(_position.xyz/_position.w,normalize(a_normal.xyz));
       v_normal=normalize(v_normal);

       gl_Position=a_position;
}