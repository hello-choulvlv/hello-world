#version 410
//precision highp float;
uniform    float    u_radius;
uniform    float    u_pointSize;
uniform    float    u_timeInterval;
layout(location=0)in    vec4    a_position;
layout(location=1)in    vec2    a_velocity;
out      vec2      v_position;
out      vec2      v_velocity;
void   main()
{
	 vec2    position=vec2(a_position);
	 vec2    nPosition=position+a_velocity*u_timeInterval;
	 v_velocity=a_velocity;
	 v_position=nPosition;
     if(nPosition.x-u_radius<-1.0)
     {
          v_velocity=reflect(v_velocity,vec2(1.0,0.0));
          v_position=vec2(-1.0+u_radius,nPosition.y);
     }
     else if(nPosition.x+u_radius > 1.0)
     { 
          v_velocity=reflect(v_velocity,vec2(-1.0,0.0));
          v_position=vec2(1.0-u_radius,nPosition.y);
     }
     else if(nPosition.y+u_radius > 1.0)
     {
          v_velocity=reflect(v_velocity,vec2(0.0,-1.0));
          v_position=vec2(nPosition.x,1.0-u_radius);
     }
     else if(nPosition.y-u_radius<-1.0)
     {
          v_velocity=reflect(v_velocity,vec2(0.0,1.0));
          v_position=vec2(nPosition.x,-1.0+u_radius);
     }
     gl_Position=vec4(v_position,0.0,1.0);
     gl_PointSize=u_pointSize;
}