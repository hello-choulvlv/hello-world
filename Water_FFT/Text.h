#pragma once
#include "Shader.h"

#define MAX_GLARE_LINE_NUM 8

// Post Processing��Դ������������ͼ�����ͼ
enum POSTPROCESSING_SOURCETYPE
{
	POSTRTTYPE_SCENE,
	POSTRTTYPE_NORMAL,
	POSTRTTYPE_DEPTH
};

// Ԥ��������ͣ��������GlareData.Init��ʹ��
enum GlareTypeDef
{
	Glare_PerpendicularCross,	// ʮ���λ��ഹֱ��4�����򣬼�Cheap Len����ת���൱��SpectralCross��CrossScreen
	Glare_PerspectCross,		// ʮ���λ��಻��ֱ��4��������Բ�εġ�X��
	Glare_SnowCross,			// 6������һ����������������б����ɵġ�X����������ͬ
	Glare_SpetralSnowCross,		// 6������һ����������������б����ɵġ�X�������Ȳ�ͬ����������б�߶�
	Glare_SunnyCross,			// 8�����򣬳�����ͬ
	Glare_SpetralSunnyCross,	// 8�����򣬳��Ȳ�ͬ����ʮ�ֳ���бʮ�ֶ�
};


//���е����嵱�豸��ʧʱ�������ͷţ����ۺ���POOL
class MYTEXT
{
public:
	tagRECT Rect;       //�����Χ(��������)
	DWORD TextFormat;  //���뷽ʽ��DrawText Format Flags  (DT_XXX)

	float Deviation;     //��������Բ���̶ȣ�ԽСԽԲ��
	float Extrusion;    //��������Z��ȣ�����ȣ�

	MYTEXT();
	~MYTEXT();
	void Release();

	//2D TEXT
	HRESULT InitFont2D(UINT FontSize, LPSTR FontName);
	void Set2DStyle(UINT Left, UINT Right, UINT Up, UINT Down, DWORD Format);
	//�������ʼĬ���Ǵ���㿪ʼ�������У������Ҫ����Ķ���Ч����ִ��Set2DStyle
	HRESULT DrawText2D(LPSTR Content, UINT Length, UINT Up, UINT Left, unsigned char r, unsigned char g, unsigned char b);
	//���غ�������ʹ�ð�͸��������alphaֵ��
	HRESULT DrawText2D(LPSTR Content, UINT Length, UINT Up, UINT Left, unsigned char r, unsigned char g, unsigned char b, unsigned char alpha);

	
	//3D TEXT������֧�����������е�Ӣ�ļ��ַ�������֧�����ġ�ע�⻭��ʱ�򣬴򿪵ƹ�������һ��Ҫ���ò��ʣ���֮һ���ùرյƹ⣬�������ʾ����
	//deviation��ʾԲ���̶ȣ���ʵ���Ƕ���������ԽС��ʾ������Խ�࣬������ģ��ҲԽԲ����TrueType�����һ�����ԣ�����С��0�����Ϊ0����ʹ�������Ĭ��ֵ��һ��Ϊ0.0xx
	//extrusion��ʾģ�ͺ�ȣ���Ҫ̫��ֵΪ��׼����ռ�����곤��
	HRESULT InitFont3D(UINT FontSize, LPSTR FontName, LPSTR Content, float deviation, float extrusion);
	void SetMaterial(float DiffuseR, float DiffuseG, float DiffuseB, float SpecularR, float SpecularG, float SpecularB, float EmissiveR, float EmissiveG, float EmissiveB, float Power);
	HRESULT DrawText3D();

private:
    UINT Create2DFontAttrib;     //2D���崴����־
	LPD3DXFONT DFont;  //D3Dʹ�õ�2D����
	LPD3DXMESH Mesh;   //D3Dʹ�õ�3D����ģ��
	D3DMATERIAL9 Material;  //3Dģ�͵Ĳ���


//�����ʽ�ĺ꣺DT_
//���õģ�CALCRECT  NOCLIP  TOP LEFT RIGHT BOTTOM  CENTER  VCENTER
//���У�INTERNAL   WORDBREAK   SINGLELINE   EXPANTABS  TABSTOP   EXTERNALLEADING   NOPREFIX
};







//�������û��DOS->D3D�����Ǹп���֣��÷�����ͬ
//һ���Զ���ɣ��������TEXTһ��������Ӱ���κ����õ�
//����ע����������������ɫ�޹أ��������ص�ALPHAֵ������Ҳ����ˣ���ʵӦ�ý�ymalpha�Ŷ�
//��û��alphaͨ����Ϣ��ͼƬ����alpha��ϵ�ʱ�򣬿����ڵ���PUTBMP֮ǰ��TSS_ALPHAOP����ͨ��ΪTFACTOR
//���ж����alphaֵ��ԽСԽ͸������GRAPH16�պ��෴
class MYIMAGE
{
public:
	UINT AlphaEnable;     //�Ƿ������͸�������Ҫ�ڳ����ж�̬�ı�
	UINT TextureNum;            //�Ѵ���������������ʼΪ0

	MYIMAGE();
	~MYIMAGE();
	//�õ���ģ�ʹӿ�ʼ�����ڵ�ʱ�䣬��λ�루GETTICKCOUNT����ֵ1����λԼΪ55���룩
	float GetTime();
	void Release();

	//��ָ���������ݿ����Դ��ڵĶ��㻺������BufferPoint�Ƕ������������ָ�룬PointNum�Ǳ�ʾ�ж��ٸ����㣬EPointSize��ÿ���������ݵĴ�С��һ����sizeof(CUSTOMVERTEX)����VertexShader������Ķ���Ĵ����С
	HRESULT Init();
	//����������ɫ��TESTFUNC��EQUAL
	HRESULT Setymcolor(unsigned char a);
	//����4�������DIFFUSE�����ηֱ��ʾ���ϡ����ϡ����¡����µĵ�
	void Setcolor(unsigned char r0, unsigned char g0, unsigned char b0, unsigned char a0, unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1, unsigned char r2, unsigned char g2, unsigned char b2, unsigned char a2, unsigned char r3, unsigned char g3, unsigned char b3, unsigned char a3);

	//������TEXTURENUM���������������Զ���1�����ܳ������ɴ���������
	HRESULT Loadbmp(LPSTR Filename);
	//����ָ������ֻ��������ALPHAͨ��ͼ���������0,���ú���ALPHAOP=selectarg1, alphaarg1=ͨ��ͼ�������ɣ���������ǵûָ�����
	HRESULT Setalphabmp(UINT picno, int picx, int picy, int width, int height);
	//ʲôʱ����ALPHAͨ��ͼ�ˣ�����������ָ�
	void Restorealphabmp();

	//��������Ժ�ALPHAͨ��ͼ��ͬ���õ�
	HRESULT Putbmp(UINT picno, int startx, int starty, int endx, int endy, int picx, int picy, int width, int height, UINT ym);
	HRESULT Putcolorbmp(UINT picno, int startx, int starty, int endx, int endy, int picx, int picy, int width, int height, UINT ym);
	HRESULT Putcolor(int startx, int starty, int endx, int endy, UINT ym);

private:
	//����ͻָ���������㡢��Ⱦ״̬����
	void Save();
	void Restore();
	DWORD Time; //ʱ�����
	UINT EachPointSize;   //ÿ��������ռ�洢�ռ�Ĵ�С��һ����sizeof(MYIMAGEVERTEXTYPE)
	UINT CreateAttrib;  //���ԣ�0��ʾδ��ʼ��

	//�Զ�������
	DWORD ymcolor;
	//�Ƿ�������ALPHAͨ��ͼ
	UINT alphabmpenable;
	//�������ݺ���������
	struct MYIMAGEVERTEXTYPE
	{
		float x,y,z,rhw;
		DWORD diffuse;
		float tu,tv;
		float atu,atv;
	}MyImageVertex[4];  //������

	LPDIRECT3DVERTEXBUFFER9 VertexBuffer;  //���㻺����
	LPDIRECT3DTEXTURE9 Texture[MYD3D_MODULETEXTURENUM];   //�������8�����Ͳ����޹�
	DWORD VertexShader;         //�����ʽ,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1

	//�����Ǳ���Ķ���
	LPDIRECT3DBASETEXTURE9 BackupTexture, AlphaTexture;
	DWORD ColorOP, ColorArg1, ColorArg2;
	DWORD ColorOP_1, ColorArg1_1, ColorArg2_1;
	DWORD AlphaOP, AlphaArg1, AlphaArg2;
	DWORD AlphaOP_1, AlphaArg1_1, AlphaArg2_1;
	DWORD AlphaBlend, SrcBlend, DestBlend;
	DWORD AlphaTest, TestFunc;
};






// Glare����������
struct GlareLineData
{
	D3DXVECTOR2 VecDir;	// ����
	UINT iLength;		// ���ȣ���λ�����أ�ע�ⲻҪ̫��
	float fAttenuation;	// ˥��ϵ������λ%��ֵ�������0��Ҳ���Դ���1��ʾ������Ĭ��Ϊ1�����������������һ�����ű��ʡ�ע��˥���Ȳ��ܹ��ͣ�����GlareЧ���ͻ���������������ܻῴ����
	D3DCOLOR Color;		// ��ɫ��Ĭ��Ϊ��ɫ
	
	GlareLineData()
	{
		VecDir = D3DXVECTOR2(0, 0);
		iLength = 0;
		fAttenuation = 1.0f;
		Color = D3DCOLOR_ARGB(255, 255, 255, 255);
	}
};

// Glare����
struct GlareData
{
	UINT m_iLineNum;	// �����������ߣ����򣩣����ǵ����յ�Glare��ͼ����ӣ���ֵ���ɴ���MAX_GLARE_LINE_NUM
	D3DXMATRIX m_MatRotate;	// ������ת
	GlareLineData *m_pLineData;	// ����������

	GlareData()
	{
		m_iLineNum = 0;
		D3DXMatrixIdentity(&m_MatRotate);
		m_pLineData = NULL;
	}
	~GlareData()
	{
		Release();
	}


	void Release()
	{
		m_iLineNum = 0;
		D3DXMatrixIdentity(&m_MatRotate);
		SAFE_DELETE_ARRAY(m_pLineData);
	}

	// ����������ת��2D����ֻ����Z��ת��
	void SetRotate(float fAngle)
	{
		D3DXMatrixRotationZ(&m_MatRotate, fAngle);
	}


	// �������ó�ʼ��Glare���ݣ��ɶ�γ�ʼ��
	HRESULT SetGlareData(UINT iLineNum, LPD3DXVECTOR2 pVecDir, UINT *pLength, float *pAttenuation = NULL, LPD3DCOLOR pColor = NULL)
	{
		if(!iLineNum || iLineNum > MAX_GLARE_LINE_NUM || !pVecDir || !pLength)
			return D3DERR_INVALIDCALL;

		// �ɶ�γ�ʼ��
		if(m_iLineNum != iLineNum)
		{
			SAFE_DELETE_ARRAY(m_pLineData);
			m_pLineData = new GlareLineData[iLineNum];
		}
		else
			m_pLineData = new GlareLineData[iLineNum];

		m_iLineNum = iLineNum;

		UINT i = 0;
		for(i = 0; i < m_iLineNum; i++)
		{
			D3DXVec2Normalize(&m_pLineData[i].VecDir, &pVecDir[i]);
			m_pLineData[i].iLength = pLength[i];
		}
		if(pAttenuation)
		{
			for(i = 0; i < m_iLineNum; i++)
				m_pLineData[i].fAttenuation = pAttenuation[i];
		}
		if(pColor)
		{
			for(i = 0; i < m_iLineNum; i++)
				m_pLineData[i].Color = pColor[i];
		}
		
		return S_OK;
	}

	// ���غ�����ʹ�ö���õ�Glareģ��
	HRESULT SetGlareData(GlareTypeDef enuGlareType)
	{
		if(enuGlareType == Glare_PerpendicularCross)
		{
			D3DXVECTOR2 VecDir[4] = {D3DXVECTOR2(1, 0),	D3DXVECTOR2(-1, 0), D3DXVECTOR2(0, 1), D3DXVECTOR2(0, -1)};
			UINT iLength[4] = {15, 15, 15, 15};
			V_RETURN(SetGlareData(4, VecDir, iLength));
		}

		if(enuGlareType == Glare_PerspectCross)
		{
			D3DXVECTOR2 VecDir[4] = {D3DXVECTOR2(1.7f, 1),	D3DXVECTOR2(1.7f, -1), D3DXVECTOR2(-1.7f, 1), D3DXVECTOR2(-1.7f, -1)};
			UINT iLength[4] = {15, 15, 15, 15};
			V_RETURN(SetGlareData(4, VecDir, iLength));
		}

		if(enuGlareType == Glare_SnowCross)
		{
			D3DXVECTOR2 VecDir[6] = {D3DXVECTOR2(1, 0),	D3DXVECTOR2(-1, 0), D3DXVECTOR2(1, 1), D3DXVECTOR2(1, -1), D3DXVECTOR2(-1, 1), D3DXVECTOR2(-1, -1)};
			UINT iLength[6] = {15, 15, 15, 15, 15, 15};
			V_RETURN(SetGlareData(6, VecDir, iLength));
		}

		if(enuGlareType == Glare_SpetralSnowCross)
		{
			D3DXVECTOR2 VecDir[6] = {D3DXVECTOR2(1, 0),	D3DXVECTOR2(-1, 0), D3DXVECTOR2(1, 1), D3DXVECTOR2(1, -1), D3DXVECTOR2(-1, 1), D3DXVECTOR2(-1, -1)};
			UINT iLength[6] = {15, 15, 8, 8, 8, 8};
			V_RETURN(SetGlareData(6, VecDir, iLength));
		}

		if(enuGlareType == Glare_SunnyCross)
		{
			D3DXVECTOR2 VecDir[8] = {D3DXVECTOR2(1, 0),	D3DXVECTOR2(-1, 0), D3DXVECTOR2(0, 1), D3DXVECTOR2(0, -1), D3DXVECTOR2(1, 1), D3DXVECTOR2(1, -1), D3DXVECTOR2(-1, 1), D3DXVECTOR2(-1, -1)};
			UINT iLength[8] = {30, 30, 30, 30, 30, 30, 30, 30};
			float fCoef[8] = {0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f};
			V_RETURN(SetGlareData(8, VecDir, iLength, fCoef));
		}

		if(enuGlareType == Glare_SpetralSunnyCross)
		{
			D3DXVECTOR2 VecDir[8] = {D3DXVECTOR2(1, 0),	D3DXVECTOR2(-1, 0), D3DXVECTOR2(0, 1), D3DXVECTOR2(0, -1), D3DXVECTOR2(1, 1), D3DXVECTOR2(1, -1), D3DXVECTOR2(-1, 1), D3DXVECTOR2(-1, -1)};
			UINT iLength[8] = {30, 30, 30, 30, 15, 15, 15, 15};
			float fCoef[8] = {0.7f, 0.7f, 0.7f, 0.7f, 0.5f, 0.5f, 0.5f, 0.5f};
			V_RETURN(SetGlareData(8, VecDir, iLength, fCoef));
		}

		return S_OK;
	}

};



















/*����Filter VS����ʱ��ģ��
D3DVERTEXELEMENT9 dclr[] = {
	{0,0,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0,12,D3DDECLTYPE_FLOAT2 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};*/
class KPostProcess
{
public:
	KPostProcess();
	~KPostProcess();
	void Release();
	/***************ϵͳ���ܽӿ�********************/

	// ֻ�ǳ�ʼ��Full-Screen Quad�Ķ��㻺���Shader���ˣ���Ϊ�����ʽ�͵�ǰ��Ⱦ��RT��ʽ�йأ����Զ��ŵ�StartRender�н���
	HRESULT Init();
	
	// ר�����ڵõ���ǰ�����������ǿ��ת��Ϊԭ�󣩣�����EdgeGlow��SoftShadow�������Ǹ��Ƴ����ģ�����Ҫ�ֶ��ͷ�
	HRESULT GetCurrentImage(LPDIRECT3DTEXTURE9 *ppTex);

	// Start/End Render֮������Ļ��潫�����ص��ö����ڲ��ĵȴ�СRT�ϣ����������ͨ��Draw��PostProcess��HDR/EndProcess
	// �����е������������false����ʹ��MRT�����ͱ�ʾ��Ҫ�������ͼ�ͷ���ͼ����ô��Ҫ����MRT�����øò�����Ϊ������ٶȣ�������õĻ��Ͳ�����MRT�����������������ܣ�
	HRESULT StartRender(bool bHDR = false, bool bMRT = false);
	HRESULT EndRender();
	
	// ��ʼ����MRT
	HRESULT StartMRT(bool bNormal = true, bool bDepth = true);
	HRESULT EndMRT();
	// ��һ�Ժ���������ʼ����Start�ǳ�ʼ���ڲ����ݣ�Ȼ��������Ҫ��PostProcess�������ɣ������EndProcess��ָ����shader��Process�õĻ���ϳɵ�ԭ����RT�ϣ�����ֱ������ˣ�������Draw��
	// ͬһ��PostProcess�����ڵ���һ��Start/EndRender����Զ�ε���Start/EndProcess����������Ͷ����Բ�һ�����ú������Զ�����ScaleScene->Temp1��m_pTex/RTָ�룬����ͬһ��KPost�������ͬʱ����Bloom��Edge��Gaussian��ͨ����ε���Start/EndProcess��ʵ��
	HRESULT StartProcess();
	// ֻ������ɫ�ʳ�����Temp1��������m_pTex/RTָ�룬Ϊ����ͬһ��Process��ͬʱ����ɫ�ʳ����ͷ��ߡ����ͼ
	HRESULT ResetProcess();

	
	// ��EndProcess��ʹ�õ�shader����ʾ��ν���Start/EndRender���ص�ԭʼLDR���棨��m_pSourceScene��m_pToneMap����ʹ����ָ����ͼ����Ϊ������Ƶ���ǰ��RT��ȥ
	// ��Щ����1��7����ͬ�����صĻ��棨m_pSourceScene/m_pToneMap��Ϊ��0������ʹ���Զ����shader���Ƶ���ǰRT�ϣ����ڶ��PostProcessing֮��Ļ�ϣ���������Ӱ��EdgeGlow��

	// ���pixel shaderָ��Ϊ�գ���ֱ��Copy����ǰRT�ϣ������Ϊ����ʹ�ø�shader��ϣ�shader����Ҫ�����ⳣ������Ⱦ������Ҫ��EndProcess֮ǰ���ú�
	// VERTEX SHADER����MultiTextureBlend.vsh��8�����������ȷ�Ĵ��룬��PS��ֱ��Ѱַ����
	
	// �Զ������������Ԥ�ȱ����Post�������������Ӱ��EdgeGlow֮�֧࣬���Զ�����7�������0���ڲ�m_pTexture��1��7���Զ�ͼ��
	// ����TexNum��ָ��һ���ж��ٸ�������ű����ppTexture���ݶ�Ӧ��Ĭ������
	
	// HDR���Ҳ������������ǰ����Ҫ�õ����е�����Bloom/Glare/EdgeGlow/SoftShadow����Ԥ�ȼ����ToneMap��������ڵ�0�㣨����HDR ToneMapping�Ļ��ͻ��Զ�ȡ��SourceScene��
	HRESULT EndProcess(PIXELSHADER* pPS = NULL, UINT iTexNum = 0, LPDIRECT3DTEXTURE9 *ppTexture = NULL);

	// ���EndProcess��ǿ�ƽ���һ��PostProcess�����������ڲ����ݺͻָ�״̬��ԭRT����ʲô��������һ��Ҫ��GetCurrentImage���ã���������Ӱ��
	HRESULT ForceEndProcess();

	/**************Post Process���ܽӿ�******************/
	// ��˹���������ڷ���ͼ�ı�Ե��ǿ�Ǳ�¶���ⲿ�������ظ�ʹ�ã�������ͬ����С�ģ����Կ����ں����ڲ�����m_pTex/RT��ָ�루������Temp1/Temp2��
	HRESULT Gaussian1D(bool bHorizon, float fRadius = 5.0f);
	HRESULT Gaussian5x5(float fRadius = 20.0f);
	HRESULT EdgeEnhanced(bool bIncr = true, bool bEdgeColorWhite = true);	// ��Ե��ǿ��true��ʾ���ӣ�false��ʾ�������ɶ��ʹ��

	// ǿ�ƽ����߻����ԭ��ͼ�����ݲ���������Edge Detection���洢����ʱ�У�Ȼ��ת�浽pTemp1������m_pTex/RT�������������ó�������ȼ�ã��ֿ��Ա��ⷨ��ͼ�ڲ�����ֵʱ�������
	// ����HDR/LDR�����ǲ��еģ���ȫ��Ӱ��ԭ���������ֻ��Ϊһ�����ӵ�ͼ�����ձض���pTemp1��
	HRESULT EdgeDetection(POSTPROCESSING_SOURCETYPE ProcRTType = POSTRTTYPE_NORMAL, float fThreshold = 0.0f, bool bEdgeColorWhite = true);

	// �ѳ�����EDGEͼ������˹ģ�������������m_pTexture�У���GetImage�õ����ɣ�����ģ��ͼ����Ҫ̫���������ԱȽ�С������Tone Mappingһ����ר�õ���ͼ�����Ա�����GetImage���Ƴ���
	HRESULT EdgeGlow(POSTPROCESSING_SOURCETYPE ProcRTType = POSTRTTYPE_NORMAL, float fThreshold = 0.0f, bool bEdgeColorWhite = true);	//�߼������Ч������ǰ��m_pTex�������Ѿ��û�����õı�Եָ�룩BrighterPass�͸�˹ģ�������õ���EdgeGlowͼ

	

	// �趨ʹ�õ�Glare����
	HRESULT SetGlare(GlareData *pGlare)
	{
		if(!pGlare)
		{
			m_bGlare = false;
			OutputDebugString("Glare�ѹرգ�");
			return S_OK;
		}

		if(!pGlare->m_iLineNum || pGlare->m_iLineNum > MAX_GLARE_LINE_NUM)
			return D3DERR_INVALIDCALL;
		
		m_GlareData.Release();

		m_GlareData.m_iLineNum = pGlare->m_iLineNum;
		m_GlareData.m_MatRotate = pGlare->m_MatRotate;
		m_GlareData.m_pLineData = new GlareLineData[m_GlareData.m_iLineNum];
		for(UINT i = 0; i < m_GlareData.m_iLineNum; i++)
		{
			D3DXVec2Normalize(&m_GlareData.m_pLineData[i].VecDir, &pGlare->m_pLineData[i].VecDir);
			m_GlareData.m_pLineData[i].iLength = pGlare->m_pLineData[i].iLength;
			m_GlareData.m_pLineData[i].fAttenuation = pGlare->m_pLineData[i].fAttenuation;
			m_GlareData.m_pLineData[i].Color = pGlare->m_pLineData[i].Color;
		}

		m_bGlare = true;
		return S_OK;
	}

	// ���������������ö�����ԴRT���������κ�ʱ�򶼿�������ʹ�ö�����Ӱ��m_pTex/RTָ��
	// ����Bloom�Ƿŵ�TempRT�еģ�����ִ�������Ҫ����GetImage����HDR2LDR������Ŀ��RT����ר�õģ�ֱ�ӱ��淵�ص�ָ�뼴��
	// Bloom����HDR��Χ��һ��Ч�������κ�Glare������������
	HRESULT Bloom(float fKeyValue=0, float fClampValue=0, float fOffset=20.0f);	// ����HDR����������HDR Bloom����LDR Bloom
	// ֻ��ӳ��HDR��LDR�Ĳ������ѣ�ӳ�����m_pTexָ��ר����ͼToneMapRT��EndProcessʱ���Զ�����Ϊ��0������
	HRESULT HDR2LDR(float fKeyValue = 0.18f);
	// ģ��Depth Of Field�������Ǿ۽�ƽ�桢Զ��ƽ�棬������ο�DOFԭ��ʹ��DOFǰһ��Ҫָ��FP16��FP32�������ǿ�������������ִ�����
	// ���һ��������ģ��ǿ�ȣ�����COF�����뾶����ֵԽ��Ч��Խģ����Ĭ��3.0��ע��̫�͵Ļ����ܿ�����ʲôģ��Ч��Ŷ
	HRESULT DOF(float fFocalPlane, float fNearPlane = 0.0f, float fFarPlane = 0.0f, float fBlurPower = 3.0f);
	
	

	
private:
	// �ڲ�ʹ�õ���Ϲ��ܴ�����
	HRESULT Resize();	// �������������Զ�ѡ��Copy��shader����Resize��Ӳ������Resize
	HRESULT MeasureLuminance();	// ͳ��ȫ�����ȣ��Զ�����shader�汾ѡ��ps2.0���Ǹ��ߵĲ���
	HRESULT BrightPass(float fKeyValue, float fClampValue, float fOffset);	// KeyValue��LDR�о���Bright-Coef��fOffsetֻ��HDR����Ч����ScaleScene��temp1��ǿ����ָ��
	HRESULT CalcAdapt();	// ����ȫ���ڲ�����������ʹ��m_pTex/RTָ��
	// ���ܽ���pTex/pRTָ�룬��Ϊǣ��������̫�㣬�������ⲿ�ӿڵ�������ʱ�ֶ�����ָ����ȷ��������ȷ��
	// ToneMap��Ҫ�����Ĵ�ͼ������ֱ����ԭ���ͼ�������������Ҫר����ͼ�������������������ͼ��ת������ר�õ���ͼ�У�����SourceScene->ToneMap����ǣ��m_pTex/RTָ��
	HRESULT ToneMapping(float fKeyValue);

	// �ڲ�ʹ�õĻ������ܴ������������ڲ�ͬ��С�������̣����Բ�Ҫ���ڲ�����ָ��
	HRESULT Glare();		// һ�λ���ģ�����з������ͼ��������Ӧ��Glare��ͼ����ȫ�ڲ����ã����ı�m_pTex/RTָ�룩
	HRESULT AddGlare();		// ��������ЩGlare��ͼ���ӵ�BrightPass�У���ȫ�ڲ����ã����ı�m_pTex/RTָ�룩
	HRESULT DOFShader(LPDIRECT3DTEXTURE9 pTexSource, LPDIRECT3DTEXTURE9 pTexBlurred, float fFocalPlane, float fNearPlane, float fFarPlane, float fBlurPower);	//����ps.2.a��Ӳ�������ã�
	HRESULT MeasureLuminanceShader();	//����ȫ���ڲ�����������ʹ��m_pTex/RTָ�룬ֱ�ӽ����ݿ�����m_pLumAdaptFrameRT��
	HRESULT MeasureLuminanceShaderPS20();	// ͬ�ϣ�������ps2.0��Ҫ����һ��pass�����ƣ���log��ds4x4�ֿ������ģ��������ز��࣬����Ӱ������
	HRESULT CalcAdaptShader(float fFrameTime, float fIdleTime);	//����ȫ���ڲ�����������ʹ��m_pTex/RTָ��
	HRESULT CopyResize(bool bUseOldRT = false);	// ��С��ͬ������¾���Copy����ͬʱ����Resize������Ӳ��������ˡ����и���������ã���������m_pRTֱ�������m_pOldRT����Ϊ������ָ�����Ͳ�ͬ��������m_pRT����OldRT������EndProcess�п�������Ҫ�����OldRT Surface
	HRESULT DownScale2x2();
	HRESULT DownScale4x4();
	HRESULT BilinearResize(bool bUseOldRT = false);	//ǿ����Shader���ˣ����ܽϵ͡����и���������ã���������m_pRTֱ�������m_pOldRT����Ϊ������ָ�����Ͳ�ͬ��������m_pRT����OldRT������EndProcess�п�������Ҫ�����OldRT Surface
	HRESULT EdgeDetectionShader(POSTPROCESSING_SOURCETYPE ProcRTType, float fThreshold, bool bEdgeColorWhite);	//��������������Ĵ�Shader��Ե��⣬���ý���ָ�룬��ȫ����Ϻ�������
	

	/**************�ڲ�����*******************/
	struct MYIMAGEVERTEXTYPE
	{
		float x,y,z;
		float tu,tv;
	};
	
	D3DSURFACE_DESC m_DescRT, m_DescDepth;		// ԭRT��Depth�����������ÿ�ζ�����StartRender�г�ʼ��
	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer, m_pBilinearVertexBuffer;	// ���㻺�������ڶ�������FP Bilinear Filtering
	LPDIRECT3DSTATEBLOCK9 m_pStateBlock;		// ������Ⱦ������״̬

	// CreateAttrib:	1��ʾ��ʼ���ɹ� 2��ʾ�Ѿ�StartRender 3��ʾ�Ѿ�EndRender����δStartProcess���Ѿ�EndProcess��  4��ʾ�Ѿ�StartProcess
	// MRTAttrib:		1��ʾ����MRT�ɹ�(StartRender),��δStartMRT�κ�һ��  2��ʾ�Ѿ�StartMRT Normal   3��ʾ�Ѿ�StartMRT Depth   4��ʾNormal/Depth���Ѿ�StartMRT
	//					5 6 7�ֱ��ʾNormal/Depth/Both ���Ѿ�EndMRT
	UINT m_iCreateAttrib, m_iMRTAttrib;
	
	// �ֱ��ʾ�Ƿ��Ѿ������ȫ�����ȡ��Զ���Ӧ���Ⱥͱ�Ե��⣬��ΪLum/Adapt���ֹ��ò��裬��Bloom/Tone�ж��п��ܵ��ã�Ϊ���������������������Ա��ж�
	bool m_bMeasureLuminance, m_bCalcAdapt, m_bCalcEdge, m_bCalcToneMap;

	// �������ã��Ƿ�MRT���Ƿ�HDR������ѡ�������������������������������
	bool m_bMRT, m_bHDR;

	// �Ƿ�ʹ��Glare��ֻ�е���SetGlare��Glare����Ч
	bool m_bGlare;
	GlareData m_GlareData;	// �û��趨��Glare����
	

	/********************* �ڲ�Texture ************************/
	// ͼ�����ʱ���ǹص�Z����ģ����ӵ���ЩZ����ֻ��Ϊ��ƥ����ͬ��С��RT������������Ⱦ����
	// ���е���ͼ��ʽ����OldRTһ���������ǰ������HDR����ô��Щ��ͼ�����HDR������Ϊ��ͨ��������
	
	// ����StartRenderʱ��RT����Ȼ��壬������
	LPDIRECT3DSURFACE9 m_pOldRT, m_pOldDepth;
	
	// �½�һ��������Ȼ��壬��ʵ����ҪZ����ģ�������Ϊ������ȾRTʱ������ȥ����Ȼ�����
	LPDIRECT3DSURFACE9 m_pFullSceneDepth;

	// ԭ����ͼ���³���ͼ���ȴ�
	LPDIRECT3DTEXTURE9 m_pSourceSceneRT;
	// �����ķ���ͼ�����ͼ����ԭͼ�ȴ�����MRT����StartRender��ʱ����ݲ������ã�NormalEdge����ʱʹ�õģ����ڽ�Normal����ת��ΪEdge������ԭ��
	LPDIRECT3DTEXTURE9 m_pDepthSceneRT, m_pNormalSceneRT;

	// ��ʱʹ�õ���ͼ��NormalEdge��ԭ�󣬴�ű�Եͼ��DepthEdge��1/4�������ݴ����Թ��˺�����ͼ
	LPDIRECT3DTEXTURE9 m_pEdgeTemp1RT, m_pEdgeTemp2RT, m_pDepthEdgeRT;
		
	// ������С1/4������Post Processing����Դ�����ͳ������ŵ�256x256��ֻ����ͳ��ȫ�����ȣ���256TempRT������ps2.0���ȼ���log��DownScale��ʱʹ�õ�
	LPDIRECT3DTEXTURE9 m_pScaleSceneRT, m_p256x256SceneRT, m_p256x256TempRT;
	
	// ��ʱʹ�ã�1/4ԭͼ��С�����ڲ������������������Ժܴ�̶����������ܣ������ǲ���ҪBilinear�ˣ�
	LPDIRECT3DTEXTURE9 m_pTemp1RT, m_pTemp2RT;
	
	// ��Ӧ��������(R16F)��1x1����ͼ����ʾ��ǰ�������ȡ��ϴ��ƶ�ǰ�ĳ������ȡ���֡�����������Ӧ���ȣ���ֱ������ӳ�䵽LDR�����ȣ�
	LPDIRECT3DTEXTURE9 m_pLumAdaptPrevFrameRT, m_pLumAdaptCurrentFrameRT;

	// ����������ToneMap��ԭͼ��С��BloomҪ�󾫶Ȳ��ߣ�������1/4��С�����ˣ���pTempRT�У�
	LPDIRECT3DTEXTURE9 m_pToneMapRT;

	// ��Ⱦ�õ�Glare��ͼ��ÿ�Ŵ���һ����β���򣬶���ÿ�Ŷ����ܻᱻ��ͬһ������ģ����Σ�������β���ȣ�
	LPDIRECT3DTEXTURE9 m_ppGlareRT[MAX_GLARE_LINE_NUM];	// ���MAX_GLARE_LINE_NUM������
	LPDIRECT3DTEXTURE9 m_pGlareTemp1RT, m_pGlareTemp2RT;	// ��ʱʹ�ã����ڶ��ģ��
	
	// ����������Depth Of Field����ԭͼ��С��LDR
	LPDIRECT3DTEXTURE9 m_pDOFRT;

	// ��256x256Scene(256)->1x1����ͼ(R16F)�����Ը���ͼ��0��3������64*64 16*16 4*4 1*1��ֻ����ͳ��ȫ�����ȣ����һ����Զ�Ǽ�ʱ�������֡��HDRȫ�����ȣ�����Ӧ��ӳ���޹أ�ͬһ��HDR����������������Ӧ��ӳ�䣬ȫ�����ȶ�����ȫһ���ģ�
	LPDIRECT3DTEXTURE9 m_ppTexDownTo1[4];

	// ָ�룬��Զָ��ǰRT����ǰTexture�͵�ǰ�ʺϵ�Depth���ڲ�����
	LPDIRECT3DTEXTURE9 m_pTexture, m_pRT;
	LPDIRECT3DSURFACE9 m_pDepth;	// ��ʵ����Զָ��m_pFullSceneDepth������ֻ�Ƿ���������ˣ������������ൽһ��
	
	
	/********************* �ڲ�Shader ************************/
	
	// ���û���QUAD��shader���Թ��˺ʹ������ճ�����Vertex Shader
	VERTEXSHADER m_VSDrawQuad, m_VSBilinearResize, m_VSMultiTextureBlend;
	
	// ��˹ģ��
	PIXELSHADER m_PSGaussianH, m_PSGaussianV, m_PSGaussian5x5;
	
	// DownScale������������͵�ƽ��ֵ�����Ա��뱣֤Ѱַ��Texel���ģ�����������Ҫ�ڲ壬��POINT����
	PIXELSHADER m_PSDownScale4x4, m_PSDownScale2x2, m_PSCopyResize;

	// ����������shader�������Թ��ˣ�����Bloom��FP32��FP16��ԭ��Ĺ��̣������ط�������Ҫʹ�ã�
	PIXELSHADER m_PSBilinearResize;
	
	// ͳ��ȫ�����ȣ��м�Ĳ������DownScale����First����log��ʹ�����ͣ�Last����exp��ʹ������
	PIXELSHADER m_PSLumLogFirst, m_PSLumLogFirstWithoutDS4x4, m_PSLumLogLast;
	// ���������Զ���Ӧ
	PIXELSHADER m_PSCalcAdapt;
	
	// ��Ե��⼰��Ե�������
	PIXELSHADER m_PSSobel3x3, m_PSNormal2x2, m_PSDepth2x2;
	PIXELSHADER m_PSEdgeIncrWhite, m_PSEdgeDecrWhite, m_PSEdgeIncrBlack, m_PSEdgeDecrBlack;
	
	// HDR��أ�ToneMapping��Bloom
	PIXELSHADER m_PSToneMapping, m_PSBrightPass, m_PSBrightPassLDR;

	// Glare
	PIXELSHADER m_PSGlare, m_PSAddGlare;

	// �������
	PIXELSHADER m_PSDOF;	// Depth Of Field
	
	
	/**************���ڲ��ӿڣ�������ã�******************/

	// �������RT DEPTH��ָ��ͱ�ǣ����˶��㻺���Shader���ⶼ�����������
	void FreeTex();

	// ��StretchRect����ͬ��С������
	HRESULT CopyTexture(LPDIRECT3DTEXTURE9 pTexSrc, LPDIRECT3DTEXTURE9 pTexDst);
	
	// �õ�1D��˹����Ȩ��
	float GetGaussianWeight(float fX, float fY, float fRadius);

	// ���ø�˹�����������Ĵ�����1D��-8��8�ģ�H��V��ʹ���ĸ�shader����������5x5�ͱ�ʾ5x5�������κβ�����û�У���RadiusԽ��ͼ��Խģ��
	void SetGaussian1D(float fRadius);	//���õ�c10/11/12ΪȨ��   c31ΪFix
	void SetGaussian5x5(float fRadius);	//���õ�c20/21/22/23ΪȨ��   c31ΪFix

};