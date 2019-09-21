#include "myd3d.h"
#include "ProcXFile.h"
#include "MeshAnim.h"
#include "EveryModule.h"
#include "Shader.h"
#include "Text.h"
#include "Texture.h"
#include "Water.h"

#include <vector>
using namespace std;


// ��ʾ��ע���CameraChange�е�ProjTransformԶƽ��Ϊ250.0f��
D3DXVECTOR3 g_PtCameraInitPos(120, 25, -60);
D3DXVECTOR3 g_VecCameraInitDir(-1, -0.4f, 0);
//D3DXVECTOR3 g_PtCameraInitPos(120, -5, -60);
//D3DXVECTOR3 g_VecCameraInitDir(-1, -0.4f, 0);

extern UINT g_iCurrentEffectNo;
HRESULT InitPlane(UINT nXSplitNum, float fWidth, UINT nZSplitNum, float fDepth, LPD3DXMESH &pMesh);
HRESULT ChangeWaterMesh();
HRESULT MyComputeTangent();
HRESULT InitFresnelTexture(UINT iWidth);


#define CONFIG_FILENAME ".\\fft+wave.ini"
#define WATER_MIN_FPS 8


//�ڴ˶������е������ģ��
MYTEXT Text;         //3#
VERTEXSHADER g_VSVTF, g_VSNoVTF;
PIXELSHADER g_PSVTF, g_PSNoVTF;
KFFTOceanWater g_FFTOceanWater, g_FFTOceanWaterHighFrequency;
OCEANWATER_ATTRIBUTE g_OceanData, g_OceanDataHighFrequency;


LPD3DXMESH g_pMesh = NULL, g_pMeshCPU = NULL;// ����CPU Lock�����µ�Mesh
LPDIRECT3DTEXTURE9 g_pTexHeightMapCPU = NULL, g_pTexNormalMapCPU = NULL, g_pTexTangentMapCPU = NULL;	// �߶�ͼ�����ߺ�����ͼ��������CPU Lock��GPU��GetHeight/NormalMap���õ�����ָ�뼴�ɣ�
LPDIRECT3DTEXTURE9 g_pTexAddOnNormalMap = NULL;		// ���޷���GPU���ɷ���ͼ������£�����Ϊ���ķ���ͼ
LPDIRECT3DTEXTURE9 g_pTexAddOnHeightMap = NULL;		// ����Ч�������ڵ��ӵĸ߶�ͼ
LPDIRECT3DTEXTURE9 g_pTexture = NULL;	// ��ͼ


KSkyDome SkyDome;
LPDIRECT3DCUBETEXTURE9 g_pTexEnvironment = NULL;	// ��������ͼ
LPDIRECT3DTEXTURE9 g_pTexFresnel = NULL;	// Fresnelϵ��ͼ



// ����������
KWaveEquationWater g_WaveEquation;
WAVEEQUATION_ATTRIBUTE g_WaveEquationData;
LPDIRECT3DTEXTURE9 g_pTexObstacle = NULL;	// �ϰ�ͼ������
LPDIRECT3DTEXTURE9 g_ppTexInject[2] = {NULL, NULL};	// ������������ˢ�ӣ�
NORMALMESH g_MeshBoat, g_MeshRock, g_MeshIsland;			// ����ʯͷ�͵���ģ��
float g_fInjectHeightBoat = 1.0f, g_fInjectHeightRock = 0.15f;	// �����ǿ�Ⱥͷ�Χ
float g_fInjectRangeBoat = 0.1f, g_fInjectRangeRock = 0.1f;
VERTEXSHADER g_VSObject, g_VSCaustics;		// ��Ⱦ�����ϵ����弰�����µ����壨��ɢ��
PIXELSHADER g_PSObject, g_PSCaustics;
BOOL g_bDisplayIsland = FALSE;			// �Ƿ���ʾ��
BOOL g_bUseNormalMap = FALSE;			// �Ƿ�ʹ��NormalMap�����������������ܣ�
UINT g_iStandardTimePerFrame = 50;		// ��׼ģ��֡����ÿ��֡��ʱ��������λ���룬���ڿ��������ٶȣ�
D3DXVECTOR4 g_ColFogUnderWater = D3DXVECTOR4(0, 0, 0, 0);		// ˮ������ɫ
D3DXVECTOR2 g_FogAdjust = D3DXVECTOR2(26.0f, 1.0f);				// ��ĳ˷�������
float g_fFogSkybox = 0.01f;


// ��������ģ�����
D3DVERTEXELEMENT9 g_WaterDeclaration[] = {
	{0,0,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0,12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0,12+12,D3DDECLTYPE_FLOAT2 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0,12+12+8,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
	{0,12+12+8+12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
	D3DDECL_END()
};
UINT g_nEachPointSize = D3DXGetDeclVertexSize(g_WaterDeclaration, 0);
DWORD *g_pAdjancy = NULL;


// �߼�����
BOOL g_bCPU = FALSE;		// VTFӲ������Ȼ����ѡ��CPU����Ⱦ���û��ֶ�ѡ�񣬿ɿ���
BOOL g_bGenerateWaterMeshMap = TRUE;	// ��GPU�����ɺ�����ص���ͼ�����ߡ�����ͼ��������PS2.a���µ��Կ��ϲ������ɣ�Ӳ��ָ�꣬��ʼ�Ͷ�����
BOOL g_bVTF = FALSE;		// �Կ��Ƿ�֧��VTF��Ӳ��ָ�꣬��ʼ�Ͷ�����
float test=1.0f;

D3DXVECTOR2 g_MeshWrapCoef(1, 1), g_HeightMapWrapCoef(1, 1);	// �û������ļ����Ƶĸ߶�ͼ����������ͷ���ͼƽ��ϵ�������и߶�ͼ������ƽ��ϵ����VTFʱ��Ч������ʱ��ǿ��(1,1)
D3DXVECTOR2 g_NormalMapWrapCoef(1, 1);	



// ��ˮ�͹�����ɫ���������ļ��ж�ȡ
D3DXVECTOR4 g_ColShallowWater = D3DXVECTOR4(0, 0, 0, 0), g_ColDeepWater = D3DXVECTOR4(0, 0, 0, 0);
D3DXVECTOR4 g_ColDiffuse = D3DXVECTOR4(1, 1, 1, 0), g_ColSpecular = D3DXVECTOR4(1, 1, 1, 0);

// ���շ���
D3DXVECTOR4 g_VecDiffuse = D3DXVECTOR4(0, 1, 0, 0), g_VecSpecular = D3DXVECTOR4(1, -0.1f, 0, 0);


//�ڴ˶�����������Ķ���ģʽ�Ͷ�������(����MODULE)
//��ʼ����
UINT InitMyScene()        //��ʼ��ÿ������Ķ��㻺�塢���������Ӧ��Xģ���ļ����������õƹ���������
{
	// ��ȷ���Կ���Shader�汾�Ա�ȷ����CPU���ɷ������߻���GPU��ֻҪС��ps2.a��ֻ����CPU�����ɷ�������
	if( !CheckPS2xSupport(32, FALSE) )		// �����GPU��������ͼ�Ļ���������Ҫ32����ʱ�Ĵ���
		g_bGenerateWaterMeshMap = FALSE;
	else
		g_bGenerateWaterMeshMap = TRUE;

	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F)))
		g_bVTF = FALSE;
	else
		g_bVTF = TRUE;


	// ���˲���
/*
	g_OceanData.iWidth = 64;
	g_OceanData.iHeight = 64;
	g_OceanData.WaterSquare = D3DXVECTOR2(150, 150);
	g_OceanData.VecWindDir = D3DXVECTOR2(1, 0);
	D3DXVec2Normalize(&g_OceanData.VecWindDir, &g_OceanData.VecWindDir);
	g_OceanData.fWaveHeightScale = 3.0f;
	g_OceanData.fWindSpeed = 1.65f;



	g_OceanDataHighFrequency.iWidth = g_OceanDataHighFrequency.iWidth;
	g_OceanDataHighFrequency.iHeight = g_OceanDataHighFrequency.iHeight;
	g_OceanDataHighFrequency.WaterSquare = D3DXVECTOR2(150, 150);
	g_OceanDataHighFrequency.VecWindDir = D3DXVECTOR2(1, 0);
	D3DXVec2Normalize(&g_OceanDataHighFrequency.VecWindDir, &g_OceanDataHighFrequency.VecWindDir);
	g_OceanDataHighFrequency.fWaveHeightScale = 2.0f;
	g_OceanDataHighFrequency.fWindSpeed = 1.45f;
*/

	// �������ļ��ж�ȡ
	char szProfileData[1000] = "";

		// ��ˮ��ɫ�����Ը���С����������
	GetPrivateProfileString("Color", "ShallowWaterColor", "0.15625,0.37890625,0.328125", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_ColShallowWater, 3);
	if(g_ColShallowWater.x > 1.0f || g_ColShallowWater.y > 1.0f || g_ColShallowWater.z > 1.0f)
		g_ColShallowWater /= 255.0f;

	GetPrivateProfileString("Color", "DeepWaterColor", "0.15625,0.37890625,0.328125", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_ColDeepWater, 3);
	if(g_ColDeepWater.x > 1.0f || g_ColDeepWater.y > 1.0f || g_ColDeepWater.z > 1.0f)
		g_ColDeepWater /= 255.0f;

		// ������ɫ
	GetPrivateProfileString("Color", "DiffuseColor", "1.0,1.0,1.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_ColDiffuse, 3);
	if(g_ColDiffuse.x > 1.0f || g_ColDiffuse.y > 1.0f || g_ColDiffuse.z > 1.0f)
		g_ColDiffuse /= 255.0f;

	GetPrivateProfileString("Color", "SpecularColor", "1.0,1.0,1.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_ColSpecular, 3);
	if(g_ColSpecular.x > 1.0f || g_ColSpecular.y > 1.0f || g_ColSpecular.z > 1.0f)
		g_ColSpecular /= 255.0f;

		// ����ɫ��΢��
	GetPrivateProfileString("Color", "FogColor", "0.0,0.5,0.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_ColFogUnderWater, 3);
	if(g_ColFogUnderWater.x > 1.0f || g_ColFogUnderWater.y > 1.0f || g_ColFogUnderWater.z > 1.0f)
		g_ColFogUnderWater /= 255.0f;
	
	GetPrivateProfileString("Color", "FogAdjust", "26.0,1.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_FogAdjust, 2);

	GetPrivateProfileString("Color", "FogSkybox", "0.01", szProfileData, 1000, CONFIG_FILENAME);
	g_fFogSkybox = (float)atof(szProfileData);


		// ���շ���
	D3DXVec4Normalize(&g_VecDiffuse, &g_VecDiffuse);
	D3DXVec4Normalize(&g_VecSpecular, &g_VecSpecular);

		// ƽ��ϵ��
	GetPrivateProfileString("Wrap", "Mesh", "1.0,1.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_MeshWrapCoef, 2);
	GetPrivateProfileString("Wrap", "HeightMap", "1.0,1.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_HeightMapWrapCoef, 2);
	GetPrivateProfileString("Wrap", "NormalMap", "1.0,1.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_NormalMapWrapCoef, 3);

	
		// Mesh Ocean Water
	g_OceanData.iWidth = GetPrivateProfileInt("Mesh", "Width", 64, CONFIG_FILENAME);
	g_OceanData.iHeight = GetPrivateProfileInt("Mesh", "Height", 64, CONFIG_FILENAME);
	g_OceanData.WaterSquare.x = (float)GetPrivateProfileInt("Mesh", "WaterSquareWidth", 150, CONFIG_FILENAME);
	g_OceanData.WaterSquare.y = (float)GetPrivateProfileInt("Mesh", "WaterSquareHeight", 150, CONFIG_FILENAME);

	GetPrivateProfileString("Mesh", "WindDir", "1.0,0.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_OceanData.VecWindDir, 2);
	D3DXVec2Normalize(&g_OceanData.VecWindDir, &g_OceanData.VecWindDir);

	GetPrivateProfileString("Mesh", "WaveHeightScale", "3.0", szProfileData, 1000, CONFIG_FILENAME);
	g_OceanData.fWaveHeightScale = (float)strtofloat(szProfileData);
	GetPrivateProfileString("Mesh", "WindSpeed", "1.65", szProfileData, 100, CONFIG_FILENAME);
	g_OceanData.fWindSpeed = (float)strtofloat(szProfileData);


		// Normalmap Ocean Water
	g_OceanDataHighFrequency.iWidth = GetPrivateProfileInt("NormalMap", "Width", 64, CONFIG_FILENAME);
	g_OceanDataHighFrequency.iHeight = GetPrivateProfileInt("NormalMap", "Height", 64, CONFIG_FILENAME);
	g_OceanDataHighFrequency.WaterSquare.x = (float)GetPrivateProfileInt("NormalMap", "WaterSquareWidth", 150, CONFIG_FILENAME);
	g_OceanDataHighFrequency.WaterSquare.y = (float)GetPrivateProfileInt("NormalMap", "WaterSquareHeight", 150, CONFIG_FILENAME);

	GetPrivateProfileString("NormalMap", "WindDir", "1.0,0.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_OceanDataHighFrequency.VecWindDir, 2);
	D3DXVec2Normalize(&g_OceanDataHighFrequency.VecWindDir, &g_OceanDataHighFrequency.VecWindDir);

	GetPrivateProfileString("NormalMap", "WaveHeightScale", "2.0", szProfileData, 1000, CONFIG_FILENAME);
	g_OceanDataHighFrequency.fWaveHeightScale = (float)strtofloat(szProfileData);
	GetPrivateProfileString("NormalMap", "WindSpeed", "1.45", szProfileData, 100, CONFIG_FILENAME);
	g_OceanDataHighFrequency.fWindSpeed = (float)strtofloat(szProfileData);

	

	// ��ʼ��������������
	g_WaveEquationData.iWidth = g_OceanData.iWidth;
	g_WaveEquationData.iHeight = g_OceanData.iHeight;
	g_WaveEquationData.WaterSquare = g_OceanData.WaterSquare;

	GetPrivateProfileString("Wave", "Damp", "0.99", szProfileData, 1000, CONFIG_FILENAME);
	g_WaveEquationData.fDampCoef = (float)atof(szProfileData);
	GetPrivateProfileString("Wave", "WaveSpeed", "0.03", szProfileData, 1000, CONFIG_FILENAME);
	g_WaveEquationData.fWaveSpeed = (float)atof(szProfileData);

	GetPrivateProfileString("Wave", "AreaDampTexture", "", g_WaveEquationData.szAreaDampTexture, 1000, CONFIG_FILENAME);

	g_bUseNormalMap = (BOOL)GetPrivateProfileInt("Wave", "UseNormalMap", 0, CONFIG_FILENAME);
	g_iStandardTimePerFrame = (UINT)(1000.0f / (float)GetPrivateProfileInt("Wave", "StandardFPS", 20, CONFIG_FILENAME));

	GetPrivateProfileString("Wave", "InjectHeightBoat", "1.0", szProfileData, 1000, CONFIG_FILENAME);
	g_fInjectHeightBoat = (float)atof(szProfileData);
	GetPrivateProfileString("Wave", "InjectHeightRock", "0.15", szProfileData, 1000, CONFIG_FILENAME);
	g_fInjectHeightRock = (float)atof(szProfileData);
	GetPrivateProfileString("Wave", "InjectRangeBoat", "0.1", szProfileData, 1000, CONFIG_FILENAME);
	g_fInjectRangeBoat = (float)atof(szProfileData);
	GetPrivateProfileString("Wave", "InjectRangeRock", "0.1", szProfileData, 1000, CONFIG_FILENAME);
	g_fInjectRangeRock = (float)atof(szProfileData);


	randomize();	// ��ʼ�����������

		// ��ʼ����������ʹ�õ������ģ��
			// ObstacleMap
	D3DXIMAGE_INFO ImageInfo;
	char szObstacleFileName[MAX_PATH] = "";
	GetPrivateProfileString("Wave", "ObstacleTexture", "", szObstacleFileName, 1000, CONFIG_FILENAME);
	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, szObstacleFileName, g_OceanData.iWidth, g_OceanData.iHeight, 0, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_POINT, D3DX_FILTER_NONE, 0, &ImageInfo, NULL, &g_pTexObstacle)))
	{
		mymessage("Texture Create Failed");
	}
	else
	{
		if(ImageInfo.Width != g_OceanData.iWidth || ImageInfo.Height != g_OceanData.iHeight)
		{
			mymessage("Obstacle Texture�ֱ��ʲ�����Ҫ��");
		}
	}


	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, "pic\\BoatInject.bmp", g_OceanData.iWidth, g_OceanData.iHeight, 0, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_NONE, 0, NULL, NULL, &g_ppTexInject[0])))
		mymessage("Texture Create Failed");
	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, "pic\\RockInject.bmp", g_OceanData.iWidth, g_OceanData.iHeight, 0, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_NONE, 0, NULL, NULL, &g_ppTexInject[1])))
		mymessage("Texture Create Failed");

	if(FAILED(g_MeshBoat.LoadFromFile("mesh\\boat.x", "mesh\\")))
		mymessage("Mesh Create Failed");
	if(FAILED(g_MeshBoat.SetUserTexture(0, "mesh\\boat.jpg")))
		mymessage("Mesh Texture Create Failed");
	if(FAILED(g_MeshBoat.SetUserTexture(1, "mesh\\boat.jpg")))
		mymessage("Mesh Texture Create Failed");

	if(FAILED(g_MeshRock.LoadFromFile("mesh\\rock.x", "mesh\\")))
		mymessage("Mesh Create Failed");
	if(FAILED(g_MeshIsland.LoadFromFile("mesh\\Island.x", "mesh\\")))
		mymessage("Mesh Create Failed");

		// ��ʼ����������ʹ�õ�����Shader
	D3DVERTEXELEMENT9 ObjectDeclaration[MAX_FVF_DECL_SIZE];
	if(FAILED(D3DXDeclaratorFromFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, ObjectDeclaration)))
		return E_FAIL;

	if(FAILED(g_VSObject.InitVertexShader("shader\\Object.vsh", ObjectDeclaration)))
		myfail(MYERR_CREATEVERTEXSHADER);
	if(FAILED(g_PSObject.InitPixelShader("shader\\Object.psh")))
		myfail(MYERR_CREATEPIXELSHADER);
	if(FAILED(g_VSCaustics.InitVertexShader("shader\\Caustics.vsh", ObjectDeclaration)))
		myfail(MYERR_CREATEVERTEXSHADER);
	if(FAILED(g_PSCaustics.InitPixelShader("shader\\Caustics.psh")))
		myfail(MYERR_CREATEPIXELSHADER);


	if(FAILED(g_WaveEquation.Init(g_WaveEquationData, FALSE, FALSE)))
	{
		mymessage("WaveEquation Water Init Failed");
		return E_FAIL;
	}



	

	// ��ʼ�����˴������棬��Ƶ����ֻ��������ͼ�ã�������������ͼ
	if(FAILED(g_FFTOceanWater.Init(g_OceanData, g_bGenerateWaterMeshMap, g_bGenerateWaterMeshMap)) || FAILED(g_FFTOceanWaterHighFrequency.Init(g_OceanDataHighFrequency, g_bGenerateWaterMeshMap, FALSE)))
	{
		mymessage("Ocean Water Init Failed");
		return E_FAIL;
	}

	if(FAILED(InitPlane(g_OceanData.iWidth*(UINT)g_MeshWrapCoef.x-1, g_OceanData.WaterSquare.x, g_OceanData.iHeight*(UINT)g_MeshWrapCoef.y-1, g_OceanData.WaterSquare.y, g_pMesh)) || FAILED(InitPlane(g_OceanData.iWidth-1, g_OceanData.WaterSquare.x, g_OceanData.iHeight-1, g_OceanData.WaterSquare.y, g_pMeshCPU)))
	{
		mymessage("Plane Init Failed");
		return E_FAIL;
	}

	// �õ�������Ϣ
	HRESULT hr = S_OK;
	SAFE_DELETE_ARRAY(g_pAdjancy);
	g_pAdjancy = new DWORD[sizeof(DWORD) * g_pMeshCPU->GetNumFaces() * 3];
	memset(g_pAdjancy, 0, sizeof(DWORD) * g_pMeshCPU->GetNumFaces() * 3);
//	hr = g_pMeshCPU->GenerateAdjacency(0.00001f, g_pAdjancy);
	if(FAILED(hr))
		mymessage("Generate Adjancy Failed!");

	// ��������
	if(FAILED(d3ddevice->CreateTexture(g_OceanData.iWidth, g_OceanData.iHeight, 0, D3DUSAGE_0, D3DFMT_A32B32G32R32F, D3DPOOL_SYSTEMMEM, &g_pTexHeightMapCPU, NULL )))
		mymessage("Texture Create Failed");
	if(FAILED(d3ddevice->CreateTexture(g_OceanData.iWidth, g_OceanData.iHeight, 0, D3DUSAGE_0, D3DFMT_A32B32G32R32F, D3DPOOL_SYSTEMMEM, &g_pTexNormalMapCPU, NULL )))
		mymessage("Texture Create Failed");
	if(FAILED(d3ddevice->CreateTexture(g_OceanData.iWidth, g_OceanData.iHeight, 0, D3DUSAGE_0, D3DFMT_A32B32G32R32F, D3DPOOL_SYSTEMMEM, &g_pTexTangentMapCPU, NULL )))
		mymessage("Texture Create Failed");

	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, "pic\\AddOnNormalMap.hdr", D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DUSAGE_0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_NONE, 0, NULL, NULL, &g_pTexAddOnNormalMap)))
		mymessage("Texture Create Failed");
//	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, "pic\\NormalMap1024.dds", D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_NONE, 0, NULL, NULL, &g_pTexture)))
//		mymessage("Texture Create Failed");
//	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, "pic\\AddOnHeightMap.bmp", D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DUSAGE_0, D3DFMT_A32B32G32R32F, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_NONE, 0, NULL, NULL, &g_pTexAddOnHeightMap)))
//		mymessage("Texture Create Failed");

	// ������պС�����ͼ��Fresnel����ͼ
//	if(FAILED(D3DXCreateCubeTextureFromFileEx(d3ddevice, "pic\\skybox.dds", D3DX_DEFAULT, D3DX_DEFAULT, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, NULL, NULL, &g_pTexEnvironment)))
//		mymessage("Texture Create Failed");
	char *szSkyBox[6] = {
		"pic\\skybox\\xpos.jpg",
			"pic\\skybox\\xneg.jpg",
			"pic\\skybox\\ypos.jpg",
			"pic\\skybox\\yneg.jpg",
			"pic\\skybox\\zpos.jpg",
			"pic\\skybox\\zneg.jpg",
	};
	if(FAILED(CreateCubeMapFromSixFiles(szSkyBox, &g_pTexEnvironment, 1024, 0, D3DFMT_A8R8G8B8)))
		mymessage("Texture Create Failed");


	V_RETURN(SkyDome.Init(0));
	V_RETURN(InitFresnelTexture(1024));


	// ʹ��CPU���ģ���SM2.0����Ⱦ
	if(FAILED(g_VSNoVTF.InitVertexShader("shader\\NoVTFWater.vsh", g_WaterDeclaration)))
		myfail(MYERR_CREATEVERTEXSHADER);
	if(FAILED(g_PSNoVTF.InitPixelShader("shader\\NoVTFWater.psh")))
		myfail(MYERR_CREATEPIXELSHADER);
	// ��֧��VTF�ģ��͵���CPU������
	if(g_bVTF)
	{
		// ֧��VTF�ľ���SM3.0
		if(FAILED(g_VSVTF.InitVertexShader("shader\\WaveEquationWater.vsh", g_WaterDeclaration)))
			myfail(MYERR_CREATEVERTEXSHADER);
		if(FAILED(g_PSVTF.InitPixelShader("shader\\WaveEquationWater.psh")))
			myfail(MYERR_CREATEPIXELSHADER);
	}
	

	//��ʼ������
	Text.InitFont2D(16,"����");
	//��ʼ��͸�ӱ任
	CameraChange.ProjTransform(1.3f);
	//����ȫ�ֵ���Ⱦģʽ������ģʽ
	d3ddevice->SetSamplerState(0,D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0,D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1,D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1,D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	d3ddevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	d3ddevice->SetRenderState( D3DRS_NORMALIZENORMALS, FALSE );
	d3ddevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );  //�˿��ز�Ҫ��Ϊȫ�����ã���ȾĳЩ����ʱ�����趨����ʼΪ��ʾ˳ʱ�������

	return S_OK;
}













// ���û����������ƶ��㣨��������Դ��
// �����������û������ٶȿ��ƣ���һ��������ʾ�Ƿ������������ã��ڶ��������Ǹ���֡���ʱ��Ա�׼�߶Ƚ������ţ�֡��Խ�͸߶Ⱦ�Ӧ��Խ�ͣ���ֹ�ۻ�ЧӦ��
void MoveBoat(BOOL bSimulation, float fHeightCoef)
{
	// �����������������Ҫ������2D���꣬��ʼλ����X�Ҷ�
	D3DXVECTOR2 PtBoatInitPos = D3DXVECTOR2(0.9f, 0.5f), PtBoatTempPos;
	D3DXVECTOR2 PtBoatPosTexture, PtBoatPosWorld;

	// ÿ֡����������ת
	D3DXMATRIX MatRotate;


	static DWORD dwStartTime = timeGetTime();
	// ��ת����10������һȦ��360�ȣ�
	float fAngle = ( timeGetTime() - dwStartTime ) / 10000.0f * 2.0f * D3DX_PI;

	D3DXMatrixRotationZ(&MatRotate, fAngle);
	// Test
	//D3DXMatrixIdentity(&MatRotate);

	// ��ƽ�Ƶ����ģ�����ת����ƽ�ƻ�ȥ
	PtBoatTempPos = PtBoatInitPos - D3DXVECTOR2(0.6f, 0.5f);
	D3DXVec2TransformCoord(&PtBoatPosTexture, &PtBoatTempPos, &MatRotate);
	PtBoatPosTexture = PtBoatPosTexture +  D3DXVECTOR2(0.5f, 0.5f);

	// ��������
	UINT iX = (UINT)(PtBoatPosTexture.x * g_WaveEquationData.iWidth);
	UINT iY = (UINT)(PtBoatPosTexture.y * g_WaveEquationData.iHeight);
	float fHeight = g_fInjectHeightBoat;		// ��׼֡���µĸ߶�
		// ���ϰ�ʱ�ص�ͻ�����˵ķ���Ч�����Ӵ󴬲��˵ĸ߶�
	if(g_bDisplayIsland)
		fHeight *= 1.5f;

	if(bSimulation && fHeightCoef > 0.0f)
	{
		fHeight *= fHeightCoef;
		if(FAILED(g_WaveEquation.SetAreaHeight(iX, iY, D3DXVECTOR2(g_fInjectRangeBoat, g_fInjectRangeBoat), g_ppTexInject[1], fHeight, FALSE)))
			mymessage("Set Height Failed");
	}




	// ��Ⱦ
	D3DXMATRIX MatWorld, MatScaling, MatTrans;
	D3DXVECTOR3 PtPos(0, 0, 0);
	// ����
	D3DXMatrixScaling(&MatScaling, 0.3f, 0.3f, 0.3f);
	// ƽ�ƣ�����ǰλ��ת��Ϊ�������꣬ƽ������Z�������
	PtPos.x = PtBoatPosTexture.x * g_WaveEquationData.WaterSquare.x;
	PtPos.z = -PtBoatPosTexture.y * g_WaveEquationData.WaterSquare.y;
	D3DXMatrixTranslation(&MatTrans, PtPos.x, PtPos.y, PtPos.z);
	// �����еĳ���
	D3DXMatrixRotationY(&MatRotate, fAngle);


	MatWorld = MatScaling * MatRotate * MatTrans;
	d3ddevice->SetTransform(D3DTS_WORLD, &MatWorld);

	D3DXVECTOR4 VecDiffuse1(-1, 1, 1, 0), VecSpecular1(-1, 0.5f, 0.3f, 0);
	D3DXVECTOR4 VecDiffuse2(1, 1, -1, 0);
	D3DXVec4Normalize(&VecDiffuse1, &VecDiffuse1);
	D3DXVec4Normalize(&VecSpecular1, &VecSpecular1);
	D3DXVec4Normalize(&VecDiffuse2, &VecDiffuse2);

	d3ddevice->SetTexture(1, g_pTexEnvironment);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	g_VSObject.SetTransform(&MatWorld);

	g_PSObject.SetPixelShader();
	g_PSObject.SetConstant(0, &VecDiffuse1, 1);
	g_PSObject.SetConstant(1, &VecSpecular1, 1);
	g_PSObject.SetConstant(2, &VecDiffuse2, 1);
//	g_PSObject.SetConstant(4, &D3DXVECTOR4(0, 40, -40, 0), 1);
//	g_PSObject.SetConstant(5, &D3DXVECTOR4(0, 30, -30, 0), 1);
	g_PSObject.SetConstant(6, &D3DXVECTOR4(1, 1, 0, 0), 1);
	g_PSObject.SetConstant(7, &D3DXVECTOR4(16, 1, 0, 0), 1);

	g_MeshBoat.DrawAll(true, g_VSObject.Declaration, g_VSObject.Handle);

	d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	g_PSObject.RestorePixelShader();
}


void MoveRock(BOOL bSimulate)
{
	static std::vector<D3DXVECTOR3> s_vecPtRocks;
	static DWORD dwLastTime = timeGetTime();
	DWORD dwCurrentTime = timeGetTime();

	// ʯͷ
	if(!effectenable)
	{
		D3DXVECTOR3 PtRock(0, 75.0f, 0);
		// ����а��س���������һ����ʯͷ�������XZλ�ƴ���ˮ��0.1��0.9��Χ�ڣ���yֵ������պж��߶�75
		while(PtRock.x < 0.1f)
			PtRock.x = randomf(0.9f, 2);
		while(PtRock.z < 0.1f)
			PtRock.z = randomf(0.9f, 2);
		s_vecPtRocks.push_back(PtRock);

		effectenable = !effectenable;
	}

	// �����б���ÿ��ʯͷ�ĵ��䣬�����ٶ�Ϊ���٣�����Ϊ1.2���ڵ���ˮ�棬��ÿ������75/1.2
	std::vector<D3DXVECTOR3>::iterator it;
	// λ�Ƹı���
	float fDeltaHeight = ( dwCurrentTime -  dwLastTime ) / 1000.0f * 75.0f / 1.2f;

	for(it = s_vecPtRocks.begin(); it != s_vecPtRocks.end(); it++)
	{
		// �������ǰ�߶�ֵ
		it->y -= fDeltaHeight;

		// ��Ⱦ����
		D3DXMATRIX MatWorld, MatScaling, MatTrans;
		D3DXVECTOR3 PtPos(0, 0, 0);
		// ����
		D3DXMatrixScaling(&MatScaling, 0.07f, 0.07f, 0.07f);
		// ƽ�ƣ�ת��Ϊ���������λ��
		PtPos.x = it->x * g_WaveEquationData.WaterSquare.x;
		PtPos.y = it->y;
		PtPos.z = -it->z * g_WaveEquationData.WaterSquare.y;
		D3DXMatrixTranslation(&MatTrans, PtPos.x, PtPos.y, PtPos.z);

		MatWorld = MatScaling * MatTrans;
		d3ddevice->SetTransform(D3DTS_WORLD, &MatWorld);

		D3DXVECTOR4 VecDiffuse1(-1, 1, 1, 0), VecSpecular1(-1, 0, 0, 0);
		D3DXVECTOR4 VecDiffuse2(1, 1, -1, 0);
		D3DXVec4Normalize(&VecDiffuse1, &VecDiffuse1);
		D3DXVec4Normalize(&VecSpecular1, &VecSpecular1);
		D3DXVec4Normalize(&VecDiffuse2, &VecDiffuse2);

		d3ddevice->SetTexture(1, g_pTexEnvironment);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);


		g_VSObject.SetTransform(&MatWorld);

		g_PSObject.SetPixelShader();
		g_PSObject.SetConstant(0, &VecDiffuse1, 1);
		g_PSObject.SetConstant(1, &VecSpecular1, 1);
		g_PSObject.SetConstant(2, &VecDiffuse2, 1);
		//	g_PSObject.SetConstant(4, &D3DXVECTOR4(0, 40, -40, 0), 1);
		//	g_PSObject.SetConstant(5, &D3DXVECTOR4(0, 30, -30, 0), 1);
		g_PSObject.SetConstant(6, &D3DXVECTOR4(1, 1, 0, 0), 1);
		g_PSObject.SetConstant(7, &D3DXVECTOR4(16, 0.3f, 0, 0), 1);

		g_MeshRock.DrawAll(true, g_VSObject.Declaration, g_VSObject.Handle);

		g_PSObject.RestorePixelShader();
		d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);


		// ����֡�����ø߶ȣ�˲ʱ���õĽ��������ø���֡���Ը߶�ֵ��������
		float fHeight = g_fInjectHeightRock;

		// ����ﵽˮ�棬�Ͳ��������������б����Ƴ���ʯͷ
		if(it->y < 0.0f)
		{
			UINT iX = (UINT)(it->x * g_WaveEquationData.iWidth);
			UINT iY = (UINT)(it->z * g_WaveEquationData.iHeight);
			if(bSimulate)
			{
				if(FAILED(g_WaveEquation.SetAreaHeight(iX, iY, D3DXVECTOR2(g_fInjectRangeRock, g_fInjectRangeRock), g_ppTexInject[1], fHeight, TRUE)))
					mymessage("Set Height Failed");
			}

			// �Ƴ�
			s_vecPtRocks.erase(it);

			// �����վ��˳������������
			if(s_vecPtRocks.empty())
				break;

		}
	}


	dwLastTime = dwCurrentTime;

}





void RenderIsland()
{
	D3DXMATRIX MatWorld, MatScaling, MatTrans;
//	D3DXVECTOR3 PtPos(80, 6, -80);
//	D3DXVECTOR3 PtPos(80, 8, -80);
	D3DXVECTOR3 PtPos(80, 0, -80);
	// ����
//	D3DXMatrixScaling(&MatScaling, 0.0015f, 0.0015f, 0.0015f);
//	D3DXMatrixScaling(&MatScaling, 0.15f, 0.15f, 0.15f);
	float fScale = 20.0f;
	D3DXMatrixScaling(&MatScaling, 0.5f, 0.5f, 0.5f);
	// ƽ�ƣ�ת��Ϊ���������λ��
	D3DXMatrixTranslation(&MatTrans, PtPos.x, PtPos.y, PtPos.z);

	MatWorld = MatScaling * MatTrans;
	d3ddevice->SetTransform(D3DTS_WORLD, &MatWorld);

	D3DXVECTOR4 VecDiffuse1(-1, 1, 1, 0), VecSpecular1(-1, 0.5f, 0.3f, 0);
	D3DXVECTOR4 VecDiffuse2(1, 1, -1, 0);
	D3DXVec4Normalize(&VecDiffuse1, &VecDiffuse1);
	D3DXVec4Normalize(&VecSpecular1, &VecSpecular1);
	D3DXVec4Normalize(&VecDiffuse2, &VecDiffuse2);

	d3ddevice->SetTexture(1, g_pTexEnvironment);
	d3ddevice->SetTexture(2, g_FFTOceanWater.GetNormalMap());
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	d3ddevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	g_VSCaustics.SetTransform(&MatWorld);

	g_PSCaustics.SetPixelShader();
	g_PSCaustics.SetConstant(0, &VecDiffuse1, 1);
	g_PSCaustics.SetConstant(1, &VecSpecular1, 1);
	g_PSCaustics.SetConstant(2, &VecDiffuse2, 1);

	// Fog
	g_PSCaustics.SetConstant(3, &D3DXVECTOR4(g_ColFogUnderWater.x, g_ColFogUnderWater.y, g_ColFogUnderWater.z, 1.0f), 1);
	g_PSCaustics.SetConstant(4, &D3DXVECTOR4(g_FogAdjust.x, g_FogAdjust.y, 1.0f, 1.0f), 1);

	g_PSCaustics.SetConstant(6, &D3DXVECTOR4(15, 15, 0, 0), 1);
	g_PSCaustics.SetConstant(7, &D3DXVECTOR4(1.0f, 0.0f, 0, 0), 1);	// Island�ص��߹�

	// For Bilinear Filtering
	g_PSCaustics.SetConstant(8, &D3DXVECTOR4((float)g_OceanData.iWidth, (float)g_OceanData.iHeight, 0, 0), 1);
	g_PSCaustics.SetConstant(9, &D3DXVECTOR4(1.0f / (float)g_OceanData.iWidth, 1.0f / (float)g_OceanData.iHeight, 0, 0), 1);

	// ˮ�²��н�ɢ���������������
	BOOL bUnderWater = FALSE;
	if(CameraChange.Eye.y < 2.0f)		// ��Ϊ�в��ˣ�����ˮƽ����΢���һЩ
		bUnderWater = TRUE;
	g_PSCaustics.SetConstantB(0, &bUnderWater, 1);


	// �����������ͶӰ��ʽ
	D3DXMATRIX MatView, MatProj;
	D3DXVECTOR3 PtCameraPos(0, 30.0f, 0);
	D3DXVECTOR3 PtLookAtPos(0, 0.0f, 0);
	D3DXVECTOR3 VecHead(0, 0, 1);
	D3DXVECTOR2 VecProjRange = g_WaveEquationData.WaterSquare * 2.0f;


	D3DXMatrixOrthoLH(&MatProj, VecProjRange.x, VecProjRange.y, 1.0f, 150.0f);
	D3DXMatrixLookAtLH(&MatView, &PtCameraPos, &PtLookAtPos, &VecHead);

	D3DXMatrixTranspose(&MatWorld, &(MatWorld * MatView * MatProj));
	g_VSCaustics.SetConstant(10, &MatWorld, 4);


	g_MeshIsland.DrawAll(true, g_VSCaustics.Declaration, g_VSCaustics.Handle);

	g_PSCaustics.RestorePixelShader();
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
}




HRESULT RenderIslandToObstacleMap()
{
	LPDIRECT3DSURFACE9 pRT = NULL, pDepthBuffer = NULL;
	V_RETURN(d3ddevice->CreateRenderTarget(g_WaveEquationData.iWidth, g_WaveEquationData.iWidth, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &pRT, NULL));
	V_RETURN(d3ddevice->CreateDepthStencilSurface(g_WaveEquationData.iWidth, g_WaveEquationData.iWidth, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, FALSE, &pDepthBuffer, NULL));

	// �ȱ��浱ǰ��RT����Ȼ��壬�����µ���Ȼ����Ա�FFT��Ⱦ
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(pDepthBuffer));


	// ����RT��Clear
	V_RETURN(d3ddevice->SetRenderTarget(0, pRT));
	V_RETURN(d3ddevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0));

	// ������ɫ��ϲ���
	SetTextureColorMix(0, D3DTOP_SELECTARG1, D3DTA_TFACTOR, D3DTA_TEXTURE);
	SetTextureColorMix(1, D3DTOP_DISABLE, D3DTA_TFACTOR, D3DTA_TEXTURE);
	d3ddevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0);

	// �����������ͶӰ��ʽ
	D3DXMATRIX MatView, MatProj;
	D3DXVECTOR3 PtCameraPos(g_WaveEquationData.WaterSquare.x / 2.0f, 30.0f, g_WaveEquationData.WaterSquare.y / -2.0f);
	D3DXVECTOR3 PtLookAtPos(g_WaveEquationData.WaterSquare.x / 2.0f, 0.0f, g_WaveEquationData.WaterSquare.y / -2.0f);
	D3DXVECTOR3 VecHead(0, 0, 1);
	D3DXVECTOR2 VecProjRange = g_WaveEquationData.WaterSquare;
	D3DXMatrixOrthoLH(&MatProj, VecProjRange.x, VecProjRange.y, 1.0f, 150.0f);
	D3DXMatrixLookAtLH(&MatView, &PtCameraPos, &PtLookAtPos, &VecHead);
	d3ddevice->SetTransform(D3DTS_VIEW, &MatView);
	d3ddevice->SetTransform(D3DTS_PROJECTION, &MatProj);


	// �����������
	D3DXMATRIX MatWorld, MatScaling, MatTrans;
	//	D3DXVECTOR3 PtPos(80, 6, -80);
	//	D3DXVECTOR3 PtPos(80, 8, -80);
	D3DXVECTOR3 PtPos(80, 0, -80);
	// ����
	//	D3DXMatrixScaling(&MatScaling, 0.0015f, 0.0015f, 0.0015f);
	//	D3DXMatrixScaling(&MatScaling, 0.15f, 0.15f, 0.15f);
	D3DXMatrixScaling(&MatScaling, 0.5f, 0.5f, 0.5f);
	// ƽ�ƣ�ת��Ϊ���������λ��
	D3DXMatrixTranslation(&MatTrans, PtPos.x, PtPos.y, PtPos.z);

	MatWorld = MatScaling * MatTrans;
	d3ddevice->SetTransform(D3DTS_WORLD, &MatWorld);

	g_VSObject.SetTransform(&MatWorld);
	d3ddevice->SetPixelShader(NULL);
	g_MeshIsland.DrawAll(true, g_VSObject.Declaration, NULL);


	// ����ˮ�棨Ϊ�˱�֤�ڵ���ϵ��
	d3ddevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
	D3DXMatrixIdentity(&MatWorld);
	d3ddevice->SetTransform(D3DTS_WORLD, &MatWorld);

	LPDIRECT3DVERTEXBUFFER9 pVB = NULL;
	LPDIRECT3DINDEXBUFFER9 pIB = NULL;
	g_pMesh->GetIndexBuffer(&pIB);
	g_pMesh->GetVertexBuffer(&pVB);
	d3ddevice->SetStreamSource(0, pVB, 0, g_nEachPointSize);
	d3ddevice->SetIndices(pIB);
	d3ddevice->BeginScene();
	d3ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, g_pMesh->GetNumVertices(), 0, g_pMesh->GetNumFaces());
	d3ddevice->EndScene();


	// �ָ�RT����Ȼ��弰�趨
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);

	// ���棬����
	V_RETURN(D3DXSaveSurfaceToFile("obstacle.tga", D3DXIFF_TGA, pRT, NULL, NULL));
	SAFE_RELEASE(pVB);
	SAFE_RELEASE(pIB);
	SAFE_RELEASE(pRT);
	SAFE_RELEASE(pDepthBuffer);
	return S_OK;
}




void SetMyScene(UINT No)      //����ÿ��������������ʡ�����ģʽ����Ⱦģʽ������ģʽ�ȣ���������任
{
	D3DXMATRIX matworld,temp;
	D3DXMatrixIdentity(&matworld);
	static DWORD dwFirstTime = timeGetTime();
	DWORD dwTime = timeGetTime();
	float fScaler = 1.0f;

	static UINT s_iEffectNo = g_iCurrentEffectNo;
	float dwOceanDeltaTime = (float)(dwTime - dwFirstTime) / 3500.0f;

	VERTEXSHADER *pVS = NULL;
	PIXELSHADER *pPS = NULL;
	LPDIRECT3DVERTEXBUFFER9 pVB = NULL;
	LPDIRECT3DINDEXBUFFER9 pIB = NULL;

	HRESULT hr = S_OK;
	g_pMesh->GetIndexBuffer(&pIB);

	BOOL bWireFrame = FALSE;


	// ģ����ƣ�Ĭ�ϱ�׼20֡��50ms�����������Boat��Simulationһ�µķ�ʽ
	BOOL bWaveSimulateThisFrame = FALSE, bMoveBoatThisFrame = FALSE;
	DWORD INTERVAL_WAVE = g_iStandardTimePerFrame, INTERVAL_BOAT = g_iStandardTimePerFrame;

	static DWORD s_dwLastFrameTime = timeGetTime();
	DWORD dwCurrentFrameTime = timeGetTime();
	DWORD dwDeltaFrameTime = dwCurrentFrameTime - s_dwLastFrameTime;
	static DWORD s_dwWaveAccumulateTime = 0, s_dwBoatAccumulateTime = 0;

	float fHeightCoef = 1.0f, fWaveSpeedCoef = 1.0f;
	

	// �������ǽ��в���ģ��
	if(s_dwWaveAccumulateTime > INTERVAL_WAVE)
	{
		bWaveSimulateThisFrame = TRUE;
		// ���࣬�������ܼ�Ϊֹ
		while(s_dwWaveAccumulateTime > INTERVAL_WAVE)
			s_dwWaveAccumulateTime -= INTERVAL_WAVE;
		// �ۼ�
		s_dwWaveAccumulateTime += dwDeltaFrameTime;
	}
	else
	{
		bWaveSimulateThisFrame = FALSE;
		s_dwWaveAccumulateTime += dwDeltaFrameTime;
	}

	// �������ǽ��д�����ģ��
	if(s_dwBoatAccumulateTime > INTERVAL_BOAT)
	{
		bMoveBoatThisFrame = TRUE;
		// ���࣬�������ܼ�Ϊֹ
		while(s_dwBoatAccumulateTime > INTERVAL_BOAT)
			s_dwBoatAccumulateTime -= INTERVAL_BOAT;
		// �ۼ�
		s_dwBoatAccumulateTime += dwDeltaFrameTime;
	}
	else
	{
		bMoveBoatThisFrame = FALSE;
		s_dwBoatAccumulateTime += dwDeltaFrameTime;
	}

	s_dwLastFrameTime = dwCurrentFrameTime;


	// ��֡������£��ʵ��������ø߶ȣ���߲���
	if(dwDeltaFrameTime > INTERVAL_WAVE && bWaveSimulateThisFrame)
	{
		fWaveSpeedCoef = (float)dwDeltaFrameTime / (float)INTERVAL_WAVE;
	}
	if(dwDeltaFrameTime > INTERVAL_BOAT && bMoveBoatThisFrame)
	{
		fHeightCoef = (float)INTERVAL_BOAT / (float)dwDeltaFrameTime;
		fHeightCoef *= fHeightCoef;
	}
	


	// ����������ObstacleMap
//	RenderIslandToObstacleMap();
//	PostQuitMessage(0);
//	return;

	switch(No)
	{
		case 0:
			// ��Ⱦ��պ���
			D3DXMatrixScaling(&temp, 75, 75, 75);
			D3DXMatrixTranslation(&matworld, 75, 0, -75);
			matworld = temp * matworld;
			d3ddevice->SetTransform(D3DTS_WORLD, &matworld);
				// ˮ�£�������
			DWORD dwColor;
			dwColor = D3DCOLOR_ARGB(255, (UINT)(g_ColFogUnderWater.x*255.0f), (UINT)(g_ColFogUnderWater.y*255.0f), (UINT)(g_ColFogUnderWater.z*255.0f));
			if(CameraChange.Eye.y < 0.0f)
			{
				d3ddevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
				d3ddevice->SetRenderState(D3DRS_FOGCOLOR, dwColor);
				d3ddevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP);
				d3ddevice->SetRenderState(D3DRS_FOGDENSITY, ftodw(g_fFogSkybox));
			}
			SkyDome.Draw(g_pTexEnvironment);
			d3ddevice->SetRenderState(D3DRS_FOGENABLE, FALSE);



			// �����û�����
			bWireFrame = FALSE;
			if(g_iCurrentEffectNo == 0)
			{
				bWireFrame = FALSE;
				g_bDisplayIsland = FALSE;
			}

			if(g_iCurrentEffectNo == 1)
				bWireFrame = TRUE;

			if(g_iCurrentEffectNo == 2)
			{
				bWireFrame = FALSE;
				g_bDisplayIsland = TRUE;
			}


			// �ƶ����壬����������ģ�⣬����������֡�����Ͳ������û�����
			if(curfps < WATER_MIN_FPS)
			{
				MoveRock(FALSE);
				MoveBoat(FALSE, 0.0f);
				g_WaveEquation.ResetWave();
			}
			else
			{
				MoveRock(TRUE);
				MoveBoat(bMoveBoatThisFrame, fHeightCoef);
			}


			// ����֡�����ò��٣�֡��Խ�Ͳ��پ�Ӧ��Խ�죬����С�����ﲨ�ٻ����������ֵ���ȶ���
			float fSpeed;
			fSpeed = g_WaveEquationData.fWaveSpeed * fWaveSpeedCoef;
//			g_WaveEquation.SetWaveSpeed(fSpeed);
			if(bWaveSimulateThisFrame)
			{
				if(FAILED(g_WaveEquation.WaterSimulation(1.0f)))
					mymessage("Water Simulation Failed");
			}


			// ���˷���
			if(FAILED(g_FFTOceanWater.WaterSimulation(dwOceanDeltaTime, g_WaveEquation.GetHeightMap())))
				mymessage("Ocean Simulation Failed");

			if(g_bUseNormalMap)
			{
				if(FAILED(g_FFTOceanWaterHighFrequency.WaterSimulation(dwOceanDeltaTime * 2.0f)))
					mymessage("Ocean Simulation Failed");
			}


			// ��Ⱦ���������赲ģʽ
			if(g_bDisplayIsland)
			{
				RenderIsland();
				if(FAILED(g_WaveEquation.SetObstacleTexture(g_pTexObstacle, NULL)))
					mymessage("Set Obstacle Failed");
			}
			else
			{
				if(FAILED(g_WaveEquation.SetObstacleTexture(NULL, NULL)))
					mymessage("Set Obstacle Failed");
			}



			// Force CPU do VTF
			if(!effectenable)
				g_bCPU = TRUE;
			else
				g_bCPU = FALSE;


			// ��֧�ֵ�ֻ����CPU��Lock�ˣ�������Ӧ��VB
			if(!g_bVTF)
			{
				if(FAILED(ChangeWaterMesh()))
					mymessage("Change Mesh Failed");

				// ��GPU���ɷ���ͼ�����У��Ǿ�ֻ���ó�ʼ���ɵľ�̬����ͼ��
				if(!g_bGenerateWaterMeshMap)
				{
					if(FAILED(hr = D3DXComputeNormals(g_pMeshCPU, g_pAdjancy)))
						mymessage("Compute Normal Failed");
					if(FAILED(hr = D3DXComputeTangent(g_pMeshCPU, 0, 0, 0, 0, g_pAdjancy)))
//					if(FAILED(hr = MyComputeTangent()))
						mymessage("Compute Tangent Failed");
				}

				g_pMeshCPU->GetVertexBuffer(&pVB);
				pVS = &g_VSNoVTF;
				pPS = &g_PSNoVTF;
			}
			// ֧�ֵ�VTF����VTF��ͼ�Ͷ�Ӧ��VB
			else
			{
				// VTFӲ������Ȼ������CPU����Ⱦ
				if(g_bCPU)
				{
					if(FAILED(ChangeWaterMesh()))
						mymessage("Change Mesh Failed");

					g_pMeshCPU->GetVertexBuffer(&pVB);
					pVS = &g_VSNoVTF;
					pPS = &g_PSNoVTF;
				}
				else
				{
					// VTF Height Map
					d3ddevice->SetTexture(D3DVERTEXTEXTURESAMPLER0, g_FFTOceanWater.GetHeightMap());
					d3ddevice->SetTexture(D3DVERTEXTEXTURESAMPLER1, g_FFTOceanWater.GetNormalMap());
					d3ddevice->SetTexture(D3DVERTEXTEXTURESAMPLER2, g_FFTOceanWater.GetTangentMap());

					d3ddevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
					d3ddevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
					d3ddevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
					d3ddevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
					d3ddevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
					d3ddevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
					d3ddevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER2, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
					d3ddevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER2, D3DSAMP_MINFILTER, D3DTEXF_POINT);
					d3ddevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER2, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

					g_pMesh->GetVertexBuffer(&pVB);
					pVS = &g_VSVTF;
					pPS = &g_PSVTF;
				}
			}

			// ��ͼ
			d3ddevice->SetTexture(0, g_pTexture);
			d3ddevice->SetTexture(1, g_pTexEnvironment);
			d3ddevice->SetTexture(2, g_pTexFresnel);
			// �������ɷ���ͼ������FFT�ĸ�Ƶ���˷���ͼ��������þ�̬�ĸ��ӷ���ͼ
			if(g_bGenerateWaterMeshMap)
				d3ddevice->SetTexture(3, g_FFTOceanWaterHighFrequency.GetNormalMap());
			else
				d3ddevice->SetTexture(3, g_pTexAddOnNormalMap);

			d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			d3ddevice->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			d3ddevice->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			d3ddevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

			d3ddevice->SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			d3ddevice->SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			d3ddevice->SetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddevice->SetSamplerState(3, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

			// ����
			D3DXMatrixIdentity(&matworld);
			pVS->SetTransform(&matworld);

			// Wave Scalar
			fScaler = g_OceanData.fWaveHeightScale;
			pVS->SetConstant(1, (float *)&D3DXVECTOR4(fScaler, fScaler, fScaler, fScaler), 1);
			// NormalMap Dimension
			pVS->SetConstant(2, (float *)&D3DXVECTOR4((float)g_OceanDataHighFrequency.iWidth, (float)g_OceanDataHighFrequency.iHeight, 0, 1), 1);
			// Texture Wrap Coef
			pVS->SetConstant(3, (float *)&D3DXVECTOR4(g_HeightMapWrapCoef.x, g_HeightMapWrapCoef.y, 0, 1), 1);

			// ��ˮ��ɫ��0��1�ֱ��ʾǳˮ����ˮ��ɫ
			//pPS->SetConstant(0, (float *)&D3DXVECTOR4(40.0f/256.0f, 97.0/256.0f, 84.0f/256.0f, 1), 1);		// From PDF
			//pPS->SetConstant(1, (float *)&D3DXVECTOR4(38.0f/256.0f, 79.0/256.0f, 99.0f/256.0f, 1), 1);	// From szlongman
			pPS->SetConstant(0, &g_ColShallowWater, 1);		// From Config
			pPS->SetConstant(1, &g_ColDeepWater, 1);

			// Diffuse��Specular������ɫ
			pPS->SetConstant(2, &g_ColDiffuse, 1);
			pPS->SetConstant(3, &g_ColSpecular, 1);

			// Diffuse��Specular������ɫ����Y�ᣩ
			pPS->SetConstant(4, &g_VecDiffuse, 1);
			pPS->SetConstant(5, &g_VecSpecular, 1);

			// ����ͼ�����������ű�����������չ����ͼ��ͨ����������������ƽ�����Ǻ�������ٸ�����ͼ
			pPS->SetConstant(6, (float *)&D3DXVECTOR4(g_NormalMapWrapCoef.x, g_NormalMapWrapCoef.y, 0, 1), 1);
			// 1/Width, 1/Height, for normalmap bilinear filtering
			pPS->SetConstant(7, (float *)&D3DXVECTOR4(1.0f / (float)g_OceanDataHighFrequency.iWidth, 1.0f / (float)g_OceanDataHighFrequency.iHeight, 0, 1), 1);

			// ����CP������
			pPS->SetConstant(8, (float *)&D3DXVECTOR4((float)g_OceanData.WaterSquare.x / 2.0f, 0.0f, (float)g_OceanData.WaterSquare.y / -2.0f, 1), 1);
			float fRadius;
			fRadius = sqrtf(g_OceanData.WaterSquare.x * g_OceanData.WaterSquare.x + g_OceanData.WaterSquare.y * g_OceanData.WaterSquare.y);
			pPS->SetConstant(9, (float *)&D3DXVECTOR4(1.0f / fRadius, 1.0f / fRadius, 1.0f / fRadius, 1.0f / fRadius), 1);

			// �����ʣ�ע���������ˮ�Ϻ�ˮ���ǲ�ͬ��
			float fRefractIndex;
			if(CameraChange.Eye.y < 0.0f)
				fRefractIndex = 1.0f / 1.33f;
			else
				fRefractIndex = 1.33f;
			pPS->SetConstant(20, (float *)&D3DXVECTOR4(fRefractIndex, fRefractIndex, fRefractIndex, fRefractIndex), 1);


			// �Ƿ�ʹ��NormalMap
			pPS->SetConstantB(0, &g_bUseNormalMap, 1);

			// Fog���������ˮ�£�ˮ��Ų�����
			pPS->SetConstant(21, &D3DXVECTOR4(g_ColFogUnderWater.x, g_ColFogUnderWater.y, g_ColFogUnderWater.z, 1.0f), 1);
			pPS->SetConstant(22, &D3DXVECTOR4(g_FogAdjust.x, g_FogAdjust.y, 1.0f, 1.0f), 1);

			BOOL bUnderWater;
			bUnderWater = FALSE;
			if(CameraChange.Eye.y < 0.0f)
				bUnderWater = TRUE;
			pPS->SetConstantB(1, &bUnderWater, 1);



			// Alpha Blend
			d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			d3ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			d3ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			
			// ����
			pPS->SetPixelShader();
			d3ddevice->SetStreamSource(0, pVB, 0, g_nEachPointSize);
			d3ddevice->SetIndices(pIB);

			if(bWireFrame)
				d3ddevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			pVS->DrawIndexPrimitive(0, g_pMesh->GetNumFaces(), g_pMesh->GetNumVertices());

			// �ָ�
			pPS->RestorePixelShader();
			d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			d3ddevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
			d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
			break;
		case 1:
			break;
	}

	SAFE_RELEASE(pVB);
	SAFE_RELEASE(pIB);
}

void RenderMyScene(UINT No)       //��Ⱦÿ�����壬������Ⱦ�ĸ����ݲ�������
{
	switch(No)
	{
		case 0:
			char a[500];
			sprintf(a, "FPS: %.0f", curfps);
			Text.DrawText2D(a, 0, 100, 150, RED);
			
			sprintf(a, "���ڿ��ٸ���Ҷ�任��FFT����Gestner���˵�Wind-Driven��ˮģ�⣬ʹ��%d*%d�ĺ�������", g_OceanData.iWidth, g_OceanData.iHeight, g_OceanDataHighFrequency.iWidth, g_OceanDataHighFrequency.iHeight);
			Text.DrawText2D(a, 0, 30, 0, YELLOW);

			Text.DrawText2D("ˮ������������������ö�ά����������ģ�⣬������ˮ�µĿ�ʴ����ɢ���ü򻯵�������߸���������", 0, 30, 30, YELLOW);
			Text.DrawText2D("���̿��ƣ����س����������һ��ʯͷ���۲�����ˮ���ϵ�Ч����ֻ�ǲ�Ҫ̫BTŶ-_-!!��", 0, 100, 100, BLACK);
			Text.DrawText2D("          ��+/-�����л��߿�ģʽ����ˮ���ϵĺ������۲첨���ڵ����ϱ��谭��Ч��", 0, 100, 120, BLACK);

			if(curfps < WATER_MIN_FPS)
				Text.DrawText2D("ע�⣺��ǰ֡��̫�ͣ��޷���ʾ��������", 0, 0, 70, RED);

			/*
			if(!g_bVTF)
			{
				Text.DrawText2D("�����Կ���֧��VTF��ֻ����CPU����Ⱦ���ˣ�֡����ǳ���", 0, 0, 30, RED);

				if(!g_bGenerateWaterMeshMap)
				{
					Text.DrawText2D("����ATI�Կ�Shader�汾�ϵͣ�����ֻ����CPU�����㷨�ߺ����ߣ�֡����ѩ�ϼ�˪����", 0, 0, 60, RED);
					Text.DrawText2D("��ǰ�Ĳ���Ⱦ�������PhillipsƵ�����ɣ�GPU��������Ҷ���任��GPU���������������ɣ�CPU�������˶�����Ⱦ��CPU��", 0, 0, 100, BLUE);
				}
				else
					Text.DrawText2D("��ǰ�Ĳ���Ⱦ�������PhillipsƵ�����ɣ�GPU��������Ҷ���任��GPU���������������ɣ�GPU�������˶�����Ⱦ��CPU��", 0, 0, 100, BLUE);
			}
			else
			{
				if(g_bCPU)
				{
					Text.DrawText2D("����ǿ����CPU����Ⱦ���ˣ�֡���ἱ���½�", 0, 0, 30, RED);
					Text.DrawText2D("��ǰ�Ĳ���Ⱦ�������PhillipsƵ�����ɣ�GPU��������Ҷ���任��GPU���������������ɣ�GPU�������˶�����Ⱦ��CPU��", 0, 0, 100, BLUE);
				}
				else
				{
					Text.DrawText2D("��������GPU֧�ֵ�VTF��ȫ����Ⱦ���ˣ�֡�����˺ܴ����ߣ����԰��س��л���CPU���������", 0, 0, 30, RED);
					Text.DrawText2D("��ǰ�Ĳ���Ⱦ�������PhillipsƵ�����ɣ�GPU��������Ҷ���任��GPU���������������ɣ�GPU�������˶�����Ⱦ��GPU��", 0, 0, 100, BLUE);
				}
			}
			*/
			break;
		case 1:
			// ���������ɸ��ӷ���ͼ
//			D3DXSaveTextureToFile("c:\\normal.hdr", D3DXIFF_HDR, g_FFTOceanWaterHighFrequency.GetNormalMap(), NULL);
			break;
	}
}



UINT ResetDevice();
UINT DisplayMyScene()  //ѭ����ÿ�����嶼��ʾ�������˺���ÿ������һ�α�ʾһ֡
{
	LARGE_INTEGER start, end;
	static DWORD LastUpdateFPSCount = GetTickCount(), CurUpdateFPSCount;
	UCHAR UpdateFPS = 0;
	static DWORD CurfpsNum = 0;
	CurfpsNum++;
	
	
	//�����豸��ʧ
	HRESULT hr = d3ddevice->TestCooperativeLevel();
	UINT InfoID = 0;
	
	if(FAILED(hr))
	{
		if(hr == D3DERR_DEVICENOTRESET)
			InfoID = ResetDevice();
		return InfoID;
	}
	
	//������ʾģʽ�л�
	if(ChangeDisplayModeSign)
	{
		ChangeDisplayModeSign = FALSE;
		InfoID = ResetDevice();
		return InfoID;
	}
	
	//FPS���¿���
	CurUpdateFPSCount = GetTickCount();
	
	if(CurUpdateFPSCount - LastUpdateFPSCount > FPSUPDATETIME || curfps == 0)
	{
		UpdateFPS = 1;
		LastUpdateFPSCount = GetTickCount();
	}
	
	//��ʼ��ʱ
	QueryPerformanceCounter(&start);
	
	d3ddevice->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(128, 0, 0),1.0f,0);
	
	for(UINT i=0;i<1;i++)
	{
		//���������ʱ����Ҫ��Ⱦ�����壬�ڴ˴�����
		//if(i!=2) continue;
		SetMyScene(i);
		RenderMyScene(i);
	}
    
	//�������ͷ�任
	CameraChange.ViewTransform();
	
	QueryPerformanceCounter(&end);
	
	d3ddevice->Present(NULL,NULL,NULL,NULL);
	
	//���㵱ǰ֡��
#ifdef USE_AVERAGEFPS
	if(UpdateFPS) 
	{
		curfps = (float)CurfpsNum  * (float)1000 / (float)FPSUPDATETIME;
		CurfpsNum = 0;
	}
#endif USE_AVERAGEFPS
#ifndef USE_AVERAGEFPS
	__int64 TimeDelay = end.QuadPart - start.QuadPart;
	if(UpdateFPS) curfps = (float)PerformanceFrequency.QuadPart / (float)TimeDelay;
#endif USE_AVERAGEFPS
	
return InfoID;
}


void ReleaseMyScene()
{
	//�������ͷ�����POOL_DEFAULT����Դ��������������е�ģ��
	g_VSVTF.Release();
	g_PSVTF.Release();
	g_VSNoVTF.Release();
	g_PSNoVTF.Release();

	Text.Release();
	g_FFTOceanWater.Release();
	g_FFTOceanWaterHighFrequency.Release();

	g_WaveEquation.Release();
	g_MeshRock.Release();
	g_MeshBoat.Release();
	g_MeshIsland.Release();
	SAFE_RELEASE(g_pTexObstacle);
	SAFE_RELEASE(g_ppTexInject[0]);
	SAFE_RELEASE(g_ppTexInject[1]);
	g_VSObject.Release();
	g_PSObject.Release();
	g_VSCaustics.Release();
	g_PSCaustics.Release();


	SkyDome.Release();
	SAFE_RELEASE(g_pMesh);
	SAFE_RELEASE(g_pMeshCPU);

	SAFE_RELEASE(g_pTexHeightMapCPU);
	SAFE_RELEASE(g_pTexNormalMapCPU);
	SAFE_RELEASE(g_pTexTangentMapCPU);

	SAFE_RELEASE(g_pTexture);
	SAFE_RELEASE(g_pTexAddOnNormalMap);
	SAFE_RELEASE(g_pTexAddOnHeightMap);
	SAFE_RELEASE(g_pTexEnvironment);
	SAFE_RELEASE(g_pTexFresnel);
}

//�ڳ�������ʱ������һ��InitMyscene()��Ȼ����WINDOWS��ѭ��







HRESULT InitFresnelTexture(UINT iWidth)
{
	SAFE_RELEASE(g_pTexFresnel);
	if(FAILED(d3ddevice->CreateTexture(iWidth, 1, 0, D3DUSAGE_0, D3DFMT_R32F, D3DPOOL_MANAGED, &g_pTexFresnel, NULL )))
		mymessage("Texture Create Failed");

	// Lock VB & Tex
	float *pData = NULL, fCosTerm = 0.0f;
	D3DLOCKED_RECT Rect;
	V_RETURN(g_pTexFresnel->LockRect(0, &Rect, NULL, 0));

	pData = (float *)Rect.pBits;
	// �������
	for(UINT iX = 0; iX < iWidth; iX++)
	{		
		fCosTerm = (float)iX / (float)iWidth;
		*pData++ = GetFresnelCoef(1.0f, 1.333f, fCosTerm);
	}

	// ����
	g_pTexFresnel->UnlockRect(0);
	return S_OK;
}










HRESULT ChangeWaterMesh()
{
	// ��Load FFT�õ���ͼ���ڴ�
	V_RETURN(D3DXLoadTextureFromTexture(g_pTexHeightMapCPU, g_FFTOceanWater.GetHeightMap()));
	if(g_bGenerateWaterMeshMap)
	{
		V_RETURN(D3DXLoadTextureFromTexture(g_pTexNormalMapCPU, g_FFTOceanWater.GetNormalMap()));
		V_RETURN(D3DXLoadTextureFromTexture(g_pTexTangentMapCPU, g_FFTOceanWater.GetTangentMap()));
	}

	UINT nXPointNum = g_OceanData.iWidth, nZPointNum = g_OceanData.iHeight;

	// ��������
	UINT nPointNum = nXPointNum * nZPointNum;

	// Lock VB & Tex
	float *pVertexSrc = NULL, *pVertexDst = NULL;
	float *pTexData = NULL, *pTexNormal = NULL, *pTexTangent = NULL;
	D3DLOCKED_RECT Rect, RectNormal, RectTangent;
	D3DXVECTOR4 HeightData, NormalData, TangentData;	// in Tex
	float fDeltaHeightX = 0.0f, fDeltaHeightY = 0.0f;	// XY������
	D3DXVECTOR3 VertexPosition;	// in Source VB

	V_RETURN(g_pMesh->LockVertexBuffer(0, (void **)&pVertexSrc));
	V_RETURN(g_pMeshCPU->LockVertexBuffer(0, (void **)&pVertexDst));
	V_RETURN(g_pTexHeightMapCPU->LockRect(0, &Rect, NULL, 0));
	if(g_bGenerateWaterMeshMap)
	{
		V_RETURN(g_pTexNormalMapCPU->LockRect(0, &RectNormal, NULL, 0));
		V_RETURN(g_pTexTangentMapCPU->LockRect(0, &RectTangent, NULL, 0));
	}


	// �������
	UINT i = 0, j = 0;
	for(j = 0; j < nZPointNum; j++)
	{
		pTexData = (float *)((BYTE *)Rect.pBits + j * Rect.Pitch);
		if(g_bGenerateWaterMeshMap)
		{
			pTexNormal = (float *)((BYTE *)RectNormal.pBits + j * RectNormal.Pitch);
			pTexTangent = (float *)((BYTE *)RectTangent.pBits + j * RectTangent.Pitch);
		}

		for(i = 0; i < nXPointNum; i++)
		{
			// ����߶�ͼ���ݣ������ݷ�����������
			HeightData = *(LPD3DXVECTOR4)pTexData;
			pTexData += 4;
			HeightData.x *= (HeightData.z - 1);
			HeightData.y *= (HeightData.w - 1);

			// �˺�������ֵ
			HeightData *= g_OceanData.fWaveHeightScale;

			// �õ�XY���������ݣ�Gestner Wave��
			fDeltaHeightY = HeightData.x;
			fDeltaHeightX = HeightData.y;
			// Test
			//			fDeltaHeightX = 0;
			//			fDeltaHeightY = 0;

			// ���뷨�ߺ����ߣ�����0��1��ԭ��-1��1
			if(g_bGenerateWaterMeshMap)
			{
				NormalData = *(LPD3DXVECTOR4)pTexNormal;
				pTexNormal += 4;
				NormalData.x -= 0.5f;	NormalData.y -= 0.5f;	NormalData.z -= 0.5f;
				NormalData *= 2.0f;

				TangentData = *(LPD3DXVECTOR4)pTexTangent;
				pTexTangent += 4;
				TangentData.x -= 0.5f;	TangentData.y -= 0.5f;	TangentData.z -= 0.5f;
				TangentData *= 2.0f;
			}

			// ���붥������
			VertexPosition = *(LPD3DXVECTOR3)pVertexSrc;
			pVertexSrc += (g_nEachPointSize / sizeof(float));

			// FVF��ָ��˳�����������˳����Ū��
			// x y z��ֻ�ı�y			
			*pVertexDst++ = VertexPosition.x + fDeltaHeightX;
			*pVertexDst++ = VertexPosition.y + fDeltaHeightY;
			pVertexDst++;
			// normal
			*pVertexDst++ = NormalData.x;
			*pVertexDst++ = NormalData.y;
			*pVertexDst++ = NormalData.z;
			// texcoord������
			pVertexDst++;
			pVertexDst++;
			// tangent
			*pVertexDst++ = TangentData.x;
			*pVertexDst++ = TangentData.y;
			*pVertexDst++ = TangentData.z;
			// binormal������
			pVertexDst++;
			pVertexDst++;
			pVertexDst++;
		}
	}


	// ����
	g_pMesh->UnlockVertexBuffer();
	g_pMeshCPU->UnlockVertexBuffer();
	g_pTexHeightMapCPU->UnlockRect(0);
	if(g_bGenerateWaterMeshMap)
	{
		g_pTexNormalMapCPU->UnlockRect(0);
		g_pTexTangentMapCPU->UnlockRect(0);
	}

	return S_OK;
}



// ����һ��XZƽ�棬�ö��С����棬���ֱ�ָ�����������ϵĸ�������X��Z�����ܳ���Width/Depth��
// �����ָ�뼴�ɣ������Զ������µĶ���/��������
// FVFĬ��ΪXYZ+Tex1+NORMAL��������Ĭ����ԭ��Ϊƽ�����Ͻǣ��ֱ���X����Z�������죬������Ҫ���������߻��һ�㣬���ܿ��ó�����ƽ��
// ��TriangleList����Primitive����2 * nXSplitNum * nZSplitNum����������(nXSplitNum+1) * (nZSplitNum+1)
HRESULT InitPlane(UINT nXSplitNum, float fWidth, UINT nZSplitNum, float fDepth, LPD3DXMESH &pMesh)
{
	if(nXSplitNum < 1 || nZSplitNum < 1 || fWidth <= 0 || fDepth <= 0)
		return D3DERR_INVALIDCALL;

	SAFE_RELEASE(pMesh);

	// ��������
	UINT nX = nXSplitNum + 1;
	UINT nZ = nZSplitNum + 1;
	UINT nPointNum = nX * nZ;

	float fEachX = (float)fWidth / (float)nXSplitNum;
	float fEachZ = (float)fDepth / (float)nZSplitNum;

	UINT nIndexNum = nXSplitNum * nZSplitNum * 6;	// ÿ��������Ҫ���������Σ���6������
	UINT nFaceNum = nXSplitNum * nZSplitNum * 2;
	UINT nEachIndexSize = nIndexNum>65535 ? sizeof(DWORD) : sizeof(WORD);
	D3DFORMAT IndexFormat = nIndexNum>65535 ? D3DFMT_INDEX32 : D3DFMT_INDEX16;

	// ������������VB����ɶ�
	if(nIndexNum > 65535)
	{
		V_RETURN(D3DXCreateMesh(nFaceNum, nPointNum, D3DXMESH_MANAGED | D3DXMESH_32BIT, g_WaterDeclaration, d3ddevice, &pMesh));
	}
	else
	{
		V_RETURN(D3DXCreateMesh(nFaceNum, nPointNum, D3DXMESH_MANAGED, g_WaterDeclaration, d3ddevice, &pMesh));
	}

	float * pVertexData = NULL;
	V_RETURN(pMesh->LockVertexBuffer(0, (void **)&pVertexData));

	void* pIndexData = NULL;
	V_RETURN(pMesh->LockIndexBuffer(0, (void **)&pIndexData));

	// �������
	UINT i = 0, j = 0;
	for(j = 0; j < nZ; j++)
	{
		for(i = 0; i < nX; i++)
		{
			// FVF��ָ��˳�����������˳����Ū��
			// x y z
			*pVertexData++ = (float)i * fEachX;
			*pVertexData++ = 0.0f;
			*pVertexData++ = (float)j * -fEachZ;
			// normal��һ��Ϊ0,1,0
			*pVertexData++ = 0.0f;
			*pVertexData++ = 1.0f;
			*pVertexData++ = 0.0f;
			// texcoord
			*pVertexData++ = (float)i / (float)nXSplitNum;
			*pVertexData++ = (float)j / (float)nZSplitNum;
			// tangent��һ��Ϊ1,0,0
			*pVertexData++ = 1.0f;
			*pVertexData++ = 0.0f;
			*pVertexData++ = 0.0f;
			// binormal��һ��Ϊ0,0,1
			*pVertexData++ = 0.0f;
			*pVertexData++ = 0.0f;
			*pVertexData++ = 1.0f;
		}
	}

	// DWORD�͵�
	if(IndexFormat == D3DFMT_INDEX32)
	{
		DWORD* pCurIndex = (DWORD *)pIndexData;
		// ÿ�������4���㣬���ϡ����ϡ����¡�����
		DWORD dwIndex1 = 0, dwIndex2 = 0, dwIndex3 = 0, dwIndex4 = 0;

		for(j = 0; j < nZSplitNum; j++)
		{
			for(i = 0; i < nXSplitNum; i++)
			{
				dwIndex1 = j * nX + i;
				dwIndex2 = dwIndex1 + 1;
				dwIndex3 = dwIndex2 + nX;
				dwIndex4 = dwIndex1 + nX;

				*pCurIndex++ = dwIndex1;
				*pCurIndex++ = dwIndex2;
				*pCurIndex++ = dwIndex4;
				*pCurIndex++ = dwIndex2;
				*pCurIndex++ = dwIndex3;
				*pCurIndex++ = dwIndex4;
			}
		}
	}
	else
	{
		WORD* pCurIndex = (WORD *)pIndexData;
		// ÿ�������4���㣬���ϡ����ϡ����¡�����
		WORD dwIndex1 = 0, dwIndex2 = 0, dwIndex3 = 0, dwIndex4 = 0;

		for(j = 0; j < nZSplitNum; j++)
		{
			for(i = 0; i < nXSplitNum; i++)
			{
				dwIndex1 = j * nX + i;
				dwIndex2 = dwIndex1 + 1;
				dwIndex3 = dwIndex2 + nX;
				dwIndex4 = dwIndex1 + nX;

				*pCurIndex++ = dwIndex1;
				*pCurIndex++ = dwIndex2;
				*pCurIndex++ = dwIndex4;
				*pCurIndex++ = dwIndex2;
				*pCurIndex++ = dwIndex3;
				*pCurIndex++ = dwIndex4;
			}
		}
	}

	pMesh->UnlockVertexBuffer();
	pMesh->UnlockIndexBuffer();

	return S_OK;
}


















































// Not-Normalize
D3DXVECTOR3 GetTangent(D3DXVECTOR3 Vertex[3], D3DXVECTOR2 TexCoord[3])
{
	D3DXVECTOR3 P1, P2, T1, U, V;
	UINT n1 = 0, n2 = 1, n3 = 2;
	P1=D3DXVECTOR3(Vertex[n2].x-Vertex[n1].x,TexCoord[n2].x-TexCoord[n1].x,TexCoord[n2].y-TexCoord[n1].y);
	P2=D3DXVECTOR3(Vertex[n3].x-Vertex[n1].x,TexCoord[n3].x-TexCoord[n1].x,TexCoord[n3].y-TexCoord[n1].y);

	T1.x=P1.y*P2.z-P1.z*P2.y;
	T1.y=P1.z*P2.x-P1.x*P2.z;
	U.x=-T1.y/T1.x;

	P1.x=Vertex[n2].y-Vertex[n1].y;
	P2.x=Vertex[n3].y-Vertex[n1].y;
	T1.x=P1.y*P2.z-P1.z*P2.y;
	T1.y=P1.z*P2.x-P1.x*P2.z;
	U.y=-T1.y/T1.x;

	P1.x=Vertex[n2].z-Vertex[n1].z;
	P2.x=Vertex[n3].z-Vertex[n3].z;
	T1.x=P1.y*P2.z-P1.z*P2.y;
	T1.y=P1.z*P2.x-P1.x*P2.z;
	U.z=-T1.y/T1.x;

	return U;
}


HRESULT MyComputeTangent()
{
	D3DXVECTOR3 Position[3] = {
		D3DXVECTOR3(0, 0, 1),
		D3DXVECTOR3(1, 0, 0),
		D3DXVECTOR3(0.5, 0, 0),
	};
	D3DXVECTOR2 TexCoord[3] = {
		D3DXVECTOR2(0, 1),
		D3DXVECTOR2(1, 0),
		D3DXVECTOR2(0, 0),
	};

	D3DXVECTOR3 VecTempTangent = D3DXVECTOR3(0, 0, 0), VecTempBiNormal = D3DXVECTOR3(0, 0, 0);
	D3DXVECTOR3 VecTangent = D3DXVECTOR3(0, 0, 0), VecBiNormal = D3DXVECTOR3(0, 0, 0);


	UINT nXPointNum = g_OceanData.iWidth, nZPointNum = g_OceanData.iHeight;

	// ��������
	UINT nPointNum = nXPointNum * nZPointNum;

	// Lock VB & Tex
	float *pVertexHead = NULL, *pVertexRead = NULL, *pVertexWrite = NULL;
	D3DXVECTOR3 VertexPosition[9];	// in Source VB
	D3DXVECTOR2 VertexTexCoord[9];
	D3DXVECTOR3 D3DTangent;

	V_RETURN(g_pMeshCPU->LockVertexBuffer(0, (void **)&pVertexHead));
	pVertexWrite = pVertexHead;


	// �������
	UINT i = 0, j = 0;
	for(j = 0; j < nZPointNum; j++)
	{
		for(i = 0; i < nXPointNum; i++)
		{
			// ����ָ����ĸ߶Ⱥ������������ݵ�VertexPosition/TexCoord��
/*			GET_VERTEX_DATA(i, j, 0);
			GET_VERTEX_DATA(i-1, j-1, 1);
			GET_VERTEX_DATA(i, j-1, 2);
			GET_VERTEX_DATA(i+1, j-1, 3);
			GET_VERTEX_DATA(i-1, j, 4);
			GET_VERTEX_DATA(i+1, j, 5);
			GET_VERTEX_DATA(i-1, j+1, 6);
			GET_VERTEX_DATA(i, j+1, 7);
			GET_VERTEX_DATA(i+1, j+1, 8);
*/
			pVertexRead = pVertexHead + (nXPointNum * j + i) * (g_nEachPointSize / sizeof(float));
			VertexPosition[0] = *(LPD3DXVECTOR3)pVertexRead;
			pVertexRead += 6;
			VertexTexCoord[0] = *(LPD3DXVECTOR2)pVertexRead;

			pVertexRead = pVertexHead + (nXPointNum * (j-1) + (i-1)) * (g_nEachPointSize / sizeof(float));
			VertexPosition[1] = *(LPD3DXVECTOR3)pVertexRead;
			pVertexRead += 6;
			VertexTexCoord[1] = *(LPD3DXVECTOR2)pVertexRead;

			pVertexRead = pVertexHead + (nXPointNum * (j-1) + i) * (g_nEachPointSize / sizeof(float));
			VertexPosition[2] = *(LPD3DXVECTOR3)pVertexRead;
			pVertexRead += 6;
			VertexTexCoord[2] = *(LPD3DXVECTOR2)pVertexRead;

			pVertexRead = pVertexHead + (nXPointNum * (j-1) + (i+1)) * (g_nEachPointSize / sizeof(float));
			VertexPosition[3] = *(LPD3DXVECTOR3)pVertexRead;
			pVertexRead += 6;
			VertexTexCoord[3] = *(LPD3DXVECTOR2)pVertexRead;



			pVertexRead = pVertexHead + (nXPointNum * (j) + (i-1)) * (g_nEachPointSize / sizeof(float));
			VertexPosition[4] = *(LPD3DXVECTOR3)pVertexRead;
			pVertexRead += 6;
			VertexTexCoord[4] = *(LPD3DXVECTOR2)pVertexRead;

			pVertexRead = pVertexHead + (nXPointNum * (j) + (i+1)) * (g_nEachPointSize / sizeof(float));
			VertexPosition[5] = *(LPD3DXVECTOR3)pVertexRead;
			pVertexRead += 6;
			VertexTexCoord[5] = *(LPD3DXVECTOR2)pVertexRead;



			pVertexRead = pVertexHead + (nXPointNum * (j+1) + (i-1)) * (g_nEachPointSize / sizeof(float));
			VertexPosition[6] = *(LPD3DXVECTOR3)pVertexRead;
			pVertexRead += 6;
			VertexTexCoord[6] = *(LPD3DXVECTOR2)pVertexRead;

			pVertexRead = pVertexHead + (nXPointNum * (j+1) + i) * (g_nEachPointSize / sizeof(float));
			VertexPosition[7] = *(LPD3DXVECTOR3)pVertexRead;
			pVertexRead += 6;
			VertexTexCoord[7] = *(LPD3DXVECTOR2)pVertexRead;

			pVertexRead = pVertexHead + (nXPointNum * (j+1) + (i+1)) * (g_nEachPointSize / sizeof(float));
			VertexPosition[8] = *(LPD3DXVECTOR3)pVertexRead;
			pVertexRead += 6;
			VertexTexCoord[8] = *(LPD3DXVECTOR2)pVertexRead;

			// ����ÿ��������߲��ۼ�
			VecTangent = D3DXVECTOR3(0, 0, 0);
			VecBiNormal = D3DXVECTOR3(0, 0, 0);


			Position[0] = VertexPosition[2];
			Position[1] = VertexPosition[0];
			Position[2] = VertexPosition[4];
			TexCoord[0] = VertexTexCoord[2];
			TexCoord[1] = VertexTexCoord[0];
			TexCoord[2] = VertexTexCoord[4];
			D3DXVec3Normalize(&VecTempTangent, &GetTangent(Position, TexCoord));
			VecTangent += VecTempTangent;

			Position[0] = VertexPosition[0];
			Position[1] = VertexPosition[5];
			Position[2] = VertexPosition[7];
			TexCoord[0] = VertexTexCoord[0];
			TexCoord[1] = VertexTexCoord[5];
			TexCoord[2] = VertexTexCoord[7];
			D3DXVec3Normalize(&VecTempTangent, &GetTangent(Position, TexCoord));
			VecTangent += VecTempTangent;

			Position[0] = VertexPosition[2];
			Position[1] = VertexPosition[3];
			Position[2] = VertexPosition[0];
			TexCoord[0] = VertexTexCoord[2];
			TexCoord[1] = VertexTexCoord[3];
			TexCoord[2] = VertexTexCoord[0];
			D3DXVec3Normalize(&VecTempTangent, &GetTangent(Position, TexCoord));
			VecTangent += VecTempTangent;

			Position[0] = VertexPosition[3];
			Position[1] = VertexPosition[5];
			Position[2] = VertexPosition[0];
			TexCoord[0] = VertexTexCoord[3];
			TexCoord[1] = VertexTexCoord[5];
			TexCoord[2] = VertexTexCoord[0];
			D3DXVec3Normalize(&VecTempTangent, &GetTangent(Position, TexCoord));
			VecTangent += VecTempTangent;

			Position[0] = VertexPosition[4];
			Position[1] = VertexPosition[0];
			Position[2] = VertexPosition[6];
			TexCoord[0] = VertexTexCoord[4];
			TexCoord[1] = VertexTexCoord[0];
			TexCoord[2] = VertexTexCoord[6];
			D3DXVec3Normalize(&VecTempTangent, &GetTangent(Position, TexCoord));
			VecTangent += VecTempTangent;

			Position[0] = VertexPosition[0];
			Position[1] = VertexPosition[7];
			Position[2] = VertexPosition[6];
			TexCoord[0] = VertexTexCoord[0];
			TexCoord[1] = VertexTexCoord[7];
			TexCoord[2] = VertexTexCoord[6];
			D3DXVec3Normalize(&VecTempTangent, &GetTangent(Position, TexCoord));
			VecTangent += VecTempTangent;

			VecTangent /= 6.0f;
			D3DXVec3Normalize(&VecTangent, &VecTangent);


			// FVF��ָ��˳�����������˳����Ū��
			// x y z������
			pVertexWrite++;
			pVertexWrite++;
			pVertexWrite++;
			// normal������
			pVertexWrite++;
			pVertexWrite++;
			pVertexWrite++;
			// texcoord������
			pVertexWrite++;
			pVertexWrite++;
			// tangent
//			D3DTangent = *(LPD3DXVECTOR3)pVertexWrite;
//			pVertexWrite += 3;
//			if(i && j && VecTangent.x < 0.8f)
//				pVertexWrite = pVertexWrite;
			*pVertexWrite++ = VecTangent.x;
			*pVertexWrite++ = VecTangent.y;
			*pVertexWrite++ = VecTangent.z;
			// binormal
			*pVertexWrite++;// = VecBiNormal.x;
			*pVertexWrite++;// = VecBiNormal.y;
			*pVertexWrite++;// = VecBiNormal.z;
		}
	}


	// ����
	g_pMeshCPU->UnlockVertexBuffer();

	return S_OK;
}