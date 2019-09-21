#ifdef GL_ES
precision mediump float;
#endif

// Inputs
attribute vec4 a_position;
attribute vec2 a_texCoord;

// Varyings
//varying vec2 v_texCoord;
#ifdef GL_ES
varying mediump vec2 v_texCoord;
#else
varying vec2 v_texCoord;
#endif

uniform	float radius ;
uniform	float angle ;


void main()
{
    gl_Position = CC_MVPMatrix * a_position;
    v_texCoord = a_texCoord;
}