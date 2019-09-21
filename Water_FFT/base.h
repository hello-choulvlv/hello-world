#pragma once

#define EXPConstant 2.71828183f		// ����e
#define GravityConstant 9.8f		// ��������g

// FP16 Data Format
typedef unsigned short float16;

extern char convstr[20];

//////////////////////////////////////////////////////////////////////////�ַ�������
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

// ��һ���ַ����õ�һ���������������ֶ���ת��int֮�ࣩ���ַ�����ʽΪ��Data1,Data2,Data3����һ������ת��INI�е�����
// iFloatDataNum��ʾֻ��ȡ�ַ����е�ǰ���ٸ�����������Ķ���
// �����ַ������ܹ��ж��ٸ�������
UINT GetFloatFromString(char * pString, float *pRetrieveData, UINT iFloatDataNum = 0);


//////////////////////////////////////////////////////////////////////////��ѧ����
float absf(float fNum);
double absf(double fNum);
unsigned long cf(int n);

// ����Ƿ�2���ݣ�����Ǿͷ����ݣ����ڵ���1����������Ǿͷ���0
// ����0��1�����᷵��0
UINT CheckPowerOf2(UINT iValue);
// λ�������ڸ����Ķ���32λ����iBitNumָ�����ܶ���λ���򣬸�λ���ֲ��䡣����˵11110000_11000110��ָ��8λ����ͱ��11110000_01100011
// ���BitNum = 0���ͻ᷵��ԭֵ
UINT ReverseBitOrder(UINT iValue, UINT iBitNum);

float clampf(float fValue, float fStandardmin = 0.0f, float fStandardmax = 0.0f);
int roundf(float fValue, float fStandard );
int CalcFactorial(int iData);
int CalcDoubleFactorial(int iData);
float randomf(float fRandomValue, UINT iBitNum = 4);

void randomize(void);
int random(int number);

double gaussrandom();	// ��˹�ֲ��������

// ����3x3�����Det�����Ե�4��Ԫ�أ�
float GetDeterminant3X3(D3DXMATRIX Mat);




/////////////////////////////////////////////////////////////////////��ɫת��
	//�����������ʱת�������ã�ǰ������ת��������Dword��ɫ����������ת��float��ɫ��Dword��ɫ
DWORD VectoRGBSigned(float x, float y, float z, float w);
DWORD VectoRGBUnsigned(float x, float y, float z, float w);
DWORD ColortoRGBSigned(float x, float y, float z, float w);
DWORD ColortoRGBUnsigned(float x, float y, float z, float w);
	//FP16 and FP32 Convert
float FP16Tofloat(float16 fValue);
float16 floatToFP16(float fValue);

	// ��DWORD��ɫת��ΪD3DXCOLOR(Vector4)
#define DWCOLORTOVECTOR(Color)	D3DXCOLOR(	(float)((Color>>16)&0xff) / 255.0f,		(float)((Color>>8)&0xff) / 255.0f,	(float)(Color&0xff) / 255.0f,		(float)((Color>>24)&0xff) / 255.0f	)





//////////////////////////////////////////////////////////////////////////��������������Ϳռ�ת������
	// ���ݹ۲췽���ָ����ͷ������������ȷ��ͷ������������������ʵ����ֱ�Ӵ�Look��Head��MatrixLookAtLH�����У���Ҳ���Զ�����Ӧ�Ĳ��裬����ֻ��Ϊ�˵õ������������ᣬ�����ӽǿ���
HRESULT GenerateViewSpace(D3DXVECTOR3 *pVecLookAt_Z, D3DXVECTOR3 *pVecHead_Y, D3DXVECTOR3 *pVecRight_X);
	// ŷ����ת�Ǻ;���
D3DXMATRIX GetXZXEulerMatrix(LPD3DXMATRIX pMat, D3DXVECTOR3 VecEulerAngle);
D3DXMATRIX GetZYZEulerMatrix(LPD3DXMATRIX pMat, D3DXVECTOR3 VecEulerAngle);
D3DXMATRIX XZXEulerRotation(LPD3DXMATRIX pMat, D3DXVECTOR3 VecAxis, float fAngle, LPD3DXVECTOR3 pVecEulerAngle = NULL);
D3DXMATRIX ZYZEulerRotation(LPD3DXMATRIX pMat, D3DXVECTOR3 VecAxis, float fAngle, LPD3DXVECTOR3 pVecEulerAngle = NULL);
D3DXMATRIX ZYZEulerRotation(LPD3DXMATRIX pMat, D3DXQUATERNION Quater, LPD3DXVECTOR3 pVecEulerAngle = NULL);
	// ��������͵ѿ�������ת��
D3DXVECTOR2 CartesianToSpherical(D3DXVECTOR3 VecCarte, LPD3DXVECTOR2 pVecSphere);
D3DXVECTOR3 SphericalToCartesian(D3DXVECTOR2 VecSphere, LPD3DXVECTOR3 pVecCarte);





//////////////////////////////////////////////////////////////////////////�󽻺���
enum MYRAYTRACING_INTERSECT
{
	INTERSECT_OK,				// �ཻ���൱��S_OK��ֻҪ��������һ��״̬���־Ͳ��᷵���ཻ��Ҳֻ�������֣������Ż᷵��TRUE
	INTERSECT_NO,				// ��ͨ�Ĳ��ཻ���൱��FALSE=0
	INTERSECT_OPPOSITEDIR,		// ���ߺ��汳�룬���ཻ����������Ĳ��ཻ���
	INTERSECT_PARALLEL,			// ���ߺ���ӽ�ƽ�У����ཻ�����ڱȽ�����Ĳ��ཻ���
	INTERSECT_SAMEFACE,			// ���������ڸ����ϣ����棬���Ǻ�����Ĳ��ཻ���
	INTERSECT_SAMEPOINT,		// �����������������һ�����㣬������Ĳ��ཻ���
	INTERSECT_INVALIDARGUMENTS,	// ��Ч���������
	INTERSECT_FATALERROR,		// �ڲ�����
};

	// �õ�����ֱ��P1P2��V1V2�Ľ��㣬����޽���ͷ���false
bool GetIntersect2D(D3DXVECTOR3 *P, D3DXVECTOR3 P1, D3DXVECTOR3 P2, D3DXVECTOR3 V1, D3DXVECTOR3 V2);
	// �жϵ�P�Ƿ�����������V1V2V3��2D���ϲ���û�����⣬3Dδ����
bool IsPointInTriangle(D3DXVECTOR3 P, D3DXVECTOR3 PtV1, D3DXVECTOR3 PtV2, D3DXVECTOR3 PtV3);
	// ���߸����󽻣��õ���ƽ��Ľ���
BOOL GetIntersectPlane(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXPLANE Plane, LPD3DXVECTOR3 pPtIntersect);
	// �õ����ߺ������ε��ཻ��Ϣ
BOOL GetIntersectTriangle3D(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXVECTOR3 PtA, D3DXVECTOR3 PtB, D3DXVECTOR3 PtC, LPD3DXVECTOR3 pPtBaryCoord, float *pLength, LPD3DXVECTOR3 pPtIntersect = NULL, MYRAYTRACING_INTERSECT *pIntersectAttribute = NULL);
	// �õ����ߺͰ�Χ�е��ཻ��Ϣ
BOOL GetIntersectBox3D(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXVECTOR3 PtMin, D3DXVECTOR3 PtMax);






////////////////////////////////////////////////////////////////////////// ��������
	// ת��shader�汾��Ϣ���ַ���������"ps_2_a"��"ps.2.a"֮��ģ�Ϊһ����������Ҫ���ڱ����໥�Ƚ��ã���������21 22 30֮���
	// ��������汾���ԣ�2.a��22 2.b��21�����������0����ʾϵͳ��֧��shader���ַ�����ʽ����
	// ���һ��������ʾת������VS�汾����PS�汾
UINT ConvertShaderVersion(const char *pszVersion, bool bPixelShader = true);
	// ����豸�Ƿ�֧��ps2.x Shader����һ��������Shaderռ���˶��ٸ���ʱ�Ĵ������ڶ���������Shader�Ƿ�ʹ���˷�֧����̬����̬���㣩���������������Ƿ�ʹ�������Ƽ�������������Ѱַ���������õĽ��٣��������������������Ĭ�ϲ����
BOOL CheckPS2xSupport(UINT iNumTemp, BOOL bFlowControl, BOOL bNoLimitDependentRead = FALSE);

	// ����RT�����RT��������Ϊ��ͼʹ�õģ����Զ���Ⲣȡ����ͼ״̬����ֹRender Bound
	// ��Ȼ����������Ϊ�գ���ʾ���ı䵱ǰ����
	// ���������غ�����һ������2D��ͼ��һ������Cube��ͼ
HRESULT SetTexturedRenderTarget(DWORD iMRTNo, LPDIRECT3DTEXTURE9 pRTTex, LPDIRECT3DSURFACE9 pDepthStencil);
HRESULT SetTexturedRenderTarget(DWORD iMRTNo, DWORD iFaceNo, LPDIRECT3DCUBETEXTURE9 pRTTex, LPDIRECT3DSURFACE9 pDepthStencil);
	// ���������ϵ��R(cos)��һ�����ڼ���ˮ���R(0)������һ�����ֱ�Ϊ�����������ʵ������ʣ�niһ��Ϊ������1.0��noһ��Ϊˮ��1.333������������ʾcos(theta)����V dot N
float GetFresnelCoef(float fni, float fno, float fcos);