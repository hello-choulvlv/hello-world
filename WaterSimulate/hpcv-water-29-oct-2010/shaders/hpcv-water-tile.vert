//
// Vertex shader 

varying vec3  Normal;
varying vec3  EyeDir;


varying vec2 texNormal0Coord;
varying vec2 texColorCoord;
varying vec2 texFlowCoord;

uniform float osg_FrameTime;
uniform mat4 osg_ViewMatrixInverse;
varying float myTime;
void main(void)
{
	
    gl_Position    = ftransform();
    Normal         = normalize(gl_NormalMatrix * gl_Normal);
  
    vec4 pos       = gl_ModelViewMatrix * gl_Vertex;
    EyeDir         = vec3(osg_ViewMatrixInverse*vec4(pos.xyz,0));;

    texNormal0Coord   = gl_MultiTexCoord1.xy;
    texColorCoord = gl_MultiTexCoord3.xy;
    texFlowCoord = gl_MultiTexCoord2.xy;

    myTime = 0.01 * osg_FrameTime;
}