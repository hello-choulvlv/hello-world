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


// 提示：注意改CameraChange中的ProjTransform远平面为250.0f！
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


//在此定义所有的物体或模型
MYTEXT Text;         //3#
VERTEXSHADER g_VSVTF, g_VSNoVTF;
PIXELSHADER g_PSVTF, g_PSNoVTF;
KFFTOceanWater g_FFTOceanWater, g_FFTOceanWaterHighFrequency;
OCEANWATER_ATTRIBUTE g_OceanData, g_OceanDataHighFrequency;


LPD3DXMESH g_pMesh = NULL, g_pMeshCPU = NULL;// 用于CPU Lock复制新的Mesh
LPDIRECT3DTEXTURE9 g_pTexHeightMapCPU = NULL, g_pTexNormalMapCPU = NULL, g_pTexTangentMapCPU = NULL;	// 高度图、法线和切线图，仅用于CPU Lock（GPU用GetHeight/NormalMap来得到纹理指针即可）
LPDIRECT3DTEXTURE9 g_pTexAddOnNormalMap = NULL;		// 在无法由GPU生成法线图的情况下，它作为定的法线图
LPDIRECT3DTEXTURE9 g_pTexAddOnHeightMap = NULL;		// 涟漪效果，用于叠加的高度图
LPDIRECT3DTEXTURE9 g_pTexture = NULL;	// 底图


KSkyDome SkyDome;
LPDIRECT3DCUBETEXTURE9 g_pTexEnvironment = NULL;	// 环境反射图
LPDIRECT3DTEXTURE9 g_pTexFresnel = NULL;	// Fresnel系数图



// 波动方程用
KWaveEquationWater g_WaveEquation;
WAVEEQUATION_ATTRIBUTE g_WaveEquationData;
LPDIRECT3DTEXTURE9 g_pTexObstacle = NULL;	// 障碍图（岛）
LPDIRECT3DTEXTURE9 g_ppTexInject[2] = {NULL, NULL};	// 波动干扰纹理（刷子）
NORMALMESH g_MeshBoat, g_MeshRock, g_MeshIsland;			// 船、石头和岛的模型
float g_fInjectHeightBoat = 1.0f, g_fInjectHeightRock = 0.15f;	// 入射的强度和范围
float g_fInjectRangeBoat = 0.1f, g_fInjectRangeRock = 0.1f;
VERTEXSHADER g_VSObject, g_VSCaustics;		// 渲染海面上的物体及海面下的物体（焦散）
PIXELSHADER g_PSObject, g_PSCaustics;
BOOL g_bDisplayIsland = FALSE;			// 是否显示岛
BOOL g_bUseNormalMap = FALSE;			// 是否使用NormalMap（可以提升不少性能）
UINT g_iStandardTimePerFrame = 50;		// 标准模拟帧数下每两帧的时间间隔（单位毫秒，用于控制整体速度）
D3DXVECTOR4 g_ColFogUnderWater = D3DXVECTOR4(0, 0, 0, 0);		// 水下雾化颜色
D3DXVECTOR2 g_FogAdjust = D3DXVECTOR2(26.0f, 1.0f);				// 雾的乘方和缩放
float g_fFogSkybox = 0.01f;


// 海面网格模型相关
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


// 逻辑控制
BOOL g_bCPU = FALSE;		// VTF硬件上仍然可以选用CPU来渲染，用户手动选择，可控制
BOOL g_bGenerateWaterMeshMap = TRUE;	// 用GPU来生成海浪相关的贴图（法线、切线图），但在PS2.a以下的显卡上不能生成，硬性指标，初始就定好了
BOOL g_bVTF = FALSE;		// 显卡是否支持VTF，硬性指标，初始就定好了
float test=1.0f;

D3DXVECTOR2 g_MeshWrapCoef(1, 1), g_HeightMapWrapCoef(1, 1);	// 用户配置文件控制的高度图、海面网格和法线图平铺系数，其中高度图和网格平铺系数仅VTF时有效，其他时候强制(1,1)
D3DXVECTOR2 g_NormalMapWrapCoef(1, 1);	



// 海水和光照颜色，从配置文件中读取
D3DXVECTOR4 g_ColShallowWater = D3DXVECTOR4(0, 0, 0, 0), g_ColDeepWater = D3DXVECTOR4(0, 0, 0, 0);
D3DXVECTOR4 g_ColDiffuse = D3DXVECTOR4(1, 1, 1, 0), g_ColSpecular = D3DXVECTOR4(1, 1, 1, 0);

// 光照方向
D3DXVECTOR4 g_VecDiffuse = D3DXVECTOR4(0, 1, 0, 0), g_VecSpecular = D3DXVECTOR4(1, -0.1f, 0, 0);


//在此定义所有物体的顶点模式和顶点坐标(仅限MODULE)
//开始操作
UINT InitMyScene()        //初始化每个物体的顶点缓冲、纹理或读入对应的X模型文件，还有设置灯光和摄像机等
{
	// 先确定显卡的Shader版本以便确定是CPU生成法、切线还是GPU，只要小于ps2.a就只能用CPU来生成法线切线
	if( !CheckPS2xSupport(32, FALSE) )		// 如果用GPU生成切线图的话，最少需要32个临时寄存器
		g_bGenerateWaterMeshMap = FALSE;
	else
		g_bGenerateWaterMeshMap = TRUE;

	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F)))
		g_bVTF = FALSE;
	else
		g_bVTF = TRUE;


	// 海浪参数
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

	// 从配置文件中读取
	char szProfileData[1000] = "";

		// 海水颜色（可以给定小数或整数）
	GetPrivateProfileString("Color", "ShallowWaterColor", "0.15625,0.37890625,0.328125", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_ColShallowWater, 3);
	if(g_ColShallowWater.x > 1.0f || g_ColShallowWater.y > 1.0f || g_ColShallowWater.z > 1.0f)
		g_ColShallowWater /= 255.0f;

	GetPrivateProfileString("Color", "DeepWaterColor", "0.15625,0.37890625,0.328125", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_ColDeepWater, 3);
	if(g_ColDeepWater.x > 1.0f || g_ColDeepWater.y > 1.0f || g_ColDeepWater.z > 1.0f)
		g_ColDeepWater /= 255.0f;

		// 光照颜色
	GetPrivateProfileString("Color", "DiffuseColor", "1.0,1.0,1.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_ColDiffuse, 3);
	if(g_ColDiffuse.x > 1.0f || g_ColDiffuse.y > 1.0f || g_ColDiffuse.z > 1.0f)
		g_ColDiffuse /= 255.0f;

	GetPrivateProfileString("Color", "SpecularColor", "1.0,1.0,1.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_ColSpecular, 3);
	if(g_ColSpecular.x > 1.0f || g_ColSpecular.y > 1.0f || g_ColSpecular.z > 1.0f)
		g_ColSpecular /= 255.0f;

		// 雾化颜色和微调
	GetPrivateProfileString("Color", "FogColor", "0.0,0.5,0.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_ColFogUnderWater, 3);
	if(g_ColFogUnderWater.x > 1.0f || g_ColFogUnderWater.y > 1.0f || g_ColFogUnderWater.z > 1.0f)
		g_ColFogUnderWater /= 255.0f;
	
	GetPrivateProfileString("Color", "FogAdjust", "26.0,1.0", szProfileData, 1000, CONFIG_FILENAME);
	GetFloatFromString(szProfileData, (float *)&g_FogAdjust, 2);

	GetPrivateProfileString("Color", "FogSkybox", "0.01", szProfileData, 1000, CONFIG_FILENAME);
	g_fFogSkybox = (float)atof(szProfileData);


		// 光照方向
	D3DXVec4Normalize(&g_VecDiffuse, &g_VecDiffuse);
	D3DXVec4Normalize(&g_VecSpecular, &g_VecSpecular);

		// 平铺系数
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

	

	// 初始化波动方程引擎
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


	randomize();	// 初始化随机发生器

		// 初始化波动方程使用的纹理和模型
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
			mymessage("Obstacle Texture分辨率不符合要求！");
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

		// 初始化波动方程使用的物体Shader
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



	

	// 初始化海浪处理引擎，高频海浪只是作法线图用，不用生成切线图
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

	// 得到网格信息
	HRESULT hr = S_OK;
	SAFE_DELETE_ARRAY(g_pAdjancy);
	g_pAdjancy = new DWORD[sizeof(DWORD) * g_pMeshCPU->GetNumFaces() * 3];
	memset(g_pAdjancy, 0, sizeof(DWORD) * g_pMeshCPU->GetNumFaces() * 3);
//	hr = g_pMeshCPU->GenerateAdjacency(0.00001f, g_pAdjancy);
	if(FAILED(hr))
		mymessage("Generate Adjancy Failed!");

	// 创建纹理
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

	// 创建天空盒、反射图和Fresnel数据图
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


	// 使用CPU做的，用SM2.0来渲染
	if(FAILED(g_VSNoVTF.InitVertexShader("shader\\NoVTFWater.vsh", g_WaterDeclaration)))
		myfail(MYERR_CREATEVERTEXSHADER);
	if(FAILED(g_PSNoVTF.InitPixelShader("shader\\NoVTFWater.psh")))
		myfail(MYERR_CREATEPIXELSHADER);
	// 不支持VTF的，就得用CPU来处理
	if(g_bVTF)
	{
		// 支持VTF的就用SM3.0
		if(FAILED(g_VSVTF.InitVertexShader("shader\\WaveEquationWater.vsh", g_WaterDeclaration)))
			myfail(MYERR_CREATEVERTEXSHADER);
		if(FAILED(g_PSVTF.InitPixelShader("shader\\WaveEquationWater.psh")))
			myfail(MYERR_CREATEPIXELSHADER);
	}
	

	//初始化字体
	Text.InitFont2D(16,"宋体");
	//初始化透视变换
	CameraChange.ProjTransform(1.3f);
	//设置全局的渲染模式和纹理模式
	d3ddevice->SetSamplerState(0,D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0,D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1,D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1,D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	d3ddevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	d3ddevice->SetRenderState( D3DRS_NORMALIZENORMALS, FALSE );
	d3ddevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );  //此开关不要作为全局设置，渲染某些物体时单独设定，初始为显示顺时针的正面

	return S_OK;
}













// 设置划船波动的移动点（波动产生源）
// 参数是连续用户交互速度控制，第一个参数表示是否跳过交互设置，第二个参数是根据帧间隔时间对标准高度进行缩放，帧数越低高度就应该越低（防止累积效应）
void MoveBoat(BOOL bSimulation, float fHeightCoef)
{
	// 划船，这里产生波动要用纹理2D坐标，初始位置在X右端
	D3DXVECTOR2 PtBoatInitPos = D3DXVECTOR2(0.9f, 0.5f), PtBoatTempPos;
	D3DXVECTOR2 PtBoatPosTexture, PtBoatPosWorld;

	// 每帧都绕中心旋转
	D3DXMATRIX MatRotate;


	static DWORD dwStartTime = timeGetTime();
	// 旋转量，10秒走完一圈（360度）
	float fAngle = ( timeGetTime() - dwStartTime ) / 10000.0f * 2.0f * D3DX_PI;

	D3DXMatrixRotationZ(&MatRotate, fAngle);
	// Test
	//D3DXMatrixIdentity(&MatRotate);

	// 先平移到中心，再旋转，再平移回去
	PtBoatTempPos = PtBoatInitPos - D3DXVECTOR2(0.6f, 0.5f);
	D3DXVec2TransformCoord(&PtBoatPosTexture, &PtBoatTempPos, &MatRotate);
	PtBoatPosTexture = PtBoatPosTexture +  D3DXVECTOR2(0.5f, 0.5f);

	// 产生波动
	UINT iX = (UINT)(PtBoatPosTexture.x * g_WaveEquationData.iWidth);
	UINT iY = (UINT)(PtBoatPosTexture.y * g_WaveEquationData.iHeight);
	float fHeight = g_fInjectHeightBoat;		// 标准帧数下的高度
		// 有障碍时重点突出波浪的反弹效果，加大船波浪的高度
	if(g_bDisplayIsland)
		fHeight *= 1.5f;

	if(bSimulation && fHeightCoef > 0.0f)
	{
		fHeight *= fHeightCoef;
		if(FAILED(g_WaveEquation.SetAreaHeight(iX, iY, D3DXVECTOR2(g_fInjectRangeBoat, g_fInjectRangeBoat), g_ppTexInject[1], fHeight, FALSE)))
			mymessage("Set Height Failed");
	}




	// 渲染
	D3DXMATRIX MatWorld, MatScaling, MatTrans;
	D3DXVECTOR3 PtPos(0, 0, 0);
	// 缩放
	D3DXMatrixScaling(&MatScaling, 0.3f, 0.3f, 0.3f);
	// 平移，将当前位置转换为世界坐标，平面是沿Z负方向的
	PtPos.x = PtBoatPosTexture.x * g_WaveEquationData.WaterSquare.x;
	PtPos.z = -PtBoatPosTexture.y * g_WaveEquationData.WaterSquare.y;
	D3DXMatrixTranslation(&MatTrans, PtPos.x, PtPos.y, PtPos.z);
	// 世界中的朝向
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

	// 石头
	if(!effectenable)
	{
		D3DXVECTOR3 PtRock(0, 75.0f, 0);
		// 如果有按回车，就生成一块新石头，在随机XZ位移处（水面0.1～0.9范围内），y值给定天空盒顶高度75
		while(PtRock.x < 0.1f)
			PtRock.x = randomf(0.9f, 2);
		while(PtRock.z < 0.1f)
			PtRock.z = randomf(0.9f, 2);
		s_vecPtRocks.push_back(PtRock);

		effectenable = !effectenable;
	}

	// 处理列表中每块石头的掉落，掉落速度为匀速，设置为1.2秒内掉到水面，即每秒运行75/1.2
	std::vector<D3DXVECTOR3>::iterator it;
	// 位移改变量
	float fDeltaHeight = ( dwCurrentTime -  dwLastTime ) / 1000.0f * 75.0f / 1.2f;

	for(it = s_vecPtRocks.begin(); it != s_vecPtRocks.end(); it++)
	{
		// 计算出当前高度值
		it->y -= fDeltaHeight;

		// 渲染出来
		D3DXMATRIX MatWorld, MatScaling, MatTrans;
		D3DXVECTOR3 PtPos(0, 0, 0);
		// 缩放
		D3DXMatrixScaling(&MatScaling, 0.07f, 0.07f, 0.07f);
		// 平移，转换为世界坐标的位置
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


		// 根据帧数设置高度，瞬时作用的交互，不用根据帧数对高度值进行缩放
		float fHeight = g_fInjectHeightRock;

		// 如果达到水面，就产生波动，并从列表中移除该石头
		if(it->y < 0.0f)
		{
			UINT iX = (UINT)(it->x * g_WaveEquationData.iWidth);
			UINT iY = (UINT)(it->z * g_WaveEquationData.iHeight);
			if(bSimulate)
			{
				if(FAILED(g_WaveEquation.SetAreaHeight(iX, iY, D3DXVECTOR2(g_fInjectRangeRock, g_fInjectRangeRock), g_ppTexInject[1], fHeight, TRUE)))
					mymessage("Set Height Failed");
			}

			// 移除
			s_vecPtRocks.erase(it);

			// 如果清空就退出，否则程序会挂
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
	// 缩放
//	D3DXMatrixScaling(&MatScaling, 0.0015f, 0.0015f, 0.0015f);
//	D3DXMatrixScaling(&MatScaling, 0.15f, 0.15f, 0.15f);
	float fScale = 20.0f;
	D3DXMatrixScaling(&MatScaling, 0.5f, 0.5f, 0.5f);
	// 平移，转换为世界坐标的位置
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
	g_PSCaustics.SetConstant(7, &D3DXVECTOR4(1.0f, 0.0f, 0, 0), 1);	// Island关掉高光

	// For Bilinear Filtering
	g_PSCaustics.SetConstant(8, &D3DXVECTOR4((float)g_OceanData.iWidth, (float)g_OceanData.iHeight, 0, 0), 1);
	g_PSCaustics.SetConstant(9, &D3DXVECTOR4(1.0f / (float)g_OceanData.iWidth, 1.0f / (float)g_OceanData.iHeight, 0, 0), 1);

	// 水下才有焦散和雾化，这可以提速
	BOOL bUnderWater = FALSE;
	if(CameraChange.Eye.y < 2.0f)		// 因为有波浪，所以水平面稍微提高一些
		bUnderWater = TRUE;
	g_PSCaustics.SetConstantB(0, &bUnderWater, 1);


	// 设置摄像机和投影方式
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

	// 先保存当前的RT和深度缓冲，设置新的深度缓冲以便FFT渲染
	LPDIRECT3DSURFACE9 pOldBackBuffer = NULL, pOldDepthBuffer = NULL;

	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldBackBuffer));
	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepthBuffer));

	V_RETURN(d3ddevice->SetDepthStencilSurface(pDepthBuffer));


	// 设置RT、Clear
	V_RETURN(d3ddevice->SetRenderTarget(0, pRT));
	V_RETURN(d3ddevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0));

	// 设置颜色混合参数
	SetTextureColorMix(0, D3DTOP_SELECTARG1, D3DTA_TFACTOR, D3DTA_TEXTURE);
	SetTextureColorMix(1, D3DTOP_DISABLE, D3DTA_TFACTOR, D3DTA_TEXTURE);
	d3ddevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0);

	// 设置摄像机和投影方式
	D3DXMATRIX MatView, MatProj;
	D3DXVECTOR3 PtCameraPos(g_WaveEquationData.WaterSquare.x / 2.0f, 30.0f, g_WaveEquationData.WaterSquare.y / -2.0f);
	D3DXVECTOR3 PtLookAtPos(g_WaveEquationData.WaterSquare.x / 2.0f, 0.0f, g_WaveEquationData.WaterSquare.y / -2.0f);
	D3DXVECTOR3 VecHead(0, 0, 1);
	D3DXVECTOR2 VecProjRange = g_WaveEquationData.WaterSquare;
	D3DXMatrixOrthoLH(&MatProj, VecProjRange.x, VecProjRange.y, 1.0f, 150.0f);
	D3DXMatrixLookAtLH(&MatView, &PtCameraPos, &PtLookAtPos, &VecHead);
	d3ddevice->SetTransform(D3DTS_VIEW, &MatView);
	d3ddevice->SetTransform(D3DTS_PROJECTION, &MatProj);


	// 设置世界矩阵
	D3DXMATRIX MatWorld, MatScaling, MatTrans;
	//	D3DXVECTOR3 PtPos(80, 6, -80);
	//	D3DXVECTOR3 PtPos(80, 8, -80);
	D3DXVECTOR3 PtPos(80, 0, -80);
	// 缩放
	//	D3DXMatrixScaling(&MatScaling, 0.0015f, 0.0015f, 0.0015f);
	//	D3DXMatrixScaling(&MatScaling, 0.15f, 0.15f, 0.15f);
	D3DXMatrixScaling(&MatScaling, 0.5f, 0.5f, 0.5f);
	// 平移，转换为世界坐标的位置
	D3DXMatrixTranslation(&MatTrans, PtPos.x, PtPos.y, PtPos.z);

	MatWorld = MatScaling * MatTrans;
	d3ddevice->SetTransform(D3DTS_WORLD, &MatWorld);

	g_VSObject.SetTransform(&MatWorld);
	d3ddevice->SetPixelShader(NULL);
	g_MeshIsland.DrawAll(true, g_VSObject.Declaration, NULL);


	// 绘制水面（为了保证遮挡关系）
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


	// 恢复RT和深度缓冲及设定
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldBackBuffer));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepthBuffer));

	SAFE_RELEASE(pOldBackBuffer);
	SAFE_RELEASE(pOldDepthBuffer);

	// 保存，结束
	V_RETURN(D3DXSaveSurfaceToFile("obstacle.tga", D3DXIFF_TGA, pRT, NULL, NULL));
	SAFE_RELEASE(pVB);
	SAFE_RELEASE(pIB);
	SAFE_RELEASE(pRT);
	SAFE_RELEASE(pDepthBuffer);
	return S_OK;
}




void SetMyScene(UINT No)      //设置每个物体的纹理、材质、顶点模式、渲染模式和纹理模式等，还有世界变换
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


	// 模拟控制，默认标准20帧（50ms），这里采用Boat和Simulation一致的方式
	BOOL bWaveSimulateThisFrame = FALSE, bMoveBoatThisFrame = FALSE;
	DWORD INTERVAL_WAVE = g_iStandardTimePerFrame, INTERVAL_BOAT = g_iStandardTimePerFrame;

	static DWORD s_dwLastFrameTime = timeGetTime();
	DWORD dwCurrentFrameTime = timeGetTime();
	DWORD dwDeltaFrameTime = dwCurrentFrameTime - s_dwLastFrameTime;
	static DWORD s_dwWaveAccumulateTime = 0, s_dwBoatAccumulateTime = 0;

	float fHeightCoef = 1.0f, fWaveSpeedCoef = 1.0f;
	

	// 跳过还是进行波动模拟
	if(s_dwWaveAccumulateTime > INTERVAL_WAVE)
	{
		bWaveSimulateThisFrame = TRUE;
		// 求余，减到不能减为止
		while(s_dwWaveAccumulateTime > INTERVAL_WAVE)
			s_dwWaveAccumulateTime -= INTERVAL_WAVE;
		// 累加
		s_dwWaveAccumulateTime += dwDeltaFrameTime;
	}
	else
	{
		bWaveSimulateThisFrame = FALSE;
		s_dwWaveAccumulateTime += dwDeltaFrameTime;
	}

	// 跳过还是进行船交互模拟
	if(s_dwBoatAccumulateTime > INTERVAL_BOAT)
	{
		bMoveBoatThisFrame = TRUE;
		// 求余，减到不能减为止
		while(s_dwBoatAccumulateTime > INTERVAL_BOAT)
			s_dwBoatAccumulateTime -= INTERVAL_BOAT;
		// 累加
		s_dwBoatAccumulateTime += dwDeltaFrameTime;
	}
	else
	{
		bMoveBoatThisFrame = FALSE;
		s_dwBoatAccumulateTime += dwDeltaFrameTime;
	}

	s_dwLastFrameTime = dwCurrentFrameTime;


	// 低帧数情况下，适当降低设置高度，提高波速
	if(dwDeltaFrameTime > INTERVAL_WAVE && bWaveSimulateThisFrame)
	{
		fWaveSpeedCoef = (float)dwDeltaFrameTime / (float)INTERVAL_WAVE;
	}
	if(dwDeltaFrameTime > INTERVAL_BOAT && bMoveBoatThisFrame)
	{
		fHeightCoef = (float)INTERVAL_BOAT / (float)dwDeltaFrameTime;
		fHeightCoef *= fHeightCoef;
	}
	


	// 在这里生成ObstacleMap
//	RenderIslandToObstacleMap();
//	PostQuitMessage(0);
//	return;

	switch(No)
	{
		case 0:
			// 渲染天空盒子
			D3DXMatrixScaling(&temp, 75, 75, 75);
			D3DXMatrixTranslation(&matworld, 75, 0, -75);
			matworld = temp * matworld;
			d3ddevice->SetTransform(D3DTS_WORLD, &matworld);
				// 水下，开启雾化
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



			// 处理用户控制
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


			// 移动物体，作波动方程模拟，如果低于最低帧数，就不能做用户交互
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


			// 根据帧数设置波速，帧数越低波速就应该越快，不过小心这里波速会过高引起数值不稳定！
			float fSpeed;
			fSpeed = g_WaveEquationData.fWaveSpeed * fWaveSpeedCoef;
//			g_WaveEquation.SetWaveSpeed(fSpeed);
			if(bWaveSimulateThisFrame)
			{
				if(FAILED(g_WaveEquation.WaterSimulation(1.0f)))
					mymessage("Water Simulation Failed");
			}


			// 海浪仿真
			if(FAILED(g_FFTOceanWater.WaterSimulation(dwOceanDeltaTime, g_WaveEquation.GetHeightMap())))
				mymessage("Ocean Simulation Failed");

			if(g_bUseNormalMap)
			{
				if(FAILED(g_FFTOceanWaterHighFrequency.WaterSimulation(dwOceanDeltaTime * 2.0f)))
					mymessage("Ocean Simulation Failed");
			}


			// 渲染岛并设置阻挡模式
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


			// 不支持的只能用CPU来Lock了，并用相应的VB
			if(!g_bVTF)
			{
				if(FAILED(ChangeWaterMesh()))
					mymessage("Change Mesh Failed");

				// 连GPU生成法线图都不行，那就只能用初始生成的静态法线图了
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
			// 支持的VTF就用VTF贴图和对应的VB
			else
			{
				// VTF硬件上仍然可以用CPU来渲染
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

			// 底图
			d3ddevice->SetTexture(0, g_pTexture);
			d3ddevice->SetTexture(1, g_pTexEnvironment);
			d3ddevice->SetTexture(2, g_pTexFresnel);
			// 可以生成法线图，就用FFT的高频海浪法线图，否则就用静态的附加法线图
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

			// 矩阵
			D3DXMatrixIdentity(&matworld);
			pVS->SetTransform(&matworld);

			// Wave Scalar
			fScaler = g_OceanData.fWaveHeightScale;
			pVS->SetConstant(1, (float *)&D3DXVECTOR4(fScaler, fScaler, fScaler, fScaler), 1);
			// NormalMap Dimension
			pVS->SetConstant(2, (float *)&D3DXVECTOR4((float)g_OceanDataHighFrequency.iWidth, (float)g_OceanDataHighFrequency.iHeight, 0, 1), 1);
			// Texture Wrap Coef
			pVS->SetConstant(3, (float *)&D3DXVECTOR4(g_HeightMapWrapCoef.x, g_HeightMapWrapCoef.y, 0, 1), 1);

			// 海水颜色，0和1分别表示浅水和深水颜色
			//pPS->SetConstant(0, (float *)&D3DXVECTOR4(40.0f/256.0f, 97.0/256.0f, 84.0f/256.0f, 1), 1);		// From PDF
			//pPS->SetConstant(1, (float *)&D3DXVECTOR4(38.0f/256.0f, 79.0/256.0f, 99.0f/256.0f, 1), 1);	// From szlongman
			pPS->SetConstant(0, &g_ColShallowWater, 1);		// From Config
			pPS->SetConstant(1, &g_ColDeepWater, 1);

			// Diffuse和Specular光照颜色
			pPS->SetConstant(2, &g_ColDiffuse, 1);
			pPS->SetConstant(3, &g_ColSpecular, 1);

			// Diffuse和Specular光照颜色方向（Y轴）
			pPS->SetConstant(4, &g_VecDiffuse, 1);
			pPS->SetConstant(5, &g_VecSpecular, 1);

			// 法线图纹理坐标缩放比例，用于扩展法线图，通过它控制整个海面平均覆盖横纵向多少个法线图
			pPS->SetConstant(6, (float *)&D3DXVECTOR4(g_NormalMapWrapCoef.x, g_NormalMapWrapCoef.y, 0, 1), 1);
			// 1/Width, 1/Height, for normalmap bilinear filtering
			pPS->SetConstant(7, (float *)&D3DXVECTOR4(1.0f / (float)g_OceanDataHighFrequency.iWidth, 1.0f / (float)g_OceanDataHighFrequency.iHeight, 0, 1), 1);

			// 计算CP向量用
			pPS->SetConstant(8, (float *)&D3DXVECTOR4((float)g_OceanData.WaterSquare.x / 2.0f, 0.0f, (float)g_OceanData.WaterSquare.y / -2.0f, 1), 1);
			float fRadius;
			fRadius = sqrtf(g_OceanData.WaterSquare.x * g_OceanData.WaterSquare.x + g_OceanData.WaterSquare.y * g_OceanData.WaterSquare.y);
			pPS->SetConstant(9, (float *)&D3DXVECTOR4(1.0f / fRadius, 1.0f / fRadius, 1.0f / fRadius, 1.0f / fRadius), 1);

			// 折射率，注意摄像机在水上和水下是不同的
			float fRefractIndex;
			if(CameraChange.Eye.y < 0.0f)
				fRefractIndex = 1.0f / 1.33f;
			else
				fRefractIndex = 1.33f;
			pPS->SetConstant(20, (float *)&D3DXVECTOR4(fRefractIndex, fRefractIndex, fRefractIndex, fRefractIndex), 1);


			// 是否使用NormalMap
			pPS->SetConstantB(0, &g_bUseNormalMap, 1);

			// Fog：摄像机到水下，水面才产生雾化
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
			
			// 绘制
			pPS->SetPixelShader();
			d3ddevice->SetStreamSource(0, pVB, 0, g_nEachPointSize);
			d3ddevice->SetIndices(pIB);

			if(bWireFrame)
				d3ddevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			pVS->DrawIndexPrimitive(0, g_pMesh->GetNumFaces(), g_pMesh->GetNumVertices());

			// 恢复
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

void RenderMyScene(UINT No)       //渲染每个物体，具体渲染哪个根据参数而定
{
	switch(No)
	{
		case 0:
			char a[500];
			sprintf(a, "FPS: %.0f", curfps);
			Text.DrawText2D(a, 0, 100, 150, RED);
			
			sprintf(a, "基于快速傅利叶变换（FFT）和Gestner波浪的Wind-Driven海水模拟，使用%d*%d的海面网格", g_OceanData.iWidth, g_OceanData.iHeight, g_OceanDataHighFrequency.iWidth, g_OceanDataHighFrequency.iHeight);
			Text.DrawText2D(a, 0, 30, 0, YELLOW);

			Text.DrawText2D("水面和外界物体的物理互动用二维波动方程来模拟，岛屿在水下的刻蚀（焦散）用简化的逆向光线跟踪来计算", 0, 30, 30, YELLOW);
			Text.DrawText2D("键盘控制：按回车会随机掉下一块石头，观察砸在水面上的效果（只是不要太BT哦-_-!!）", 0, 100, 100, BLACK);
			Text.DrawText2D("          按+/-可以切换线框模式，及水面上的海岛，观察波浪在岛边上被阻碍的效果", 0, 100, 120, BLACK);

			if(curfps < WATER_MIN_FPS)
				Text.DrawText2D("注意：当前帧数太低，无法显示波动！！", 0, 0, 70, RED);

			/*
			if(!g_bVTF)
			{
				Text.DrawText2D("由于显卡不支持VTF，只能用CPU来渲染海浪，帧数会非常低", 0, 0, 30, RED);

				if(!g_bGenerateWaterMeshMap)
				{
					Text.DrawText2D("由于ATI显卡Shader版本较低，现在只能用CPU来计算法线和切线，帧数会雪上加霜……", 0, 0, 60, RED);
					Text.DrawText2D("当前四步渲染的情况：Phillips频谱生成（GPU）、傅里叶反变换（GPU）、法线切线生成（CPU）、海浪顶点渲染（CPU）", 0, 0, 100, BLUE);
				}
				else
					Text.DrawText2D("当前四步渲染的情况：Phillips频谱生成（GPU）、傅里叶反变换（GPU）、法线切线生成（GPU）、海浪顶点渲染（CPU）", 0, 0, 100, BLUE);
			}
			else
			{
				if(g_bCPU)
				{
					Text.DrawText2D("现在强制用CPU来渲染海浪，帧数会急剧下降", 0, 0, 30, RED);
					Text.DrawText2D("当前四步渲染的情况：Phillips频谱生成（GPU）、傅里叶反变换（GPU）、法线切线生成（GPU）、海浪顶点渲染（CPU）", 0, 0, 100, BLUE);
				}
				else
				{
					Text.DrawText2D("现在是用GPU支持的VTF来全速渲染海浪，帧数有了很大的提高，可以按回车切换到CPU，看看差距", 0, 0, 30, RED);
					Text.DrawText2D("当前四步渲染的情况：Phillips频谱生成（GPU）、傅里叶反变换（GPU）、法线切线生成（GPU）、海浪顶点渲染（GPU）", 0, 0, 100, BLUE);
				}
			}
			*/
			break;
		case 1:
			// 在这里生成附加法线图
//			D3DXSaveTextureToFile("c:\\normal.hdr", D3DXIFF_HDR, g_FFTOceanWaterHighFrequency.GetNormalMap(), NULL);
			break;
	}
}



UINT ResetDevice();
UINT DisplayMyScene()  //循环将每个物体都显示出来，此函数每被运行一次表示一帧
{
	LARGE_INTEGER start, end;
	static DWORD LastUpdateFPSCount = GetTickCount(), CurUpdateFPSCount;
	UCHAR UpdateFPS = 0;
	static DWORD CurfpsNum = 0;
	CurfpsNum++;
	
	
	//处理设备丢失
	HRESULT hr = d3ddevice->TestCooperativeLevel();
	UINT InfoID = 0;
	
	if(FAILED(hr))
	{
		if(hr == D3DERR_DEVICENOTRESET)
			InfoID = ResetDevice();
		return InfoID;
	}
	
	//处理显示模式切换
	if(ChangeDisplayModeSign)
	{
		ChangeDisplayModeSign = FALSE;
		InfoID = ResetDevice();
		return InfoID;
	}
	
	//FPS更新控制
	CurUpdateFPSCount = GetTickCount();
	
	if(CurUpdateFPSCount - LastUpdateFPSCount > FPSUPDATETIME || curfps == 0)
	{
		UpdateFPS = 1;
		LastUpdateFPSCount = GetTickCount();
	}
	
	//开始计时
	QueryPerformanceCounter(&start);
	
	d3ddevice->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(128, 0, 0),1.0f,0);
	
	for(UINT i=0;i<1;i++)
	{
		//如果遇到临时不需要渲染的物体，在此处设置
		//if(i!=2) continue;
		SetMyScene(i);
		RenderMyScene(i);
	}
    
	//摄像机镜头变换
	CameraChange.ViewTransform();
	
	QueryPerformanceCounter(&end);
	
	d3ddevice->Present(NULL,NULL,NULL,NULL);
	
	//计算当前帧数
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
	//在这里释放所有POOL_DEFAULT的资源，包括字体和所有的模型
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

//在程序启动时先运行一次InitMyscene()，然后在WINDOWS主循环







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
	// 填充数据
	for(UINT iX = 0; iX < iWidth; iX++)
	{		
		fCosTerm = (float)iX / (float)iWidth;
		*pData++ = GetFresnelCoef(1.0f, 1.333f, fCosTerm);
	}

	// 结束
	g_pTexFresnel->UnlockRect(0);
	return S_OK;
}










HRESULT ChangeWaterMesh()
{
	// 先Load FFT好的贴图到内存
	V_RETURN(D3DXLoadTextureFromTexture(g_pTexHeightMapCPU, g_FFTOceanWater.GetHeightMap()));
	if(g_bGenerateWaterMeshMap)
	{
		V_RETURN(D3DXLoadTextureFromTexture(g_pTexNormalMapCPU, g_FFTOceanWater.GetNormalMap()));
		V_RETURN(D3DXLoadTextureFromTexture(g_pTexTangentMapCPU, g_FFTOceanWater.GetTangentMap()));
	}

	UINT nXPointNum = g_OceanData.iWidth, nZPointNum = g_OceanData.iHeight;

	// 顶点数量
	UINT nPointNum = nXPointNum * nZPointNum;

	// Lock VB & Tex
	float *pVertexSrc = NULL, *pVertexDst = NULL;
	float *pTexData = NULL, *pTexNormal = NULL, *pTexTangent = NULL;
	D3DLOCKED_RECT Rect, RectNormal, RectTangent;
	D3DXVECTOR4 HeightData, NormalData, TangentData;	// in Tex
	float fDeltaHeightX = 0.0f, fDeltaHeightY = 0.0f;	// XY轴增量
	D3DXVECTOR3 VertexPosition;	// in Source VB

	V_RETURN(g_pMesh->LockVertexBuffer(0, (void **)&pVertexSrc));
	V_RETURN(g_pMeshCPU->LockVertexBuffer(0, (void **)&pVertexDst));
	V_RETURN(g_pTexHeightMapCPU->LockRect(0, &Rect, NULL, 0));
	if(g_bGenerateWaterMeshMap)
	{
		V_RETURN(g_pTexNormalMapCPU->LockRect(0, &RectNormal, NULL, 0));
		V_RETURN(g_pTexTangentMapCPU->LockRect(0, &RectTangent, NULL, 0));
	}


	// 填充数据
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
			// 读入高度图数据，并根据符号重置数据
			HeightData = *(LPD3DXVECTOR4)pTexData;
			pTexData += 4;
			HeightData.x *= (HeightData.z - 1);
			HeightData.y *= (HeightData.w - 1);

			// 乘海浪缩放值
			HeightData *= g_OceanData.fWaveHeightScale;

			// 得到XY轴增量数据（Gestner Wave）
			fDeltaHeightY = HeightData.x;
			fDeltaHeightX = HeightData.y;
			// Test
			//			fDeltaHeightX = 0;
			//			fDeltaHeightY = 0;

			// 读入法线和切线，并从0～1还原到-1～1
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

			// 读入顶点数据
			VertexPosition = *(LPD3DXVECTOR3)pVertexSrc;
			pVertexSrc += (g_nEachPointSize / sizeof(float));

			// FVF有指定顺序，下面的数据顺序不能弄反
			// x y z，只改变y			
			*pVertexDst++ = VertexPosition.x + fDeltaHeightX;
			*pVertexDst++ = VertexPosition.y + fDeltaHeightY;
			pVertexDst++;
			// normal
			*pVertexDst++ = NormalData.x;
			*pVertexDst++ = NormalData.y;
			*pVertexDst++ = NormalData.z;
			// texcoord，跳过
			pVertexDst++;
			pVertexDst++;
			// tangent
			*pVertexDst++ = TangentData.x;
			*pVertexDst++ = TangentData.y;
			*pVertexDst++ = TangentData.z;
			// binormal，跳过
			pVertexDst++;
			pVertexDst++;
			pVertexDst++;
		}
	}


	// 结束
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



// 创建一个XZ平面，用多个小格代替，并分别指定两个方向上的格子数（X和Z）和总长宽（Width/Depth）
// 传入空指针即可，它会自动创建新的顶点/索引缓冲
// FVF默认为XYZ+Tex1+NORMAL，顶点是默认以原点为平面左上角，分别向X正向，Z负向延伸，所以需要将摄像机设高或低一点，才能看得出来该平面
// 用TriangleList画，Primitive数是2 * nXSplitNum * nZSplitNum，顶点数是(nXSplitNum+1) * (nZSplitNum+1)
HRESULT InitPlane(UINT nXSplitNum, float fWidth, UINT nZSplitNum, float fDepth, LPD3DXMESH &pMesh)
{
	if(nXSplitNum < 1 || nZSplitNum < 1 || fWidth <= 0 || fDepth <= 0)
		return D3DERR_INVALIDCALL;

	SAFE_RELEASE(pMesh);

	// 顶点数量
	UINT nX = nXSplitNum + 1;
	UINT nZ = nZSplitNum + 1;
	UINT nPointNum = nX * nZ;

	float fEachX = (float)fWidth / (float)nXSplitNum;
	float fEachZ = (float)fDepth / (float)nZSplitNum;

	UINT nIndexNum = nXSplitNum * nZSplitNum * 6;	// 每个格子需要两个三角形，即6个索引
	UINT nFaceNum = nXSplitNum * nZSplitNum * 2;
	UINT nEachIndexSize = nIndexNum>65535 ? sizeof(DWORD) : sizeof(WORD);
	D3DFORMAT IndexFormat = nIndexNum>65535 ? D3DFMT_INDEX32 : D3DFMT_INDEX16;

	// 创建缓冲区，VB必须可读
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

	// 填充数据
	UINT i = 0, j = 0;
	for(j = 0; j < nZ; j++)
	{
		for(i = 0; i < nX; i++)
		{
			// FVF有指定顺序，下面的数据顺序不能弄反
			// x y z
			*pVertexData++ = (float)i * fEachX;
			*pVertexData++ = 0.0f;
			*pVertexData++ = (float)j * -fEachZ;
			// normal，一律为0,1,0
			*pVertexData++ = 0.0f;
			*pVertexData++ = 1.0f;
			*pVertexData++ = 0.0f;
			// texcoord
			*pVertexData++ = (float)i / (float)nXSplitNum;
			*pVertexData++ = (float)j / (float)nZSplitNum;
			// tangent，一律为1,0,0
			*pVertexData++ = 1.0f;
			*pVertexData++ = 0.0f;
			*pVertexData++ = 0.0f;
			// binormal，一律为0,0,1
			*pVertexData++ = 0.0f;
			*pVertexData++ = 0.0f;
			*pVertexData++ = 1.0f;
		}
	}

	// DWORD型的
	if(IndexFormat == D3DFMT_INDEX32)
	{
		DWORD* pCurIndex = (DWORD *)pIndexData;
		// 每个方块的4个点，左上、右上、右下、左下
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
		// 每个方块的4个点，左上、右上、右下、左下
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

	// 顶点数量
	UINT nPointNum = nXPointNum * nZPointNum;

	// Lock VB & Tex
	float *pVertexHead = NULL, *pVertexRead = NULL, *pVertexWrite = NULL;
	D3DXVECTOR3 VertexPosition[9];	// in Source VB
	D3DXVECTOR2 VertexTexCoord[9];
	D3DXVECTOR3 D3DTangent;

	V_RETURN(g_pMeshCPU->LockVertexBuffer(0, (void **)&pVertexHead));
	pVertexWrite = pVertexHead;


	// 填充数据
	UINT i = 0, j = 0;
	for(j = 0; j < nZPointNum; j++)
	{
		for(i = 0; i < nXPointNum; i++)
		{
			// 读入指定点的高度和纹理坐标数据到VertexPosition/TexCoord中
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

			// 计算每个面的切线并累加
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


			// FVF有指定顺序，下面的数据顺序不能弄反
			// x y z，跳过
			pVertexWrite++;
			pVertexWrite++;
			pVertexWrite++;
			// normal，跳过
			pVertexWrite++;
			pVertexWrite++;
			pVertexWrite++;
			// texcoord，跳过
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


	// 结束
	g_pMeshCPU->UnlockVertexBuffer();

	return S_OK;
}