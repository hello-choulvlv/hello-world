;//c0：		ShallowWaterColor
;//c1：		DeepWaterColor
;//c2：		Diffuse Light Color
;//c3：		Specular Light Color
;//c4：		Diffuse Light Direction：Normalized
;//c5：		Specular Light Direction：Normalized
;//c6：		Normalmap的平铺次数（Width, Height），便于平铺同一个NormalMap
;//c7：		Normalmap分辨率的倒数（1/Width, 1/Height）
;//C8：		水面中心点世界坐标，计算CP校正向量
;//C9：		1 / 水面半径，计算CP校正向量
;//Tex0：	BaseMap
;//Tex1：	Env CubeMap
;//Tex2：	Fresnel Map
;//Tex3：	High Frequency NormalMap（Normals are always Normalized）

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

#define FresnelCoefScale c11.x	;//用于进一步缩放FresnelMap中的值
#define FresnelPower c11.y		;// Fresnel (1-cos)^Power
#define SpecularCoefScale c11.z		;//高光缩放系数
#define SpecularPower c11.w		;//高光幂

#define SrcAlpha c12.x				;//输出Alpha值用于alpha混合

;//输入
#define TexBaseMap s0
#define TexEnvMap s1
#define TexFresnelMap s2
#define TexNormalMap s3


;//公用数据
#define Normal r0
#define ReflectVector r1
#define DiffuseCoef r2.x
#define SpecularCoef r2.y
#define FresnelCoef r2.z
#define BaseColor r3
#define ReflectColor r4
#define Color r5
#define WaterColor r6

;//临时数据
#define EyeVector r7	;//EyeVector在计算好ReflectVector之后就已经无用了，所以后面的Color和它公用
#define EdotN r8			;//clamp(E dot N)
#define rCP r9				;// 精确寻址向量CP

#define Temp1 r10
#define Temp2 r11



;//输入Temp1（整数纹理坐标），输出到Temp2，代码是拷贝的，会用到很多乱序号的寄存器，所以一定要放到程序开始执行
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
