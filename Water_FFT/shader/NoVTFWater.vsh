/******************CPU-Lock FFT Ocean Water Vertex Shader*************/
;//c2		TexWidth, TexHeight
;//c70    Eye Position
;----------------------------------------
#define InputPosition v0
#define InputTexCoord v1

#define InputTangent v2
#define InputNormal v3
#define InputBiNormal v4

#define OutputTexCoord oT0
#define OutputEyeVector oT1
#define OutputTNBRow1 oT2
#define OutputTNBRow2 oT3
#define OutputTNBRow3 oT4
#define OutputTexCoordInteger oT5
#define OutputVertexNormal oT6
#define OutputVertexPosition oT7


#define EyePosition c70
#define TexDimension c2

#define Temp1 r7
#define Temp2 r8

#define Tangent r9
#define Normal r10
#define BiNormal r11


vs.2.0

dcl_position0 InputPosition
dcl_texcoord0 InputTexCoord
dcl_tangent0 InputTangent
dcl_normal0 InputNormal
dcl_binormal0 InputBiNormal


;//输出：t0 TexCoord
;//t1：Eye Vector
;//t2t3t4：TNB矩阵 Row1/2/3
;//t5：转换为整数后的TexCoord，避免在插值时丢失精度，便于PS中进行Bilinear Filter FP NormalMap
;//t6：VertexNormal，用于取消法线图
;//t7：VertexPosition，用于计算CP

m4x4 oPos, InputPosition, c92
m4x4 OutputVertexPosition, InputPosition, c76

mov OutputTexCoord, InputTexCoord
mov OutputVertexNormal, InputNormal


;//输出转换为整数的TexCoord
mul OutputTexCoordInteger, InputTexCoord, TexDimension


;//计算Eye Vector
sub Temp1, EyePosition, InputPosition
nrm Temp2, Temp1
mov OutputEyeVector, Temp2


;//TNB（转置矩阵）
;//Binormal手动计算
mov Normal, InputNormal
mov Tangent, InputTangent
;//crs Temp.xyz, Tangent, Normal	;//D3DX Use
crs Temp1.xyz, Normal, Tangent	;//NVIDIA的方法用
nrm BiNormal, Temp1



mov r0.x, Tangent.x
mov r0.y, Normal.x
mov r0.z, BiNormal.x

mov r1.x, Tangent.y
mov r1.y, Normal.y
mov r1.z, BiNormal.y

mov r2.x, Tangent.z
mov r2.y, Normal.z
mov r2.z, BiNormal.z

mov OutputTNBRow1, r0
mov OutputTNBRow2, r1
mov OutputTNBRow3, r2