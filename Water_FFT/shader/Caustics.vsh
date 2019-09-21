/******************Caustics Vertex Shader*************/
;//c10    Caustics Space World * View * Proj Matrix Transpose
;//c70    Eye Position
;----------------------------------------
#define InputPosition v0
#define InputNormal v1
#define InputTexCoord v2

#define OutputTexCoord oT0
#define OutputVertexNormal oT1
#define OutputVertexPosition oT2
#define OutputEyeVector oT3
#define OutputCaustics oT4
#define OutputProjPosition oT5

#define EyePosition c70
#define CausticsMatrix c10

#define ZERO c72.x

#define Temp1 r7
#define Temp2 r8

vs.2.0

dcl_position0 InputPosition
dcl_texcoord0 InputTexCoord
dcl_normal0 InputNormal


;//输出：t0 TexCoord
;//t1：Vertex Normal
;//t2：Vertex Position
;//t3：Eye Vector
;//t4：Caustics Space Position
;//t5：Proj Space Position

m4x4 oPos, InputPosition, c92

mov OutputTexCoord, InputTexCoord
m3x3 OutputVertexNormal.xyz, InputNormal, c76
mov OutputVertexNormal.w, ZERO

m4x4 OutputCaustics, InputPosition, CausticsMatrix

;//计算Eye Vector
sub Temp1, EyePosition, InputPosition
nrm Temp2, Temp1
mov OutputEyeVector, Temp2

;//计算World Space Position
m4x4 OutputVertexPosition, InputPosition, c76

;//计算Proj Space Depth用
m4x4 OutputProjPosition, InputPosition, c92