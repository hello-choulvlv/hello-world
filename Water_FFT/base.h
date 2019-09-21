#pragma once

#define EXPConstant 2.71828183f		// 常量e
#define GravityConstant 9.8f		// 重力常量g

// FP16 Data Format
typedef unsigned short float16;

extern char convstr[20];

//////////////////////////////////////////////////////////////////////////字符串函数
void inttostr(unsigned long n);
void floattostr(int bitnum,double data);
unsigned long strtoint(char *p);
double strtofloat(char *phead);
int getintbits(unsigned long aa);

void getstrconsist(unsigned int *hznum,unsigned int *charnum,char *p);
unsigned int getcharnum(char *p);
void wordtostr(char *word,char *str);
void delstr(char *p,int from,int num);
void insstr(char *dest,int from,char *source);
char *copystr(char *source,int from,int num);

// 从一个字符串得到一个浮点向量（可手动再转成int之类），字符串格式为“Data1,Data2,Data3”，一般用于转换INI中的配置
// iFloatDataNum表示只读取字符串中的前多少个浮点数，多的丢弃
// 返回字符串中总共有多少个浮点数
UINT GetFloatFromString(char * pString, float *pRetrieveData, UINT iFloatDataNum = 0);


//////////////////////////////////////////////////////////////////////////数学函数
float absf(float fNum);
double absf(double fNum);
unsigned long cf(int n);

// 检查是否2的幂，如果是就返回幂（大于等于1），如果不是就返回0
// 传入0和1，都会返回0
UINT CheckPowerOf2(UINT iValue);
// 位反序，由于给定的都是32位数，iBitNum指定按总多少位反序，高位保持不变。比如说11110000_11000110，指定8位反序就变成11110000_01100011
// 如果BitNum = 0，就会返回原值
UINT ReverseBitOrder(UINT iValue, UINT iBitNum);

float clampf(float fValue, float fStandardmin = 0.0f, float fStandardmax = 0.0f);
int roundf(float fValue, float fStandard );
int CalcFactorial(int iData);
int CalcDoubleFactorial(int iData);
float randomf(float fRandomValue, UINT iBitNum = 4);

void randomize(void);
int random(int number);

double gaussrandom();	// 高斯分布的随机数

// 计算3x3矩阵的Det（忽略第4个元素）
float GetDeterminant3X3(D3DXMATRIX Mat);




/////////////////////////////////////////////////////////////////////颜色转换
	//建立查表纹理时转换数据用，前两个是转换向量到Dword颜色，后两个是转换float颜色到Dword颜色
DWORD VectoRGBSigned(float x, float y, float z, float w);
DWORD VectoRGBUnsigned(float x, float y, float z, float w);
DWORD ColortoRGBSigned(float x, float y, float z, float w);
DWORD ColortoRGBUnsigned(float x, float y, float z, float w);
	//FP16 and FP32 Convert
float FP16Tofloat(float16 fValue);
float16 floatToFP16(float fValue);

	// 将DWORD颜色转换为D3DXCOLOR(Vector4)
#define DWCOLORTOVECTOR(Color)	D3DXCOLOR(	(float)((Color>>16)&0xff) / 255.0f,		(float)((Color>>8)&0xff) / 255.0f,	(float)(Color&0xff) / 255.0f,		(float)((Color>>24)&0xff) / 255.0f	)





//////////////////////////////////////////////////////////////////////////矩阵、向量、坐标和空间转换函数
	// 根据观察方向和指定的头顶向量生成正确的头顶向量和右向量，其实可以直接传Look和Head到MatrixLookAtLH函数中，它也会自动做相应的步骤，这里只是为了得到完整的三个轴，用于视角控制
HRESULT GenerateViewSpace(D3DXVECTOR3 *pVecLookAt_Z, D3DXVECTOR3 *pVecHead_Y, D3DXVECTOR3 *pVecRight_X);
	// 欧拉旋转角和矩阵
D3DXMATRIX GetXZXEulerMatrix(LPD3DXMATRIX pMat, D3DXVECTOR3 VecEulerAngle);
D3DXMATRIX GetZYZEulerMatrix(LPD3DXMATRIX pMat, D3DXVECTOR3 VecEulerAngle);
D3DXMATRIX XZXEulerRotation(LPD3DXMATRIX pMat, D3DXVECTOR3 VecAxis, float fAngle, LPD3DXVECTOR3 pVecEulerAngle = NULL);
D3DXMATRIX ZYZEulerRotation(LPD3DXMATRIX pMat, D3DXVECTOR3 VecAxis, float fAngle, LPD3DXVECTOR3 pVecEulerAngle = NULL);
D3DXMATRIX ZYZEulerRotation(LPD3DXMATRIX pMat, D3DXQUATERNION Quater, LPD3DXVECTOR3 pVecEulerAngle = NULL);
	// 球面坐标和笛卡儿坐标转换
D3DXVECTOR2 CartesianToSpherical(D3DXVECTOR3 VecCarte, LPD3DXVECTOR2 pVecSphere);
D3DXVECTOR3 SphericalToCartesian(D3DXVECTOR2 VecSphere, LPD3DXVECTOR3 pVecCarte);





//////////////////////////////////////////////////////////////////////////求交函数
enum MYRAYTRACING_INTERSECT
{
	INTERSECT_OK,				// 相交，相当于S_OK，只要下面任意一种状态出现就不会返回相交，也只有它出现，函数才会返回TRUE
	INTERSECT_NO,				// 普通的不相交，相当于FALSE=0
	INTERSECT_OPPOSITEDIR,		// 射线和面背离，不相交，属于特殊的不相交情况
	INTERSECT_PARALLEL,			// 射线和面接近平行，不相交，属于比较特殊的不相交情况
	INTERSECT_SAMEFACE,			// 射线起点就在该面上，共面，这是很特殊的不相交情况
	INTERSECT_SAMEPOINT,		// 射线起点就是面的其中一个顶点，很特殊的不相交情况
	INTERSECT_INVALIDARGUMENTS,	// 无效的输入参数
	INTERSECT_FATALERROR,		// 内部错误
};

	// 得到两条直线P1P2和V1V2的交点，如果无交点就返回false
bool GetIntersect2D(D3DXVECTOR3 *P, D3DXVECTOR3 P1, D3DXVECTOR3 P2, D3DXVECTOR3 V1, D3DXVECTOR3 V2);
	// 判断点P是否在三角形内V1V2V3，2D面上测试没有问题，3D未测试
bool IsPointInTriangle(D3DXVECTOR3 P, D3DXVECTOR3 PtV1, D3DXVECTOR3 PtV2, D3DXVECTOR3 PtV3);
	// 射线跟踪求交，得到和平面的交点
BOOL GetIntersectPlane(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXPLANE Plane, LPD3DXVECTOR3 pPtIntersect);
	// 得到射线和三角形的相交信息
BOOL GetIntersectTriangle3D(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXVECTOR3 PtA, D3DXVECTOR3 PtB, D3DXVECTOR3 PtC, LPD3DXVECTOR3 pPtBaryCoord, float *pLength, LPD3DXVECTOR3 pPtIntersect = NULL, MYRAYTRACING_INTERSECT *pIntersectAttribute = NULL);
	// 得到射线和包围盒的相交信息
BOOL GetIntersectBox3D(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXVECTOR3 PtMin, D3DXVECTOR3 PtMax);






////////////////////////////////////////////////////////////////////////// 其他函数
	// 转换shader版本信息（字符串，形如"ps_2_a"或"ps.2.a"之类的）为一个整数，主要用于便于相互比较用，整数就是21 22 30之类的
	// 对于特殊版本而言，2.a是22 2.b是21，如果返回是0，表示系统不支持shader或字符串格式有误
	// 最后一个参数表示转换的是VS版本还是PS版本
UINT ConvertShaderVersion(const char *pszVersion, bool bPixelShader = true);
	// 检查设备是否支持ps2.x Shader，第一个参数是Shader占用了多少个临时寄存器，第二个参数是Shader是否使用了分支（静态、动态都算），第三个参数是是否使用无限制级数的依赖纹理寻址，不过它用的较少，而且有替代方案，所以默认不检查
BOOL CheckPS2xSupport(UINT iNumTemp, BOOL bFlowControl, BOOL bNoLimitDependentRead = FALSE);

	// 设置RT，这个RT必须是作为贴图使用的，会自动检测并取消贴图状态，防止Render Bound
	// 深度缓冲可以设置为空，表示不改变当前设置
	// 两个是重载函数，一个用于2D贴图，一个用于Cube贴图
HRESULT SetTexturedRenderTarget(DWORD iMRTNo, LPDIRECT3DTEXTURE9 pRTTex, LPDIRECT3DSURFACE9 pDepthStencil);
HRESULT SetTexturedRenderTarget(DWORD iMRTNo, DWORD iFaceNo, LPDIRECT3DCUBETEXTURE9 pRTTex, LPDIRECT3DSURFACE9 pDepthStencil);
	// 计算菲涅耳系数R(cos)，一般用于计算水面的R(0)，参数一、二分别为出射和射入介质的折射率（ni一般为空气即1.0，no一般为水即1.333），参数三表示cos(theta)，即V dot N
float GetFresnelCoef(float fni, float fno, float fcos);