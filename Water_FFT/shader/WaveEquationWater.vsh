/******************VTF FFT+Wave Water Vertex Shader*************/
;//c1			�߶�����ֵScaler
;//c2			TexWidth, TexHeight
;//c3			HeightMap Wrap Coef������ϵ��������ͬһ�Ÿ߶�ͼƽ�̴������ˮ
;//tex0		HeightMap��real, imag, real_Sign, imag_Sign
;//tex1		NormalMap: D3DXVECTOR3, 1��ע�ⷶΧ0��1����Ҫ_bx2��չ��
;//tex2		TangentMap: ͬ��
;//----------------------------------------

;//����
#define InputPosition v0
#define InputNormal v1
#define InputTexCoord v2

#define TexHeightMap s0
#define TexNormalMap s1
#define TexTangentMap s2

;//���
#define OutputPosition o0
#define OutputTexCoord o1
#define OutputTexCoordInteger o2

#define OutputEyeVector o3
#define OutputVertexNormal o4
#define OutputVertexPosition o5

#define OutputTNBRow1 o6
#define OutputTNBRow2 o7
#define OutputTNBRow3 o8

#define OutputProjPosition o9

;//����
#define ConstScaler c1
#define ConstTexDimension c2
#define ConstWrapCoef c3

#define ZERO c0.x
#define HALF c0.y
#define ONE c0.z
#define TWO c0.w


;//��ʱʹ��
#define HeightValue r0
#define HeightValue_re r0.x
#define HeightValue_im r0.y
#define HeightSign_re r0.z
#define HeightSign_im r0.w

#define VertexPosition r1
#define VertexNormal r2
#define VertexTangent r3
#define VertexBiNormal r4
#define EyeVector r5

#define TexCoord r6

#define Temp r10

vs.3.0

dcl_position0 InputPosition
dcl_normal0 InputNormal
dcl_texcoord0 InputTexCoord

dcl_position OutputPosition
dcl_texcoord0 OutputTexCoord
dcl_texcoord1 OutputTexCoordInteger	;//ת��Ϊ������TexCoord���Ա�Normalmap Bilinear Filtering

dcl_normal1 OutputEyeVector	;//Eye Vector
dcl_normal0 OutputVertexNormal	;//VertexNormal����������Normalmap
dcl_normal2 OutputVertexPosition	;// VertexPosition In WorldSpace�����ڼ������Ѱַ����CP
dcl_normal3 OutputProjPosition		;// VertexPosition In ProjSpace�����ڼ���Fog

dcl_tangent0 OutputTNBRow1	;//TNBRow1
dcl_tangent1 OutputTNBRow2	;//TNBRow3
dcl_tangent2 OutputTNBRow3	;//TNBRow3

dcl_2d TexHeightMap
dcl_2d TexNormalMap
dcl_2d TexTangentMap

def c0, 0, 0.5, 1, 2

;//Texture Wrap Coef
mul TexCoord, InputTexCoord, ConstWrapCoef

;//Get HeightMap and Mul Scaler
texldl HeightValue, TexCoord, TexHeightMap

;//Restore HeightValue from HeightSign
sub HeightSign_re, HeightSign_re, ONE
sub HeightSign_im, HeightSign_im, ONE
mul HeightValue_re, HeightValue_re, HeightSign_re
mul HeightValue_im, HeightValue_im, HeightSign_im

;//Add HeightValue to Vertex XY Coord
mov VertexPosition.z, InputPosition.z
mov VertexPosition.w, ONE

;//Gestener Wave, Notion that HeightValue.x->Cos->Y
mad VertexPosition.x, HeightValue_im, ConstScaler, v0.x
mad VertexPosition.y, HeightValue_re, ConstScaler, v0.y


;//Get Normal
texldl Temp, TexCoord, TexNormalMap
sub Temp, Temp, HALF	;//��0��1�ָ�Ϊ-1��1���ȼ�0.5�ٳ�2��
mul Temp, Temp, TWO
nrm VertexNormal, Temp

	;// �������ˮ�£�ˮ�淨��ȡ��
mov Temp.x, c70.y
if_le Temp.x, c0.x
	mov VertexNormal, -VertexNormal
endif

;//Get Tangent
texldl Temp, TexCoord, TexTangentMap
sub Temp, Temp, HALF	;//��0��1�ָ�Ϊ-1��1���ȼ�0.5�ٳ�2��
mul Temp, Temp, TWO
nrm VertexTangent, Temp

;//Calculate Binormal
crs Temp.xyz, VertexNormal, VertexTangent	// NVDIA�ķ�����ҪN X T�����ģ�
nrm VertexBiNormal, Temp


;//Get Eye Vector
sub Temp, c70, VertexPosition
nrm EyeVector, Temp


;//Output ProjCoord & WorldCoord & TexCoord & Normal & Eye
m4x4 OutputPosition, VertexPosition, c92
m4x4 OutputProjPosition, VertexPosition, c92
m4x4 OutputVertexPosition, VertexPosition, c76

;//Output TexCoord
mov OutputTexCoord, InputTexCoord
mul OutputTexCoordInteger, InputTexCoord, ConstTexDimension	;//���ת��Ϊ������TexCoord
;//Output Vector
mov OutputEyeVector, EyeVector
mov OutputVertexNormal, VertexNormal

;//Output Matrix TNB��֮������TNB����TBN����FFT����ͼ����ʱ�Ŀռ����йأ�
mov OutputTNBRow1.x, VertexTangent.x
mov OutputTNBRow1.y, VertexNormal.x
mov OutputTNBRow1.z, VertexBiNormal.x

mov OutputTNBRow2.x, VertexTangent.y
mov OutputTNBRow2.y, VertexNormal.y
mov OutputTNBRow2.z, VertexBiNormal.y

mov OutputTNBRow3.x, VertexTangent.z
mov OutputTNBRow3.y, VertexNormal.z
mov OutputTNBRow3.z, VertexBiNormal.z
