;//c0��		ShallowWaterColor
;//c1��		DeepWaterColor
;//c2��		Diffuse Light Color
;//c3��		Specular Light Color
;//c4��		Diffuse Light Direction��Normalized
;//c5��		Specular Light Direction��Normalized
;//c6��		Normalmap��ƽ�̴�����Width, Height��������ƽ��ͬһ��NormalMap
;//c7��		Normalmap�ֱ��ʵĵ�����1/Width, 1/Height��
;//C8��		ˮ�����ĵ��������꣬����CPУ������
;//C9��		1 / ˮ��뾶������CPУ������
;//Tex0��	BaseMap
;//Tex1��	Env CubeMap
;//Tex2��	Fresnel Map
;//Tex3��	High Frequency NormalMap��Normals are always Normalized��

#define ShallowWaterColor c0
#define DeepWaterColor c1
#define LightDiffuseColor c2
#define LightSpecularColor c3

#define LightDiffuseDir c4
#define LightSpecularDir c5

#define NormalmapWrapNum c6
#define NormalmapDimension c7

#define cWaterCentrePos c8
#define cOneByWaterRadius c9

#define ZERO c10.x
#define HALF c10.y
#define ONE c10.z
#define TWO c10.w

#define FresnelCoefScale c11.x	;//���ڽ�һ������FresnelMap�е�ֵ
#define FresnelPower c11.y		;// Fresnel (1-cos)^Power
#define SpecularCoefScale c11.z		;//�߹�����ϵ��
#define SpecularPower c11.w		;//�߹���

#define SrcAlpha c12.x				;//���Alphaֵ����alpha���

;//����
#define TexBaseMap s0
#define TexEnvMap s1
#define TexFresnelMap s2
#define TexNormalMap s3


;//��������
#define Normal r0
#define ReflectVector r1
#define DiffuseCoef r2.x
#define SpecularCoef r2.y
#define FresnelCoef r2.z
#define BaseColor r3
#define ReflectColor r4
#define Color r5
#define WaterColor r6

;//��ʱ����
#define EyeVector r7	;//EyeVector�ڼ����ReflectVector֮����Ѿ������ˣ����Ժ����Color��������
#define EdotN r8			;//clamp(E dot N)
#define rCP r9				;// ��ȷѰַ����CP

#define Temp1 r10
#define Temp2 r11



;//����Temp1�������������꣩�������Temp2�������ǿ����ģ����õ��ܶ�����ŵļĴ���������һ��Ҫ�ŵ�����ʼִ��
#define BILINEAR_TEXLD 	frc Temp2, Temp1\
						sub r1, Temp1, Temp2\
						\
						sub Temp2.z, ONE, Temp2.x\
						sub Temp2.w, ONE, Temp2.y\
						\
						add r4, r1, ONE\
						\
						mov r2.x, r4.x\
						mov r2.y, r1.y\
						mov r2.zw, ONE\
						\
						mov r3.x, r1.x\
						mov r3.y, r4.y\
						mov r3.zw, ONE\
						\
						mul r5, Temp2.z, Temp2.w\
						mul r6, Temp2.x, Temp2.w\
						mul r7, Temp2.z, Temp2.y\
						mul r8, Temp2.x, Temp2.y\
						\
						add r1, r1, HALF\
						mul r1, r1, NormalmapDimension\
						texld r1, r1, s3\
						\
						add r2, r2, HALF\
						mul r2, r2, NormalmapDimension\
						texld r2, r2, s3\
						\
						add r3, r3, HALF\
						mul r3, r3, NormalmapDimension\
						texld r3, r3, s3\
						\
						add r4, r4, HALF\
						mul r4, r4, NormalmapDimension\
						texld r4, r4, s3\
						\
						mul Temp2, r1, r5\
						mad Temp2, r2, r6, Temp2\
						mad Temp2, r3, r7, Temp2\
						mad Temp2, r4, r8, Temp2\
