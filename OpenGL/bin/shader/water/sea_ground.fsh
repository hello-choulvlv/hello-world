#version 330 core
//precision  highp   float;
uniform    sampler2D        u_baseMap;
uniform    sampler2D        u_lightMap;
//the equation of sean plane ,it means a*x+b*y+c*z+d=0
//and a,b,c,d refered u_planeEquation.xyzw
//and vector3 u_planeEquation.xyz is normalized
uniform    vec4    u_planeEquation;
uniform    float   u_time;
layout(location=0)out       vec4        outColor;


in      vec3       v_position;
in      vec3       v_normal;
in      vec2       v_fragCoord;

vec2     __gradient_wave(float x,float y)
{
    const    float    VAVE_ATITUDE=0.08;
    float    d=sqrt(x*x+y*y)/27.0;
    vec2     dxdy=vec2(0.0);
    const    int    _iterator_times=8;
    int      _now_times=0;
    float    _param=0.5;
    const   float   _speed =1.0;
    const   float   _WAVESIZE=0.0067;
    do
    {
    	float   sinValue=sin(u_time*_speed +1.0/_param*_WAVESIZE*x*y);
    	float   cosValue=cos(u_time*_speed +1.0/_param*_WAVESIZE*x*y);

         dxdy.x += d*sinValue*y*_WAVESIZE - _param*cosValue*x/d;

         dxdy.y += d*sinValue*x*_WAVESIZE - _param*cosValue*y/d;

         _param/=2.0;
         ++_now_times;
    }while(_now_times<_iterator_times);
    return  VAVE_ATITUDE*dxdy*0.787;
}

vec2   __refract_light( )
{
    vec2    _gradVec2 = __gradient_wave(v_position.x,v_position.z);
//note derivative is  (dx,1.0,dz)
    vec3    _gradVec3 = vec3(_gradVec2.x,1.0,_gradVec2.y);
    vec3    _refractVec3 = refract(vec3(0.0,1.0,0.0),_gradVec3,1.33);
    
    return normalize(_refractVec3).xz;
}

void   main()
{
//	vec2   _gradVec2 = __gradient_wave(v_position.x,v_position.z);
//	vec3   _normal_gradVec3=normalize(vec3(_gradVec2.x,1.0,_gradVec2.y));
//	float  _crossAngle=dot(v_normal,_normal_gradVec3);
//	vec4   _planeEquation=vec4(0.0,1.0,0.0,-8.0);
//	vec3   _intercept_point = v_position+_normal_gradVec3*(_planeEquation.w - dot(_planeEquation.xyz,v_position))/_crossAngle;
//    vec3   _inVector3=vec3(1.0,1.0,0.0);
    vec2   _refeclctVector2 = __refract_light();
	outColor =texture(u_baseMap,v_fragCoord) ;
	outColor.rgb+= texture(u_lightMap,_refeclctVector2.xy).rgb;
}