#version 330 core
precision   highp     float;
//uniform     sampler2D       u_baseMap;
uniform     vec4            u_renderColor;
uniform     float           u_critical;
uniform     vec2            u_inc;
layout(location=0)out       vec4       outColor;

in        vec2        v_texCoord;
in        vec2        v_position;
const        float          square_critical=4.0;
const        int            max_iteration=40;
void   main()
{
       int      	i=0;
       vec2     	z_iterator=v_position;
       vec2         _origin_position;

       while(i<max_iteration && dot(z_iterator,z_iterator)<square_critical)
       {
       	       vec2     z,hz,hhz;
       	       z.x=z_iterator.x*z_iterator.x - z_iterator.y*z_iterator.y;
       	       z.y=2.0*z_iterator.x*z_iterator.y;

       	       hz.x=z_iterator.x*z.x- z_iterator.y*z.y;
               hz.y=z_iterator.x*z.y+z_iterator.y*z.x;
               
               hhz.x=z.x*z.x-z.y*z.y;
               hhz.y=2.0*z.x*z.y;

       	       z_iterator=hhz+vec2(-z.y,z.x)+u_inc;
       	       ++i;
       }
       if(i==max_iteration)
            outColor=u_renderColor;
       else
       discard;//outColor=texture(u_baseMap,v_texCoord);//*u_renderColor;
}