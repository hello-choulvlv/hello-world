/******************Normal Object Vertex Shader*************/
;//c70    Eye Position
;----------------------------------------
#define InputPosition v0
#define InputNormal v1
#define InputTexCoord v2

#define OutputTexCoord oT0
#define OutputVertexNormal oT1
#define OutputVertexPosition oT2
#define OutputEyeVector oT3

#define EyePosition c70
#define ZERO c72.x

#define Temp1 r7
#define Temp2 r8

vs.2.0

dcl_position0 InputPosition
dcl_texcoord0 InputTexCoord
dcl_normal0 InputNormal


;// ‰≥ˆ£∫t0 TexCoord
;//t1£∫Vertex Normal
;//t2£∫Vertex Position
;//t3£∫Eye Vector

m4x4 oPos, InputPosition, c92
mov OutputTexCoord, InputTexCoord
m3x3 OutputVertexNormal.xyz, InputNormal, c76
mov OutputVertexNormal.w, ZERO


;//º∆À„Eye Vector
sub Temp1, EyePosition, InputPosition
nrm Temp2, Temp1
mov OutputEyeVector, Temp2

;//º∆À„Position
m4x4 OutputVertexPosition, InputPosition, c76