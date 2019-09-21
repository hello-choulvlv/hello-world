/*
  *海水的渲染
  *此文件为工程的核心
  *关于海水渲染的物理模型在OceanSimulate.cpp文件中
  *2018年4月5日
  *@author:xiaohuaxiong
 */
#include "common.h"
#include "render.h"
#include "Camera.h"
#include<d3dx11.h>
#include<d3d11.h>
#include<assert.h>
#include<vector>
////////////////////////////////
///关于网格维度的信息/////////
const    int			g_MeshSize = 128;
const    int         g_DisplacementDim = 512;
const    float		g_PatchSize = 2000;//每一个网格块的大小
//Perlin噪声
const    float                            g_PerlinSize = 1.0f;
const    float                            g_PerlinSpeed = 0.06f;
const    D3DXVECTOR3      g_PerlinAmplitude(35, 42, 57);
const    D3DXVECTOR3      g_PerlinGradient(1.4f, 1.6f, 2.2f);
const    D3DXVECTOR3      g_PerlinOctave(1.12f, 0.59f, 0.23f);
//风的方向
const    D3DXVECTOR2      g_WindDir(0.8f,0.6f);
//光线
const   D3DXVECTOR3       g_SkyColor(0.38f, 0.45f, 0.56f);
const    D3DXVECTOR3	  g_WaterbodyColor(0.07f, 0.15f, 0.2f);
const    D3DXVECTOR3      g_BlendParam(0.1f, -0.4f, 0.2f);
const    D3DXVECTOR3      g_SunDir(0.936016f, -0.343206f, 0.0780013f);
const    D3DXVECTOR3      g_SunColor(1.0f, 1.0f, 0.6f);
const    float                            g_Sunshine = 400;
const    float                            g_SkyBlending = 16.0f;
///////
const    int                               g_FurthestCoverage = 9;
const    int                               g_Lod = 7;
const    float                            g_UpperGridCoverage = 100.0f;
//////////////////////////DirectX11/////////////////////////////////
ID3D11ShaderResourceView *s_ShaderViewFreshnel=nullptr;
ID3D11ShaderResourceView *s_ShaderViewPerlin = nullptr;
ID3D11ShaderResourceView *s_ShaderViewReflectCube = nullptr;
ID3D11SamplerState  *s_SamplerPerlin=nullptr;
ID3D11SamplerState  *s_SamplerFreshnel = nullptr;
ID3D11SamplerState  *s_SamplerDisplacement=nullptr;
ID3D11SamplerState  *s_SamplerGradient = nullptr;
ID3D11SamplerState  *s_SamplerCube = nullptr;
/////光栅化对象
ID3D11RasterizerState *s_RasterizerSolid=nullptr;
ID3D11RasterizerState  *s_RasterizerWire = nullptr;
//深度模版测试
ID3D11DepthStencilState  *s_DepthStencilDisable = nullptr;
//颜色混溶对象
ID3D11BlendState               *s_BlendTransparent = nullptr;
//constant buffer/////
ID3D11Buffer               *s_VertexBuffer=nullptr;
ID3D11Buffer               *s_IndexBuffer = nullptr;
ID3D11Buffer               *s_ConstantBuffer = nullptr;
ID3D11Buffer               *s_PerFrameBuffer = nullptr;
//////shader////
ID3D11VertexShader  *s_VertexShader = nullptr;
ID3D11PixelShader     *s_PixelShader = nullptr;
ID3D11InputLayout    *s_LayoutInput = nullptr;
////////全局数据结构
struct   ConstantBuffer
{
	D3DXVECTOR3        g_SkyColor;
	float							  g_TexelLength_x2;
	D3DXVECTOR3        g_WaterbodyColor;
	float							  g_UVScale;
	D3DXVECTOR3        g_PerlinAmplitude;
	float							  g_UVOffset;
	D3DXVECTOR3        g_PerlinOctave;
	float							  g_PerlinSize;
	D3DXVECTOR3        g_BlendParam;
	float							  g_Sunshine;
	D3DXVECTOR3       g_PerlinGradient;
	float							 padding0;
	D3DXVECTOR3       g_SunDir;
	float							 g_padding1;
	D3DXVECTOR3      g_SunColor;
	float							padding1;
};

struct cbPerFrameBuffer
{
	D3DXMATRIX    g_ViewProjMatrix;
	D3DXMATRIX    g_LocalMatrix;
	D3DXVECTOR3  g_EyeLocation;
	float					    unuse;
	D3DXVECTOR2  g_UVBase;
	D3DXVECTOR2  g_PerlinMovement;
};
//渲染无限网格区域需要用到的数据结构
struct RenderPatternParam
{
	//中心三角形的信息
	int          vertexCount;//顶点的数目
	int          vertexOffset;//顶点在索引缓冲区中的偏移量
	int          triangleFaceCount;//三角形的数目

	int          boundaryVertexCount;//边界顶点的数目
	int          boundaryOffset;//边界顶点的偏移量
	int          boundaryFaceCount;//边界三角形的数目,三角形的实际数目为 boundaryFaceCount + 2
};
//四叉树
struct   QuadTree
{
	D3DXVECTOR2     left_bottom;
	float                           length;
	int                              lod;
	int                              child[4];
};
//全局无限网格模型
//g_RenderPattern[level][l][r][b][t] 代表的含义为目标网格区域所处的位置为
//lod = level,且左右下上的邻接网格的长度为目标网格的2的l/r/b/t倍
static RenderPatternParam        g_RenderPattern[7][3][3][3][3];
static std::vector<QuadTree>    g_RenderTree;
///////////////////////////////////////////////////////////////
void     createSurface();//创建海平面的表面
//创建某一个网格区域的中心表面
int     createCenterSurface(const RECT &rect,int meshSize,int *index_array);
//创建某一个区域网格的边界
int     createBoundarySurface(const RECT &rect,int meshSize,int left_degree,int right_degree,int bottom_degree,int top_gree,int *index_array);
//四叉树遍历
bool     isLeaf(QuadTree   &node);
int        buildQuadTree(QuadTree  &root,Camera &camera,int *visibility);
//
void     initRenderResource()
{
	initShaderView();
	initBuffer();
	initSamplerState();
	initFreshnelMap();
	createSurface();
	//load Shader
	ID3D10Blob    *error_blob = nullptr;
	ID3D10Blob    *vertex_blob = nullptr;
	ID3D10Blob    *pixel_blob = nullptr;
	//
	unsigned shaderCompileFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderCompileFlag |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	int result = D3DX11CompileFromFile(L"shader/ocean_shader.hlsl",nullptr,nullptr,"vs_main","vs_5_0", 
		shaderCompileFlag,0,nullptr,&vertex_blob,&error_blob,nullptr);
	if (result < 0)
	{
		if (error_blob)
		{
			MessageBoxA(nullptr,static_cast<char*>(error_blob->GetBufferPointer()),nullptr,0);
			error_blob->Release();
		}
	}
	result = D3DX11CompileFromFile(L"shader/ocean_shader.hlsl",nullptr,nullptr,"ps_main","ps_5_0",
		shaderCompileFlag,0,nullptr,&pixel_blob,&error_blob,nullptr);
	if (result < 0)
	{
		if (error_blob)
		{
			MessageBoxA(nullptr,static_cast<char*>(error_blob->GetBufferPointer()),nullptr,0);
			error_blob->Release();
		}
	}
	result = s_device->CreateVertexShader(vertex_blob->GetBufferPointer(), vertex_blob->GetBufferSize(), nullptr, &s_VertexShader);
	assert(result>=0);

	result = s_device->CreatePixelShader(pixel_blob->GetBufferPointer(), pixel_blob->GetBufferSize(), nullptr, &s_PixelShader);
	assert(result>=0);

	D3D11_INPUT_ELEMENT_DESC    layoutDesc;
	layoutDesc.SemanticName = "POSITION";
	layoutDesc.SemanticIndex = 0;
	layoutDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	layoutDesc.InputSlot = 0;
	layoutDesc.AlignedByteOffset = 0;
	layoutDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layoutDesc.InstanceDataStepRate = 0;
	result = s_device->CreateInputLayout(&layoutDesc, 1, vertex_blob->GetBufferPointer(), vertex_blob->GetBufferSize(), &s_LayoutInput);
	assert(result>=0);

	vertex_blob->Release();
	pixel_blob->Release();
}

void     initBuffer()
{
	float    *vertex_data = new float[(g_MeshSize+1)*(g_MeshSize+1)*2];
	int       index = 0;
	//生成顶点
	for (int i = 0; i < g_MeshSize + 1; ++i)//纵向
	{
		for (int j = 0; j < g_MeshSize + 1; ++j)//横向
		{
			vertex_data[index] = 1.0f*j;
			vertex_data[index + 1] = 1.0f*i;
			index += 2;
		}
	}
	D3D11_BUFFER_DESC   bufferDesc;
	bufferDesc.ByteWidth = 2*sizeof(float)*(g_MeshSize+1)*(g_MeshSize+1);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA  resData;
	resData.pSysMem = vertex_data;
	int result = s_device->CreateBuffer(&bufferDesc, &resData, &s_VertexBuffer);
	assert(result>=0);
	delete[] vertex_data;
	vertex_data = nullptr;
	////创建一个临时索引缓冲区对象,待程序稳定之后,我们将会实现完整的索引缓冲区对象
	//int   *index_data = new int[g_MeshSize*g_MeshSize*6];
	//index = 0;
	//for (int i = 0; i < g_MeshSize; ++i)
	//{
	//	for (int j = 0; j < g_MeshSize; ++j)
	//	{
	//		index_data[index] = i*(g_MeshSize+1)+j;
	//		index_data[index + 1] = (i + 1)*(g_MeshSize + 1) + j;
	//		index_data[index + 2] = (i+1)*(g_MeshSize+1)+(j+1);

	//		index_data[index + 3] = (i+1)*(g_MeshSize+1)+(j+1);
	//		index_data[index + 4] = i*(g_MeshSize + 1) + j + 1;
	//		index_data[index + 5] = i*(g_MeshSize+1) + j;

	//		index += 6;
	//	}
	//}
	//bufferDesc.ByteWidth = 6*sizeof(int)*g_MeshSize*g_MeshSize;
	//bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	//bufferDesc.CPUAccessFlags = 0;
	//bufferDesc.MiscFlags = 0;
	//bufferDesc.StructureByteStride = 0;

	//resData.pSysMem = index_data;
	//result = s_device->CreateBuffer(&bufferDesc, &resData, &s_IndexBuffer);
	//assert(result>=0);
	//////Constant Buffer/////
	ConstantBuffer      per_call_buffer;
	per_call_buffer.g_SkyColor = g_SkyColor;
	per_call_buffer.g_TexelLength_x2 = g_PatchSize / g_DisplacementDim * 2.0f;
	per_call_buffer.g_WaterbodyColor = g_WaterbodyColor;
	per_call_buffer.g_UVScale = 1.0f / g_PatchSize;
	per_call_buffer.g_PerlinAmplitude = g_PerlinAmplitude;
	per_call_buffer.g_UVOffset = 1.0f / g_DisplacementDim;
	per_call_buffer.g_PerlinOctave = g_PerlinOctave;
	per_call_buffer.g_PerlinSize = g_PerlinSize;
	per_call_buffer.g_BlendParam = g_BlendParam;
	per_call_buffer.g_Sunshine = g_Sunshine;
	per_call_buffer.g_PerlinGradient = g_PerlinGradient;
	per_call_buffer.g_SunDir = g_SunDir;
	per_call_buffer.g_SunColor = g_SunColor;

	bufferDesc.ByteWidth = sizeof(per_call_buffer);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.MiscFlags = 0;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.StructureByteStride = 0;

	resData.pSysMem = &per_call_buffer;
	result = s_device->CreateBuffer(&bufferDesc, &resData, &s_ConstantBuffer);
	assert(result>=0);
	//
	bufferDesc.ByteWidth = sizeof(cbPerFrameBuffer);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.MiscFlags = 0;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.StructureByteStride = 0;
	result = s_device->CreateBuffer(&bufferDesc, nullptr, &s_PerFrameBuffer);
	assert(result>=0);
}

void    initSamplerState()
{
	D3D11_SAMPLER_DESC   samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = FLT_MAX;
	int result = s_device->CreateSamplerState(&samplerDesc, &s_SamplerDisplacement);
	assert(result>=0);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	result = s_device->CreateSamplerState(&samplerDesc, &s_SamplerCube);
	assert(result>=0);

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 8;
	result = s_device->CreateSamplerState(&samplerDesc, &s_SamplerGradient);
	assert(result>=0);

	samplerDesc.MaxAnisotropy = 4;
	result = s_device->CreateSamplerState(&samplerDesc, &s_SamplerPerlin);
	assert(result>=0);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	result = s_device->CreateSamplerState(&samplerDesc, &s_SamplerFreshnel);
	assert(result>=0);
	//光栅化对象
	D3D11_RASTERIZER_DESC     rasterDesc;
	memset(&rasterDesc,0,sizeof(rasterDesc));
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.MultisampleEnable = TRUE;
	rasterDesc.AntialiasedLineEnable = FALSE;
	result = s_device->CreateRasterizerState(&rasterDesc, &s_RasterizerSolid);
	assert(result>=0);
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	result = s_device->CreateRasterizerState(&rasterDesc, &s_RasterizerWire);
	assert(result>=0);
	//深度模版测试对象
	D3D11_DEPTH_STENCIL_DESC   depthStencilDesc;
	memset(&depthStencilDesc, 0, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.StencilEnable = FALSE;
	result = s_device->CreateDepthStencilState(&depthStencilDesc, &s_DepthStencilDisable);
	assert(result>=0);
	//颜色混溶对象
	D3D11_BLEND_DESC  blendDesc;
	memset(&blendDesc, 0, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	result = s_device->CreateBlendState(&blendDesc, &s_BlendTransparent);
	assert(result>=0);
}

void    initFreshnelMap()
{
	unsigned    freshnelMap[256];
	for (int i = 0; i < 256; ++i)
	{
		//在给定的角度下,求反射率
		float			cos_v = i / 256.0f;
		unsigned    freshnel = D3DXFresnelTerm(cos_v,1.33f)*255;
		unsigned    sky_blend = powf(1.0f / (1.0f + cos_v), g_SkyBlending) * 255;
		freshnelMap[i] = freshnel | (sky_blend<<8);
	}
	D3D11_TEXTURE1D_DESC   fresnelDesc;
	fresnelDesc.Width = 256;
	fresnelDesc.MipLevels = 1;
	fresnelDesc.ArraySize = 1;
	fresnelDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	fresnelDesc.Usage = D3D11_USAGE_IMMUTABLE;
	fresnelDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	fresnelDesc.CPUAccessFlags = 0;
	fresnelDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA  resData;
	resData.pSysMem = freshnelMap;
	resData.SysMemPitch = 0;
	resData.SysMemSlicePitch = 0;

	ID3D11Texture1D    *textureFresnel = nullptr;
	int result = s_device->CreateTexture1D(&fresnelDesc, &resData,&textureFresnel);
	assert(result>=0);

	D3D11_SHADER_RESOURCE_VIEW_DESC   shaderViewDesc;
	shaderViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	shaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	shaderViewDesc.Texture1D.MipLevels = 1;
	shaderViewDesc.Texture1D.MostDetailedMip = 0;
	result = s_device->CreateShaderResourceView(textureFresnel, &shaderViewDesc, &s_ShaderViewFreshnel);
	textureFresnel->Release();
	assert(result>=0);
}

void    initShaderView()
{
	int result = D3DX11CreateShaderResourceViewFromFile(s_device, L"media/reflect_cube.dds",nullptr,nullptr,&s_ShaderViewReflectCube,nullptr);
	assert(result>=0);
	result = D3DX11CreateShaderResourceViewFromFile(s_device,L"media/perlin_noise.dds",nullptr,nullptr,&s_ShaderViewPerlin,nullptr);
	assert(result>=0);
}

void destroyRenderResource()
{
	s_ShaderViewFreshnel->Release();
	s_ShaderViewPerlin->Release();
	s_ShaderViewReflectCube->Release();
	s_SamplerPerlin->Release();
	s_SamplerFreshnel->Release();
	s_SamplerDisplacement->Release();
	s_SamplerGradient->Release();
	s_SamplerCube->Release();

	s_RasterizerSolid->Release();
	s_RasterizerWire->Release();
	s_DepthStencilDisable->Release();
	s_BlendTransparent->Release();

	s_VertexBuffer->Release();
	s_IndexBuffer->Release();
	s_ConstantBuffer->Release();
	s_PerFrameBuffer->Release();

	s_VertexShader->Release();
	s_PixelShader->Release();
	s_LayoutInput->Release();
}

void   createSurface()
{
	int     *index_array = new int[3689820];
	int       offset = 0;
	int      level_size = g_MeshSize;
	memset(g_RenderPattern,0,sizeof(g_RenderPattern));
	for (int level = 0; level <= g_Lod - 2; ++level)
	{
		int    left_level = level_size;
		for (int left_type = 0; left_type < 3; ++left_type)
		{
			int right_level = level_size;
			for (int right_type = 0; right_type < 3; ++right_type)
			{
				int bottom_level = level_size;
				for (int bottom_type = 0; bottom_type < 3; ++bottom_type)
				{
					int top_level = level_size;
					for (int top_type = 0; top_type < 3; ++top_type)
					{
						RenderPatternParam    &param = g_RenderPattern[level][left_type][right_type][bottom_type][top_type];
						RECT    innerRect;
						innerRect.left = (left_level == level_size)?0:1;
						innerRect.right = (right_level == level_size)?level_size:level_size-1;
						innerRect.bottom = (bottom_level==level_size)?0:1;
						innerRect.top = (top_level == level_size)?level_size:level_size-1;
						int   index_count = createCenterSurface(innerRect,g_MeshSize, index_array+offset);
						param.vertexCount = index_count;
						param.vertexOffset = offset;
						param.triangleFaceCount = index_count -2;
						offset += index_count;
						/////////////////////////////Boundary-Triangle///////////////
						RECT   boundaryRect;
						int  l_degree = (left_level == level_size)?0:left_level;
						int  r_degree = (right_level == level_size)?0:right_level;
						int  b_degree = (bottom_level == level_size)?0:bottom_level;
						int  t_degree = (top_level == level_size)?0:top_level;
						boundaryRect.left = 0;
						boundaryRect.right = level_size;
						boundaryRect.bottom = 0;
						boundaryRect.top = level_size;
						index_count = createBoundarySurface(boundaryRect, g_MeshSize, l_degree, r_degree, 
																								b_degree, t_degree, index_array + offset);
						param.boundaryVertexCount = index_count;
						param.boundaryOffset = offset;
						param.boundaryFaceCount = index_count /3;
						offset += index_count;
						/////////////////////////////////////////////////////////
						top_level /= 2;
					}
					bottom_level /= 2;
				}
				right_level /= 2;
			}
			left_level /= 2;
		}
		level_size /= 2;
	}
	//index buffer
	D3D11_BUFFER_DESC  bufferDesc;
	bufferDesc.ByteWidth = sizeof(int)*offset;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA  bufferData;
	bufferData.pSysMem = index_array;
	bufferData.SysMemPitch = 0;
	bufferData.SysMemSlicePitch = 0;
	int result = s_device->CreateBuffer(&bufferDesc,&bufferData,&s_IndexBuffer);
	assert(result>=0);
	delete[] index_array;
	index_array = nullptr;
}
//需要使用一些宏
#define MESH_INDEX(x,y)      ( (rect.bottom + (y)) * (meshSize+1) + (rect.left + (x)) )
int    createCenterSurface(const RECT &rect, int meshSize, int *index_array)
{
	int   index = 0;
	int   width  = rect.right - rect.left;
	int   height = rect.top - rect.bottom;
	bool    inverse = false;
	//使用三角形带算法
	for (int i = 0; i < height; ++i)
	{
		if (!inverse)
		{
			index_array[index] = MESH_INDEX(0,i);
			index_array[index + 1] = MESH_INDEX(0,i+1);
			index += 2;
			for (int j = 0; j < width; ++j)
			{
				index_array[index] = MESH_INDEX(j+1,i);
				index_array[index + 1] = MESH_INDEX(j+1,i+1);
				index += 2;
			}
		}
		else
		{
			index_array[index] = MESH_INDEX(width,i);
			index_array[index + 1] = MESH_INDEX(width,i+1);
			index += 2;
			for (int j = width; j > 0 ;--j)
			{
				index_array[index] = MESH_INDEX(j-1,i);
				index_array[index+1] = MESH_INDEX(j-1,i+1);
				index += 2;
			}
		}
		inverse = !inverse;
	}
	return index;
}

int createBoundarySurface(const RECT &rect, int meshSize, int left_degree, int right_degree, int bottom_degree, int top_degree, int *index_array)
{
	int  index = 0;
	const int  width = rect.right - rect.left;
	const int  height = rect.top - rect.bottom;
	//如果下侧有过渡网格
	if (bottom_degree > 0)
	{
		int   b_step = width / bottom_degree;//建立网格时的跨度
		for (int j = 0; j < width; j+= b_step)
		{
			index_array[index] = MESH_INDEX(j,0);
			index_array[index + 1] = MESH_INDEX(j+b_step/2,1);
			index_array[index + 2] = MESH_INDEX(j+b_step,0);
			index += 3;
			//左侧的互补网格
			for (int s = 0; s < b_step / 2; ++s)
			{
				if (!j && !s && left_degree > 0)
					continue;
				index_array[index] = MESH_INDEX(j,0);
				index_array[index + 1] = MESH_INDEX(j+s,1);
				index_array[index + 2] = MESH_INDEX(j+s+1,1);
				index += 3;
			}
			for (int s = b_step / 2; s < b_step; ++s)
			{
				if (j == width - b_step && s == b_step - 1 && right_degree > 0)
					continue;
				index_array[index] = MESH_INDEX(j+b_step,0);
				index_array[index + 1] = MESH_INDEX(j+s,1);
				index_array[index + 2] = MESH_INDEX(j+s+1,1);
				index += 3;
			}
		}
	}
	//上侧
	if (top_degree > 0)
	{
		int t_step = width / top_degree;
		for (int j = 0; j < width; j += t_step)
		{
			index_array[index] = MESH_INDEX(j,height);
			index_array[index + 1] = MESH_INDEX(j+t_step/2,height-1);
			index_array[index + 2] = MESH_INDEX(j+t_step,height);
			index += 3;
			//左侧
			for (int s = 0; s < t_step / 2; ++s)
			{
				//剔除掉公共的三角形
				if (!j && !s && left_degree > 0)
					continue;
				index_array[index] = MESH_INDEX(j,height);
				index_array[index + 1] = MESH_INDEX(j+s,height-1);
				index_array[index + 2] = MESH_INDEX(j+s+1,height-1);
				index += 3;
			}
			//右侧三角形扇
			for (int s = t_step / 2; s < t_step; ++s)
			{
				if (j == width - t_step && s == t_step - 1 && right_degree > 0)
					continue;
				index_array[index] = MESH_INDEX(j+t_step,height);
				index_array[index + 1] = MESH_INDEX(j+s,height-1);
				index_array[index + 2] = MESH_INDEX(j+s+1,height-1);
				index += 3;
			}
		}
	}
	//左侧的三角形序列
	if (left_degree > 0)
	{
		int  l_step = height / left_degree;
		for (int j = 0; j < height; j += l_step)
		{
			index_array[index] = MESH_INDEX(0,j);
			index_array[index + 1] = MESH_INDEX(1,j+l_step/2);
			index_array[index + 2] = MESH_INDEX(0,j+ l_step);
			index += 3;
			//下侧的三角形序列
			for (int s = 0; s < l_step / 2; ++s)
			{
				if (!j && !s && bottom_degree > 0)
					continue;
				index_array[index] = MESH_INDEX(0,j);
				index_array[index + 1] = MESH_INDEX(1,j+s);
				index_array[index + 2] = MESH_INDEX(1,j+s+1);
				index += 3;
			}
			//上侧的三角形序列
			for (int s = l_step / 2; s < l_step; ++s)
			{
				if (j == height - l_step && s == l_step - 1 && top_degree > 0)
					continue;
				index_array[index] = MESH_INDEX(0,j+l_step);
				index_array[index + 1] = MESH_INDEX(1,j+ s);
				index_array[index + 2] = MESH_INDEX(1,j+s+1);
				index += 3;
			}
		}
	}
	//右侧的三角形序列
	if (right_degree > 0)
	{
		const int r_step = height / right_degree;
		for (int j = 0; j < height; j += r_step)
		{
			index_array[index] = MESH_INDEX(width,j);
			index_array[index + 1] = MESH_INDEX(width-1,j+r_step/2);
			index_array[index + 2] = MESH_INDEX(width,j+r_step);
			index += 3;
			//下侧的三角形序列
			for (int s = 0; s < r_step / 2; ++s)
			{
				if (!j && !s && bottom_degree > 0)
					continue;
				index_array[index] = MESH_INDEX(width,j);
				index_array[index + 1] = MESH_INDEX(width-1,j+s);
				index_array[index + 2] = MESH_INDEX(width-1,j+s+1);
				index += 3;
			}
			//上侧的三角形序列
			for (int s = r_step / 2; s < r_step; ++s)
			{
				if (j == height - r_step && s == r_step - 1 && top_degree > 0)
					continue;
				index_array[index] = MESH_INDEX(width,j+r_step);
				index_array[index + 1] = MESH_INDEX(width-1,j+ s);
				index_array[index + 2] = MESH_INDEX(width-1,j+s+1);
				index += 3;
			}
		}
	}
	return index;
}
//检测节点的可见性
bool   checkNodeVisibility(const QuadTree &node,Camera &camera,int *vilibility)
{
	*vilibility = 0;
	const D3DXMATRIX   matView = s_reflect_matrix * camera.getViewMatrix();
	//节点的四个顶点
	D3DXVECTOR3  quad_vertex[4] = {
		D3DXVECTOR3(node.left_bottom.x,node.left_bottom.y,0),
		D3DXVECTOR3(node.left_bottom.x+node.length,node.left_bottom.y,0),
		D3DXVECTOR3(node.left_bottom.x+node.length,node.left_bottom.y+node.length,0),
		D3DXVECTOR3(node.left_bottom.x,node.left_bottom.y + node.length,0),
	};
	//变换到摄像机空间
	for (int k = 0; k < 4; ++k)
		D3DXVec3TransformCoord(quad_vertex + k, quad_vertex + k, &matView);
	//初步检测是否在摄像机之后
	if (quad_vertex[0].z < 0 && quad_vertex[1].z < 0 && quad_vertex[2].z < 0 && quad_vertex[3].z < 0)
		return false;
	//视锥体裁剪
	const float *matrix_array = reinterpret_cast<const float*>(&camera.getProjMatrix());
	//视锥体左侧平面裁剪
	float   f = 1.0f/sqrtf(1 + matrix_array[0] * matrix_array[0]);
	D3DXVECTOR3     l_plane(f *matrix_array[0],0,f);
	float  distance[4];
	for (int k = 0; k < 4; ++k)
		distance[k] = l_plane.x *quad_vertex[k].x + l_plane.y *quad_vertex[k].y + l_plane.z * quad_vertex[k].z;
	if (distance[0] < 0 && distance[1] < 0 && distance[2] < 0 && distance[3] < 0)
		return false;
	//右侧的平面
	D3DXVECTOR3     r_plane(-f*matrix_array[0],0,f);
	for (int k = 0; k < 4; ++k)
		distance[k] = r_plane.x * quad_vertex[k].x + r_plane.y * quad_vertex[k].y + r_plane.z* quad_vertex[k].z;
	if (distance[0] < 0 && distance[1] < 0 && distance[2] < 0 && distance[3] < 0)
		return false;
	//下侧的平面裁剪
	f = 1.0f/sqrtf(1+matrix_array[5]*matrix_array[5]);
	D3DXVECTOR3   b_plane(0,f*matrix_array[5],f);
	for (int k = 0; k < 4; ++k)
		distance[k] = b_plane.x*quad_vertex[k].x + b_plane.y*quad_vertex[k].y + b_plane.z*quad_vertex[k].z;
	if (distance[0] < 0 && distance[1] < 0 && distance[2] < 0 && distance[3] < 0)
		return false;
	//上侧的平面
	D3DXVECTOR3   t_plane(0,-f*matrix_array[5],f);
	for (int k = 0; k < 4; ++k)
		distance[k] = t_plane.x*quad_vertex[k].x + t_plane.y*quad_vertex[k].y + t_plane.z*quad_vertex[k].z;
	if (distance[0] < 0 && distance[1] < 0 && distance[2] < 0 && distance[3] < 0)
		return false;
	*vilibility = 1;
	return true;
}
bool   isLeaf(QuadTree &node)
{
	const int *child = node.child;
	return child[0] == -1 && child[1] == -1 && child[2] == -1 && child[3] == -1;
}
//查找最佳目标叶子节点
int     searchLeaf(std::vector<QuadTree> &treeVec,D3DXVECTOR2  &point)
{
	QuadTree   *node = &treeVec.back();
	bool    found = false;
	int      select_index = -1;
	while (!isLeaf(*node))
	{
		found = false;
		for (int k = 0; k < 4; ++k)
		{
			select_index = node->child[k];
			if (select_index == -1)
				continue;
			QuadTree  *secondary = &treeVec[select_index];
			const D3DXVECTOR2 &boundary = secondary->left_bottom;
			const float length = secondary->length;
			if (boundary.x <= point.x && boundary.x + length >= point.x &&
				boundary.y <= point.y && boundary.y + length >= point.y)
			{
				node = secondary;
				found = true;
				break;
			}
		}
		if (!found)
			return -1;
	}
	return select_index;
}
//评估目标子空间区域的网格的屏幕投影面积,最后得出的数值并不代表着真实的屏幕面积
float  estimateGridCoverage(const QuadTree &child,Camera &camera,float  screen_area)
{
	const    D3DXMATRIX  &matView =  camera.getViewMatrix();
	// Test 16 points on the quad and find out the biggest one.
	const static float sample_pos[16][2] =
	{
		{ 0, 0 },
		{ 0, 1 },
		{ 1, 0 },
		{ 1, 1 },
		{ 0.5f, 0.333f },
		{ 0.25f, 0.667f },
		{ 0.75f, 0.111f },
		{ 0.125f, 0.444f },
		{ 0.625f, 0.778f },
		{ 0.375f, 0.222f },
		{ 0.875f, 0.556f },
		{ 0.0625f, 0.889f },
		{ 0.5625f, 0.037f },
		{ 0.3125f, 0.37f },
		{ 0.8125f, 0.704f },
		{ 0.1875f, 0.148f },
	};
	D3DXVECTOR3     eye_location = camera.getEyePosition();
	eye_location = D3DXVECTOR3(eye_location.x,eye_location.z,eye_location.y);
	const float *matrix_array = reinterpret_cast<const float*>(&camera.getProjMatrix());
	float   grid_length_world = child.length / g_MeshSize;
	float   grid_area = grid_length_world * matrix_array[0] * grid_length_world * matrix_array[5];
	float   max_area = 0;
	for (int k = 0; k < 16; ++k)
	{
		D3DXVECTOR3    target_location(child.left_bottom.x + child.length*sample_pos[k][0],
																		child.left_bottom.y + child.length*sample_pos[k][1],0);
		D3DXVECTOR3    interpolation = eye_location - target_location;
		float    proj_area = grid_area / (interpolation.x*interpolation.x+ interpolation.y*interpolation.y+ interpolation.z*interpolation.z);
		if (proj_area > max_area)
			max_area = proj_area;
	}
	return max_area * screen_area *0.25f;
}
//建立四叉树
int      buildQuadTree(QuadTree  &root,Camera &camera,int *visibility)
{
	//可见性检测
	if (!checkNodeVisibility(root, camera, visibility))
		return -1;
	//确定大概的投影面积
	float    proj_area = estimateGridCoverage(root, camera, s_window_width*s_window_height);
	int       visible = true;
	int       child_visibility[4];
	//如果投影面积大于给定的上界,并且子空间的跨度大于单位Patch,则开始分裂子空间
	if (proj_area > g_UpperGridCoverage && root.length > g_PatchSize)
	{
		auto &lb_point = root.left_bottom;
		float  half_length = root.length * 0.5f;
		//left bottom
		QuadTree   left_bottom_node = {lb_point,half_length,0,-1,-1,-1,-1};
		root.child[0] = buildQuadTree(left_bottom_node, camera, child_visibility);
		//right bottom
		QuadTree right_bottom_node = {D3DXVECTOR2(lb_point.x+half_length,lb_point.y),half_length,0,-1,-1,-1,-1};
		root.child[1] = buildQuadTree(right_bottom_node, camera, child_visibility + 1);
		//right top
		QuadTree right_top_node = {D3DXVECTOR2(lb_point.x+half_length,lb_point.y+half_length),half_length,0,-1,-1,-1,-1};
		root.child[2] = buildQuadTree(right_top_node, camera, child_visibility + 2);
		//right left
		QuadTree left_top_node = {D3DXVECTOR2(lb_point.x,lb_point.y+half_length),half_length,0,-1,-1,-1,-1};
		root.child[3] = buildQuadTree(left_top_node, camera, child_visibility + 3);
		visible = !isLeaf(root);
	}
	//计算lod
	if (visible)
	{
		int  k = 0;
		for (; k < g_Lod - 2; ++k)
		{
			if (proj_area > g_UpperGridCoverage)
				break;
			proj_area *= 4;
		}
		root.lod = min(k, g_Lod - 2);
	}
	else
		return -1;
	//将可以渲染的四叉树子空间加入到渲染队列中
	g_RenderTree.push_back(root);
	return g_RenderTree.size() - 1;
}
//选择与给定的四叉树节点相对应的渲染模型
RenderPatternParam  &selectRenderPattern(std::vector<QuadTree> &root,QuadTree &node)
{
	D3DXVECTOR2    left_adj_point = { node.left_bottom.x - g_PatchSize*0.5f,node.left_bottom.y+ node.length*0.5f};
	int   left_select_index = searchLeaf(root, left_adj_point);

	D3DXVECTOR2    right_adj_point = {node.left_bottom.x + node.length + g_PatchSize*0.5f,node.left_bottom.y + node.length *0.5f};
	int   right_select_index = searchLeaf(root, right_adj_point);

	D3DXVECTOR2    bottom_adj_point = { node.left_bottom.x + node.length*0.5f,node.left_bottom.y - g_PatchSize*0.5f };
	int   bottom_select_index = searchLeaf(root, bottom_adj_point);

	D3DXVECTOR2    top_adj_point = { node.left_bottom.x + node.length*0.5f,node.left_bottom.y + node.length + g_PatchSize*0.5f };
	int   top_select_index = searchLeaf(root, top_adj_point);
	//选定
	int   left_type = 0;
	if (left_select_index != -1 && root[left_select_index].length > node.length*0.999f)
	{
		const QuadTree & adj_node = root[left_select_index];
		float   scale = adj_node.length *(1 << adj_node.lod) / (node.length*(1<<node.lod));
		//float   scale = adj_node.length / node.length * (g_MeshSize >> node.lod) / (g_MeshSize >> adj_node.lod);
		if (scale > 3.999f)
			left_type = 2;
		else if (scale > 1.999f)
			left_type = 1;
	}
	int    right_type = 0;
	if (right_select_index != -1 && root[right_select_index].length > node.length*0.999f)
	{
		const QuadTree &adj_node = root[right_select_index];
		float   scale = adj_node.length *(1<<adj_node.lod)/(node.length*(1<<node.lod));
		//float   scale = adj_node.length / node.length * (g_MeshSize >> node.lod) / (g_MeshSize >> adj_node.lod);
		if (scale > 3.999f)
			right_type = 2;
		else if (scale > 1.999f)
			right_type = 1;
	}
	int  bottom_type = 0;
	if (bottom_select_index != -1 && root[bottom_select_index].length > node.length*0.999f)
	{
		const QuadTree &adj_node = root[bottom_select_index];
		float scale = adj_node.length*(1 << adj_node.lod) / (node.length*(1<<node.lod));
		//float   scale = adj_node.length / node.length * (g_MeshSize >> node.lod) / (g_MeshSize >> adj_node.lod);
		if (scale > 3.999f)
			bottom_type = 2;
		else if (scale > 1.999f)
			bottom_type = 1;
	}
	int top_type = 0;
	if (top_select_index != -1 && root[top_select_index].length > node.length *0.999f)
	{
		const QuadTree &adj_node = root[top_select_index];
		float scale = adj_node.length*(1<<adj_node.lod)/(node.length*(1<<node.lod));
		//float   scale = adj_node.length / node.length * (g_MeshSize >> node.lod) / (g_MeshSize >> adj_node.lod);
		if (scale > 3.999f)
			top_type = 2;
		else if (scale > 1.999f)
			top_type = 1;
	}
	return g_RenderPattern[node.lod][left_type][right_type][bottom_type][top_type];
}
static bool s_isFirst = true;
void    renderOceanMeshShader(Camera &camera, ID3D11ShaderResourceView *shaderViewDisplacement, ID3D11ShaderResourceView *shaderViewGradient,float time)
{
	//if(s_isFirst)
	g_RenderTree.clear();
	float   ocean_extent = g_PatchSize * (1<<g_FurthestCoverage);
	QuadTree    root = {D3DXVECTOR2(-0.5f*ocean_extent,-0.5f*ocean_extent),ocean_extent,0,-1,-1,-1,-1};
	int                 visibility[4];
	//if(s_isFirst)
	buildQuadTree(root, camera,visibility);
	//s_isFirst = false;
	//建立四叉树
	//渲染前的预备工作
	s_deviceContext->VSSetShader(s_VertexShader,nullptr,0);
	s_deviceContext->PSSetShader(s_PixelShader,nullptr,0);
	//vertex
	ID3D11SamplerState  *vertex_sampler[2] = {s_SamplerDisplacement,s_SamplerPerlin};
	s_deviceContext->VSSetSamplers(0, 2, vertex_sampler);
	ID3D11ShaderResourceView  *vertex_shader_view[2] = {shaderViewDisplacement,s_ShaderViewPerlin};
	s_deviceContext->VSSetShaderResources(0, 2, vertex_shader_view);
	ID3D11Buffer     *vertex_buffer[2] = {s_ConstantBuffer,s_PerFrameBuffer};
	s_deviceContext->VSSetConstantBuffers(0, 2, vertex_buffer);
	//pixel
	ID3D11SamplerState  *pixel_sampler[4] = {s_SamplerPerlin,s_SamplerGradient,s_SamplerCube,s_SamplerFreshnel};
	s_deviceContext->PSSetSamplers(1, 4, pixel_sampler);
	ID3D11ShaderResourceView *pixel_shader_view[4] = {s_ShaderViewPerlin,shaderViewGradient,s_ShaderViewReflectCube,s_ShaderViewFreshnel};
	s_deviceContext->PSSetShaderResources(1, 4, pixel_shader_view);
	ID3D11Buffer    *pixel_buffer[2] = {s_ConstantBuffer,s_PerFrameBuffer};
	s_deviceContext->PSSetConstantBuffers(0, 2, pixel_buffer);
	//缓冲区绑定
	unsigned stride = sizeof(float)*2;
	unsigned offset = 0;
	s_deviceContext->IASetVertexBuffers(0, 1, &s_VertexBuffer,&stride,&offset );
	s_deviceContext->IASetIndexBuffer(s_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	s_deviceContext->IASetInputLayout(s_LayoutInput);
	s_deviceContext->RSSetState(s_RasterizerSolid);
	//s_deviceContext->RSSetState(s_RasterizerWire);
	//公共的视图矩阵
	const D3DXMATRIX    matView = s_reflect_matrix * camera.getViewMatrix();
	D3DXMATRIX    matScale, matTranslate;
	//渲染所有的网格模型
	//for (int k = 21; k < 22/*g_RenderTree.size()*/; ++k)
	for (int k = 0; k < g_RenderTree.size(); ++k)
	{
		QuadTree   &node = g_RenderTree[k];
		//如果非叶子节点,直接跳过
		if (!isLeaf(node))
			continue;
		const    RenderPatternParam  &pattern = selectRenderPattern(g_RenderTree,node);
		cbPerFrameBuffer    per_frame_buffer;
		float    level_size = g_MeshSize >> node.lod;
		//
		D3DXMatrixScaling(&matScale, node.length / level_size, node.length / level_size, 1.0f);
		D3DXMatrixTranspose(&per_frame_buffer.g_LocalMatrix, &matScale);

		D3DXMatrixTranslation(&matTranslate, node.left_bottom.x, node.left_bottom.y, 0);
		D3DXMATRIX    matLocalView = matTranslate * matView;
		D3DXVECTOR3  eye_location(0, 0, 0);
		D3DXMatrixInverse(&matLocalView, nullptr, &matLocalView);
		D3DXVec3TransformCoord(&per_frame_buffer.g_EyeLocation, &eye_location, &matLocalView);

		D3DXMATRIX matMVP = matTranslate * matView * camera.getProjMatrix();
		D3DXMatrixTranspose(&per_frame_buffer.g_ViewProjMatrix, &matMVP);
		//UVBase
		per_frame_buffer.g_UVBase = node.left_bottom / g_PatchSize * g_PerlinSize;
		per_frame_buffer.g_PerlinMovement = -g_WindDir * time * g_PerlinSpeed;

		D3D11_MAPPED_SUBRESOURCE  mapRes;
		s_deviceContext->Map(s_PerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapRes);
		cbPerFrameBuffer *map_pointer = (cbPerFrameBuffer *)mapRes.pData;
		*map_pointer = per_frame_buffer;
		s_deviceContext->Unmap(s_PerFrameBuffer, 0);
		//内部三角形
		if (pattern.triangleFaceCount > 0)
		{
			s_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			s_deviceContext->DrawIndexed(pattern.triangleFaceCount +2, pattern.vertexOffset, 0);
		}
		//外部三角形
		if (pattern.boundaryFaceCount > 0)
		{
			s_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			s_deviceContext->DrawIndexed(pattern.boundaryFaceCount * 3, pattern.boundaryOffset, 0);
		}
	}
	//restore
	float blendFactor[4] = {0,0,0,0};
	s_deviceContext->OMSetBlendState(nullptr,blendFactor, -1);
	s_deviceContext->RSSetState(nullptr);
	vertex_buffer[0] = nullptr, vertex_buffer[1] = nullptr;
	s_deviceContext->VSSetConstantBuffers(0, 2, vertex_buffer);
	s_deviceContext->PSSetConstantBuffers(0, 2,vertex_buffer);
	vertex_shader_view[0] = nullptr;
	vertex_shader_view[1] = nullptr;
	s_deviceContext->VSSetShaderResources(0, 2, vertex_shader_view);
	pixel_shader_view[0] = nullptr;
	pixel_shader_view[1] = nullptr;
	pixel_shader_view[2] = nullptr;
	pixel_shader_view[3] = nullptr;
	s_deviceContext->PSSetShaderResources(1, 4, pixel_shader_view);
}