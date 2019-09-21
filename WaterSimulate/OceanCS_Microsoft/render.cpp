//--------------------------------------------------------------------------------------
// File: render.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include "utilities.h"
#include "DXUT.h"
#include "DXUTCamera.h"
#include "SDKmisc.h"

#include "ocean_simulator.h"

#pragma warning(disable:4995)
#include <vector>
using namespace std;

#define FRESNEL_TEX_SIZE			256
#define PERLIN_TEX_SIZE				64

struct ocean_vertex
{
    float index_x;
    float index_y;
};

// Mesh properties:

// Mesh grid dimension, must be 2^n. 4x4 ~ 256x256//128
const int g_MeshDim = 128;
// Side length of square shaped mesh patch//2000
//2000
float g_PatchLength = 2000;
// Dimension of displacement map//512
//512
int g_DisplaceMapDim = 512;
// Subdivision threshold. Any quad covers more pixels than this value needs to be subdivided.
//100
const float g_UpperGridCoverage = 100.0f;
// Draw distance = g_PatchLength * 2^g_FurthestCover
//9
const int g_FurthestCover = 9;


// Shading properties:
// Two colors for waterbody and sky color
D3DXVECTOR3 g_SkyColor= D3DXVECTOR3(0.38f, 0.45f, 0.56f);
D3DXVECTOR3 g_WaterbodyColor = D3DXVECTOR3(0.07f, 0.15f, 0.2f);
// Blending term for sky cubemap
const float g_SkyBlending = 16.0f;

// Perlin wave parameters//1.0f
const float g_PerlinSize = 1.0f;
//Perlin Speed 0.06f
const float g_PerlinSpeed = 0.06f;
//35, 42, 57
const D3DXVECTOR3 g_PerlinAmplitude = D3DXVECTOR3(35, 42, 57);
//1.4f, 1.6f, 2.2f
const D3DXVECTOR3 g_PerlinGradient = D3DXVECTOR3(1.4f, 1.6f, 2.2f);
//1.12f, 0.59f, 0.23f
const D3DXVECTOR3 g_PerlinOctave = D3DXVECTOR3(1.12f, 0.59f, 0.23f);
D3DXVECTOR2 g_WindDir;//(0.8,0.6)
//0.1f, -0.4f, 0.2f
const D3DXVECTOR3 g_BendParam = D3DXVECTOR3(0.1f, -0.4f, 0.2f);

// Sunspot parameters//0.936016f, -0.343206f, 0.0780013f
const D3DXVECTOR3 g_SunDir = D3DXVECTOR3(0.936016f, -0.343206f, 0.0780013f);
//1.0f, 1.0f, 0.6f
const D3DXVECTOR3 g_SunColor = D3DXVECTOR3(1.0f, 1.0f, 0.6f);
const float g_Shineness = 400.0f;//400.0f

// Quadtree structures & routines
struct QuadNode
{
    D3DXVECTOR2 bottom_left;
    float length;
    int lod;

    int sub_node[4];
};
//四叉树渲染模型
struct QuadRenderParam
{
    UINT num_inner_verts;
    UINT num_inner_faces;
    UINT inner_start_index;

    UINT num_boundary_verts;
    UINT num_boundary_faces;
    UINT boundary_start_index;
};

// Quad-tree LOD, 0 to 9 (1x1 ~ 512x512) 
//final ==> 7
int g_Lods = 0;
// Pattern lookup array. Filled at init time.
//关于该数据结构的说明,请参考 createSurfaceMesh 函数
QuadRenderParam g_mesh_patterns[9][3][3][3][3];
// Pick a proper mesh pattern according to the adjacent patches.
QuadRenderParam& selectMeshPattern(const QuadNode& quad_node);

// Rendering list
vector<QuadNode> g_render_list;
int buildNodeList(QuadNode& quad_node, const CBaseCamera& camera,int *visibilitys);

// D3D11 buffers and layout
ID3D11Buffer* g_pMeshVB = NULL;
ID3D11Buffer* g_pMeshIB = NULL;
ID3D11InputLayout* g_pMeshLayout = NULL;

// Color look up 1D texture
ID3D11Texture1D* g_pFresnelMap = NULL;
ID3D11ShaderResourceView* g_pSRV_Fresnel = NULL;

// Distant perlin wave
ID3D11ShaderResourceView* g_pSRV_Perlin = NULL;

// Environment maps
ID3D11ShaderResourceView* g_pSRV_ReflectCube = NULL;

// logo
//ID3D11ShaderResourceView* g_pSRV_Logo = NULL;
const UINT g_LogoWidth = 80;
const UINT g_LogoHeight = 64;

// HLSL shaders
ID3D11VertexShader* g_pOceanSurfVS = NULL;
ID3D11PixelShader* g_pOceanSurfPS = NULL;
ID3D11PixelShader* g_pWireframePS = NULL;
ID3D11VertexShader* g_pLogoVS = NULL;
ID3D11PixelShader* g_pLogoPS = NULL;

// Samplers
ID3D11SamplerState* g_pHeightSampler = NULL;
ID3D11SamplerState* g_pGradientSampler = NULL;
ID3D11SamplerState* g_pFresnelSampler = NULL;
ID3D11SamplerState* g_pPerlinSampler = NULL;
ID3D11SamplerState* g_pCubeSampler = NULL;

// Constant buffer
struct Const_Per_Call
{
    D3DXMATRIX	g_matLocal;//模型的局部坐标系变换,将XOY平面转换到XOZ平面
    D3DXMATRIX	g_matWorldViewProj;
    D3DXVECTOR2 g_UVBase;
    D3DXVECTOR2 g_PerlinMovement;
    D3DXVECTOR3	g_LocalEye;
	float                        unuse;
	D3DXCOLOR      g_WireColor;
};

struct Const_Per_Frame
{
    D3DXVECTOR4	g_LogoPlacement;
};

struct Const_Shading
{
    D3DXVECTOR3	g_SkyColor;
    float		g_TexelLength_x2;//2000/512 x 2
    D3DXVECTOR3	g_WaterbodyColor;
    float		g_UVScale;//1.0f/2000
    D3DXVECTOR3	g_PerlinAmplitude;//35, 42, 57
    float		g_UVOffset;//1.0f/512
    D3DXVECTOR3	g_PerlinOctave;//1.12f, 0.59f, 0.23f
    float		g_PerlinSize;//1.0f
    D3DXVECTOR3	g_PerlinGradient;//1.4f, 1.6f, 2.2f
    float		g_Shineness;////400.0f
    D3DXVECTOR3	g_BendParam;//0.1f, -0.4f, 0.2f
    float		unused0;
    D3DXVECTOR3	g_SunDir;//0.936016f, -0.343206f, 0.0780013f
    float		unused1;
    D3DXVECTOR3	g_SunColor;//1.0f, 1.0f, 0.6f
    float		unused2;
};

ID3D11Buffer* g_pPerCallCB = NULL;
ID3D11Buffer* g_pPerFrameCB = NULL;
ID3D11Buffer* g_pShadingCB = NULL;

// State blocks
ID3D11RasterizerState* g_pRSState_Solid = NULL;
ID3D11RasterizerState* g_pRSState_Wireframe = NULL;
ID3D11DepthStencilState* g_pDSState_Disable = NULL;
ID3D11BlendState* g_pBState_Transparent = NULL;

// init & cleanup
void initRenderResource(const ocean_parameter& ocean_param, ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
void cleanupRenderResource();
// create a triangle strip mesh for ocean surface.
void createSurfaceMesh(ID3D11Device* pd3dDevice);
// create color/fresnel lookup table.
void createFresnelMap(ID3D11Device* pd3dDevice);
// create perlin noise texture for far-sight rendering
void loadTextures(ID3D11Device* pd3dDevice);
// Rendering routines
void renderShaded(const CBaseCamera& camera, ID3D11ShaderResourceView* displacemnet_map, ID3D11ShaderResourceView* gradient_map, ID3D11DeviceContext* pd3dContext);
void renderWireframe(const CBaseCamera& camera, ID3D11ShaderResourceView* displacemnet_map, ID3D11DeviceContext* pd3dContext);
// Shader compilation
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);


void initRenderResource(const ocean_parameter& ocean_param, ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    g_PatchLength = ocean_param.patch_length;
    g_DisplaceMapDim = ocean_param.dmap_dim;
    g_WindDir = ocean_param.wind_dir;

    // D3D buffers
    createSurfaceMesh(pd3dDevice);
    createFresnelMap(pd3dDevice);
    loadTextures(pd3dDevice);

    // HLSL
    // Vertex & pixel shaders
    ID3DBlob* pBlobOceanSurfVS = nullptr;
    ID3DBlob* pBlobOceanSurfPS = nullptr;
    ID3DBlob* pBlobWireframePS = nullptr;
    ID3DBlob* pBlobLogoVS = nullptr;
    ID3DBlob* pBlobLogoPS = nullptr;

    tstring filePath = GetFilePath::GetFilePath(_T("ocean_shading.hlsl"));

    CompileShaderFromFile((WCHAR *)filePath.c_str(), "OceanSurfVS", "vs_4_0", &pBlobOceanSurfVS);
    CompileShaderFromFile((WCHAR *)filePath.c_str(), "OceanSurfPS", "ps_4_0", &pBlobOceanSurfPS);
    CompileShaderFromFile((WCHAR *)filePath.c_str(), "WireframePS", "ps_4_0", &pBlobWireframePS);
    CompileShaderFromFile((WCHAR *)filePath.c_str(), "DisplayLogoVS", "vs_4_0", &pBlobLogoVS);
    CompileShaderFromFile((WCHAR *)filePath.c_str(), "DisplayLogoPS", "ps_4_0", &pBlobLogoPS);
    assert(pBlobOceanSurfVS);
    assert(pBlobOceanSurfPS);
    assert(pBlobWireframePS);
    assert(pBlobLogoVS);
    assert(pBlobLogoPS);

    pd3dDevice->CreateVertexShader(pBlobOceanSurfVS->GetBufferPointer(), pBlobOceanSurfVS->GetBufferSize(), NULL, &g_pOceanSurfVS);
    pd3dDevice->CreatePixelShader(pBlobOceanSurfPS->GetBufferPointer(), pBlobOceanSurfPS->GetBufferSize(), NULL, &g_pOceanSurfPS);
    pd3dDevice->CreatePixelShader(pBlobWireframePS->GetBufferPointer(), pBlobWireframePS->GetBufferSize(), NULL, &g_pWireframePS);
    pd3dDevice->CreateVertexShader(pBlobLogoVS->GetBufferPointer(), pBlobLogoVS->GetBufferSize(), NULL, &g_pLogoVS);
    pd3dDevice->CreatePixelShader(pBlobLogoPS->GetBufferPointer(), pBlobLogoPS->GetBufferSize(), NULL, &g_pLogoPS);
    assert(g_pOceanSurfVS);
    assert(g_pOceanSurfPS);
    assert(g_pWireframePS);
    assert(g_pLogoVS);
    assert(g_pLogoPS);
    SAFE_RELEASE(pBlobOceanSurfPS);
    SAFE_RELEASE(pBlobWireframePS);
    SAFE_RELEASE(pBlobLogoVS);
    SAFE_RELEASE(pBlobLogoPS);

    // Input layout
    D3D11_INPUT_ELEMENT_DESC mesh_layout_desc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    pd3dDevice->CreateInputLayout(mesh_layout_desc, 1, pBlobOceanSurfVS->GetBufferPointer(), pBlobOceanSurfVS->GetBufferSize(), &g_pMeshLayout);
    assert(g_pMeshLayout);

    SAFE_RELEASE(pBlobOceanSurfVS);

    // Constants
    D3D11_BUFFER_DESC cb_desc;
    cb_desc.Usage = D3D11_USAGE_DYNAMIC;
    cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cb_desc.MiscFlags = 0;    
    cb_desc.ByteWidth = PAD16(sizeof(Const_Per_Call));
    cb_desc.StructureByteStride = 0;
    pd3dDevice->CreateBuffer(&cb_desc, NULL, &g_pPerCallCB);
    assert(g_pPerCallCB);

    cb_desc.ByteWidth = PAD16(sizeof(Const_Per_Frame));
    pd3dDevice->CreateBuffer(&cb_desc, NULL, &g_pPerFrameCB);
    assert(g_pPerFrameCB);

    Const_Shading shading_data;
    // Grid side length * 2 g_PatchLength==>2000 ,g_DisplaceMapDim==>512
    shading_data.g_TexelLength_x2 = g_PatchLength / g_DisplaceMapDim * 2;;
    // Color
    shading_data.g_SkyColor = g_SkyColor;
    shading_data.g_WaterbodyColor = g_WaterbodyColor;
    // Texcoord
    shading_data.g_UVScale = 1.0f / g_PatchLength;
    shading_data.g_UVOffset = 1.0f / g_DisplaceMapDim;
    // Perlin
    shading_data.g_PerlinSize = g_PerlinSize;
    shading_data.g_PerlinAmplitude = g_PerlinAmplitude;
    shading_data.g_PerlinGradient = g_PerlinGradient;
    shading_data.g_PerlinOctave = g_PerlinOctave;
    // Reflection workaround
    shading_data.g_BendParam = g_BendParam;
    // Sun spots
    shading_data.g_SunColor = g_SunColor;
    shading_data.g_SunDir = g_SunDir;
    shading_data.g_Shineness = g_Shineness;

    D3D11_SUBRESOURCE_DATA cb_init_data;
    cb_init_data.pSysMem = &shading_data;
    cb_init_data.SysMemPitch = 0;
    cb_init_data.SysMemSlicePitch = 0;

    cb_desc.Usage = D3D11_USAGE_IMMUTABLE;
    cb_desc.CPUAccessFlags = 0;
    cb_desc.ByteWidth = PAD16(sizeof(Const_Shading));
    cb_desc.StructureByteStride = 0;
    pd3dDevice->CreateBuffer(&cb_desc, &cb_init_data, &g_pShadingCB);
    assert(g_pShadingCB);

    // Samplers
    D3D11_SAMPLER_DESC sam_desc;
    sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sam_desc.MipLODBias = 0;
    sam_desc.MaxAnisotropy = 1;
    sam_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sam_desc.BorderColor[0] = 1.0f;
    sam_desc.BorderColor[1] = 1.0f;
    sam_desc.BorderColor[2] = 1.0f;
    sam_desc.BorderColor[3] = 1.0f;
    sam_desc.MinLOD = 0;
    sam_desc.MaxLOD = FLT_MAX;
    pd3dDevice->CreateSamplerState(&sam_desc, &g_pHeightSampler);
    assert(g_pHeightSampler);

    sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    pd3dDevice->CreateSamplerState(&sam_desc, &g_pCubeSampler);
    assert(g_pCubeSampler);

    sam_desc.Filter = D3D11_FILTER_ANISOTROPIC;
    sam_desc.MaxAnisotropy = 8;
    pd3dDevice->CreateSamplerState(&sam_desc, &g_pGradientSampler);
    assert(g_pGradientSampler);

    sam_desc.MaxAnisotropy = 4;
    pd3dDevice->CreateSamplerState(&sam_desc, &g_pPerlinSampler);
    assert(g_pPerlinSampler);

    sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    pd3dDevice->CreateSamplerState(&sam_desc, &g_pFresnelSampler);
    assert(g_pFresnelSampler);

    // State blocks
    D3D11_RASTERIZER_DESC ras_desc;
    ras_desc.FillMode = D3D11_FILL_SOLID; 
    ras_desc.CullMode = D3D11_CULL_NONE; 
    ras_desc.FrontCounterClockwise = FALSE; 
    ras_desc.DepthBias = 0;
    ras_desc.SlopeScaledDepthBias = 0.0f;
    ras_desc.DepthBiasClamp = 0.0f;
    ras_desc.DepthClipEnable= TRUE;
    ras_desc.ScissorEnable = FALSE;
    ras_desc.MultisampleEnable = TRUE;
    ras_desc.AntialiasedLineEnable = FALSE;

    pd3dDevice->CreateRasterizerState(&ras_desc, &g_pRSState_Solid);
    assert(g_pRSState_Solid);

    ras_desc.FillMode = D3D11_FILL_WIREFRAME; 

    pd3dDevice->CreateRasterizerState(&ras_desc, &g_pRSState_Wireframe);
    assert(g_pRSState_Wireframe);

    D3D11_DEPTH_STENCIL_DESC depth_desc;
    memset(&depth_desc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));
    depth_desc.DepthEnable = FALSE;
    depth_desc.StencilEnable = FALSE;
    pd3dDevice->CreateDepthStencilState(&depth_desc, &g_pDSState_Disable);
    assert(g_pDSState_Disable);

    D3D11_BLEND_DESC blend_desc;
    memset(&blend_desc, 0, sizeof(D3D11_BLEND_DESC));
    blend_desc.AlphaToCoverageEnable = FALSE;
    blend_desc.IndependentBlendEnable = FALSE;
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    pd3dDevice->CreateBlendState(&blend_desc, &g_pBState_Transparent);
    assert(g_pBState_Transparent);
}

void cleanupRenderResource()
{
    SAFE_RELEASE(g_pMeshIB);
    SAFE_RELEASE(g_pMeshVB);
    SAFE_RELEASE(g_pMeshLayout);

    SAFE_RELEASE(g_pOceanSurfVS);
    SAFE_RELEASE(g_pOceanSurfPS);
    SAFE_RELEASE(g_pWireframePS);
    SAFE_RELEASE(g_pLogoVS);
    SAFE_RELEASE(g_pLogoPS);

    SAFE_RELEASE(g_pFresnelMap);
    SAFE_RELEASE(g_pSRV_Fresnel);
    SAFE_RELEASE(g_pSRV_Perlin);
    SAFE_RELEASE(g_pSRV_ReflectCube);
    //SAFE_RELEASE(g_pSRV_Logo);

    SAFE_RELEASE(g_pHeightSampler);
    SAFE_RELEASE(g_pGradientSampler);
    SAFE_RELEASE(g_pFresnelSampler);
    SAFE_RELEASE(g_pPerlinSampler);
    SAFE_RELEASE(g_pCubeSampler);

    SAFE_RELEASE(g_pPerCallCB);
    SAFE_RELEASE(g_pPerFrameCB);
    SAFE_RELEASE(g_pShadingCB);

    SAFE_RELEASE(g_pRSState_Solid);
    SAFE_RELEASE(g_pRSState_Wireframe);
    SAFE_RELEASE(g_pDSState_Disable);
    SAFE_RELEASE(g_pBState_Transparent);

    g_render_list.clear();
}
//计算子空间索引,用于四叉树的子区间划分
#define MESH_INDEX_2D(x, y)	(((y) + vert_rect.bottom) * (g_MeshDim + 1) + (x) + vert_rect.left)
/*
  *目前猜测该函数的功能是 对四叉树的每个子节点进行空间网格划分,找出属于目标子空间的网格顶点索引
  *
 */
// Generate boundary mesh for a patch. Return the number of generated indices
int generateBoundaryMesh(int left_degree, int right_degree, int bottom_degree, int top_degree,
    RECT &vert_rect, int g_MeshDim, DWORD* output)
{
    // Triangle list for bottom boundary
    int i, j;
    int counter = 0;
    int width = vert_rect.right - vert_rect.left;
	//判断,是否需要在四边形的下方建立三角形序列来衔接其邻接四边形
    if (bottom_degree > 0)
    {
		//此两个数值之间的除法运算一定会整除
        int b_step = width / bottom_degree;

        for (i = 0; i < width; i += b_step)
        {
			//边界平滑过渡
            output[counter++] = MESH_INDEX_2D(i, 0);
            output[counter++] = MESH_INDEX_2D(i + b_step / 2, 1);
            output[counter++] = MESH_INDEX_2D(i + b_step, 0);

            for (j = 0; j < b_step / 2; j ++)
            {
                if (i == 0 && j == 0 && left_degree > 0)
                    continue;

                output[counter++] = MESH_INDEX_2D(i, 0);
                output[counter++] = MESH_INDEX_2D(i + j, 1);
                output[counter++] = MESH_INDEX_2D(i + j + 1, 1);
            }

            for (j = b_step / 2; j < b_step; j ++)
            {
                if (i == width - b_step && j == b_step - 1 && right_degree > 0)
                    continue;

                output[counter++] = MESH_INDEX_2D(i + b_step, 0);
                output[counter++] = MESH_INDEX_2D(i + j, 1);
                output[counter++] = MESH_INDEX_2D(i + j + 1, 1);
            }
        }
    }

    // Right boundary
    int height = vert_rect.top - vert_rect.bottom;

    if (right_degree > 0)
    {
        int r_step = height / right_degree;

        for (i = 0; i < height; i += r_step)
        {
            output[counter++] = MESH_INDEX_2D(width, i);
            output[counter++] = MESH_INDEX_2D(width - 1, i + r_step / 2);
            output[counter++] = MESH_INDEX_2D(width, i + r_step);

            for (j = 0; j < r_step / 2; j ++)
            {
                if (i == 0 && j == 0 && bottom_degree > 0)
                    continue;

                output[counter++] = MESH_INDEX_2D(width, i);
                output[counter++] = MESH_INDEX_2D(width - 1, i + j);
                output[counter++] = MESH_INDEX_2D(width - 1, i + j + 1);
            }

            for (j = r_step / 2; j < r_step; j ++)
            {
                if (i == height - r_step && j == r_step - 1 && top_degree > 0)
                    continue;

                output[counter++] = MESH_INDEX_2D(width, i + r_step);
                output[counter++] = MESH_INDEX_2D(width - 1, i + j);
                output[counter++] = MESH_INDEX_2D(width - 1, i + j + 1);
            }
        }
    }

    // Top boundary
    if (top_degree > 0)
    {
        int t_step = width / top_degree;

        for (i = 0; i < width; i += t_step)
        {
            output[counter++] = MESH_INDEX_2D(i, height);
            output[counter++] = MESH_INDEX_2D(i + t_step / 2, height - 1);
            output[counter++] = MESH_INDEX_2D(i + t_step, height);

            for (j = 0; j < t_step / 2; j ++)
            {
                if (i == 0 && j == 0 && left_degree > 0)
                    continue;

                output[counter++] = MESH_INDEX_2D(i, height);
                output[counter++] = MESH_INDEX_2D(i + j, height - 1);
                output[counter++] = MESH_INDEX_2D(i + j + 1, height - 1);
            }

            for (j = t_step / 2; j < t_step; j ++)
            {
                if (i == width - t_step && j == t_step - 1 && right_degree > 0)
                    continue;

                output[counter++] = MESH_INDEX_2D(i + t_step, height);
                output[counter++] = MESH_INDEX_2D(i + j, height - 1);
                output[counter++] = MESH_INDEX_2D(i + j + 1, height - 1);
            }
        }
    }

    // Left boundary
    if (left_degree > 0)
    {
        int l_step = height / left_degree;

        for (i = 0; i < height; i += l_step)
        {
            output[counter++] = MESH_INDEX_2D(0, i);
            output[counter++] = MESH_INDEX_2D(1, i + l_step / 2);
            output[counter++] = MESH_INDEX_2D(0, i + l_step);

            for (j = 0; j < l_step / 2; j ++)
            {
                if (i == 0 && j == 0 && bottom_degree > 0)
                    continue;

                output[counter++] = MESH_INDEX_2D(0, i);
                output[counter++] = MESH_INDEX_2D(1, i + j);
                output[counter++] = MESH_INDEX_2D(1, i + j + 1);
            }

            for (j = l_step / 2; j < l_step; j ++)
            {
                if (i == height - l_step && j == l_step - 1 && top_degree > 0)
                    continue;

                output[counter++] = MESH_INDEX_2D(0, i + l_step);
                output[counter++] = MESH_INDEX_2D(1, i + j);
                output[counter++] = MESH_INDEX_2D(1, i + j + 1);
            }
        }
    }

    return counter;
}

// Generate inner mesh for a patch. Return the number of generated indices
int generateInnerMesh(RECT &vert_rect, int g_MeshDim, DWORD* output)
{
    int i, j;
    int counter = 0;
    int width = vert_rect.right - vert_rect.left;
    int height = vert_rect.top - vert_rect.bottom;

    bool reverse = false;
	//Triangle Strip
	//对于空间的网格做一系列的Triangle Strip 分解,分解的过程中会出现三角形顶点顺序的顺时针
	//与逆时针相互交替的行为
    for (i = 0; i < height; i++)
    {
        if (reverse == false)
        {
            output[counter++] = MESH_INDEX_2D(0, i);
            output[counter++] = MESH_INDEX_2D(0, i + 1);
            for (j = 0; j < width; j++)
            {
                output[counter++] = MESH_INDEX_2D(j + 1, i);
                output[counter++] = MESH_INDEX_2D(j + 1, i + 1);
            }
        }
        else//严格来说,此算法并不严格正确,因为在两个行间距产生了无效的三角形顶点序列
			//因为DirectX的三角形裁剪算法,将会把生成的衔接处的直线裁剪掉,因此渲染后的结果是正确的
        {
            output[counter++] = MESH_INDEX_2D(width, i);
            output[counter++] = MESH_INDEX_2D(width, i + 1);
            for (j = width - 1; j >= 0; j--)
            {
                output[counter++] = MESH_INDEX_2D(j, i);
                output[counter++] = MESH_INDEX_2D(j, i + 1);
            }
        }

        reverse = !reverse;
    }

    return counter;
}

void createSurfaceMesh(ID3D11Device* pd3dDevice)
{
    // --------------------------------- Vertex Buffer -------------------------------
    int num_verts = (g_MeshDim + 1) * (g_MeshDim + 1);
    ocean_vertex* pV = new ocean_vertex[num_verts];
    assert(pV);

    int i, j;
	int index = 0;
    for (i = 0; i <= g_MeshDim; i++)
    {
        for (j = 0; j <= g_MeshDim; j++)
        {
            pV[index].index_x = (float)j;
            pV[index].index_y = (float)i;

			index += 1;
        }
    }

    D3D11_BUFFER_DESC vb_desc;
    vb_desc.ByteWidth = num_verts * sizeof(ocean_vertex);
    vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
    vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vb_desc.CPUAccessFlags = 0;
    vb_desc.MiscFlags = 0;
    vb_desc.StructureByteStride = sizeof(ocean_vertex);

    D3D11_SUBRESOURCE_DATA init_data;
    init_data.pSysMem = pV;
    init_data.SysMemPitch = 0;
    init_data.SysMemSlicePitch = 0;

    SAFE_RELEASE(g_pMeshVB);
    pd3dDevice->CreateBuffer(&vb_desc, &init_data, &g_pMeshVB);
    assert(g_pMeshVB);

    SAFE_DELETE(pV);


    // --------------------------------- Index Buffer -------------------------------
    // The index numbers for all mesh LODs (up to 256x256)
    const int index_size_lookup[] = {0, 0, 4284, 18828, 69444, 254412, 956916, 3689820, 14464836};

    memset(g_mesh_patterns, 0, sizeof(g_mesh_patterns));

    g_Lods = 0;
    for (i = g_MeshDim; i > 1; i >>= 1)
        g_Lods ++;

    // Generate patch meshes. Each patch contains two parts: the inner mesh which is a regular
    // grids in a triangle strip. The boundary mesh is constructed w.r.t.
	//(with regard to with reference to ) the edge degrees to
    // meet water-tight requirement.
	//边界,关于边的度,满足水网格的边界紧凑衔接需要
	//这里所谓的边的度,指代的是再给定宽高的区域内,横向/纵向的网格的数目
    DWORD* index_array = new DWORD[index_size_lookup[g_Lods]];
    assert(index_array);

    int offset = 0;
	//from 128==>1,final ==> 4
    int level_size = g_MeshDim;
	/*
	  *关于渲染模型的说明
	  *在创建的模型中,对于任意一个数组元素g_mesh_patterns[level][l][r][b][t]
	  *代表着在细节层次level上,与该模型相邻的四个Patch,他们的细节层次边长倍数分别为目标模型的
	  *2的l/r/b/t 次幂倍数
	  *在实际的细节计算应用中,程序只是用了头三个(0,1,2)
	 */
    // Enumerate patterns
    for (int level = 0; level <= g_Lods - 2; level ++)//lod,丢弃掉了 1x1,2x2网格
    {
        int left_degree = level_size;
        for (int left_type = 0; left_type < 3; left_type ++)
        {
            int right_degree = level_size;
            for (int right_type = 0; right_type < 3; right_type ++)
            {
                int bottom_degree = level_size;
                for (int bottom_type = 0; bottom_type < 3; bottom_type ++)
                {
                    int top_degree = level_size;
                    for (int top_type = 0; top_type < 3; top_type ++)
                    {
                        QuadRenderParam* pattern = &g_mesh_patterns[level][left_type][right_type][bottom_type][top_type];
                        // Inner mesh (triangle strip)
						//检查边界条件,如果四边形的邻接四边形与该四边形具有相同的边宽度,则
						//周围的四边形就不需要与该四边形形成无缝的邻接边
                        RECT inner_rect;
                        inner_rect.left   = (left_degree   == level_size) ? 0 : 1;
                        inner_rect.right  = (right_degree  == level_size) ? level_size : level_size - 1;
                        inner_rect.bottom = (bottom_degree == level_size) ? 0 : 1;
                        inner_rect.top    = (top_degree    == level_size) ? level_size : level_size - 1;

                        int num_new_indices = generateInnerMesh(inner_rect, g_MeshDim, index_array + offset);

                        pattern->inner_start_index = offset;
                        pattern->num_inner_verts = (level_size + 1) * (level_size + 1);
                        pattern->num_inner_faces = num_new_indices - 2;
                        offset += num_new_indices;

                        // Boundary mesh (triangle list)
                        int l_degree = (left_degree   == level_size) ? 0 : left_degree;
                        int r_degree = (right_degree  == level_size) ? 0 : right_degree;
                        int b_degree = (bottom_degree == level_size) ? 0 : bottom_degree;
                        int t_degree = (top_degree    == level_size) ? 0 : top_degree;

                        RECT outer_rect = {0, level_size, level_size, 0};
                        num_new_indices = generateBoundaryMesh(l_degree, r_degree, b_degree, t_degree, outer_rect, g_MeshDim, index_array + offset);

                        pattern->boundary_start_index = offset;
                        pattern->num_boundary_verts = (level_size + 1) * (level_size + 1);
                        pattern->num_boundary_faces = num_new_indices / 3;
                        offset += num_new_indices;

                        top_degree /= 2;
                    }
                    bottom_degree /= 2;
                }
                right_degree /= 2;
            }
            left_degree /= 2;
        }
        level_size /= 2;
    }

    assert(offset == index_size_lookup[g_Lods]);

    D3D11_BUFFER_DESC ib_desc;
    ib_desc.ByteWidth = index_size_lookup[g_Lods] * sizeof(DWORD);
    ib_desc.Usage = D3D11_USAGE_IMMUTABLE;
    ib_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ib_desc.CPUAccessFlags = 0;
    ib_desc.MiscFlags = 0;
    ib_desc.StructureByteStride = sizeof(DWORD);

    init_data.pSysMem = index_array;

    SAFE_RELEASE(g_pMeshIB);
    pd3dDevice->CreateBuffer(&ib_desc, &init_data, &g_pMeshIB);
    assert(g_pMeshIB);

    SAFE_DELETE(index_array);
}

void createFresnelMap(ID3D11Device* pd3dDevice)
{
    DWORD buffer[FRESNEL_TEX_SIZE];
    for (int i = 0; i < FRESNEL_TEX_SIZE; i++)
    {
        float cos_a = i / (FLOAT)FRESNEL_TEX_SIZE;
        // Using water's refraction index 1.33
        DWORD fresnel = (DWORD)(D3DXFresnelTerm(cos_a, 1.33f) * 255);

        DWORD sky_blend = (DWORD)(powf(1 / (1 + cos_a), g_SkyBlending) * 255);

        buffer[i] = (sky_blend << 8) | fresnel;
    }

    D3D11_TEXTURE1D_DESC tex_desc;
    tex_desc.Width = FRESNEL_TEX_SIZE;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    tex_desc.CPUAccessFlags = 0;
    tex_desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA init_data;
    init_data.pSysMem = buffer;
    init_data.SysMemPitch = 0;
    init_data.SysMemSlicePitch = 0;

    pd3dDevice->CreateTexture1D(&tex_desc, &init_data, &g_pFresnelMap);
    assert(g_pFresnelMap);

    //SAFE_DELETE_ARRAY(buffer);

    // Create shader resource
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    srv_desc.Texture1D.MipLevels = 1;
    srv_desc.Texture1D.MostDetailedMip = 0;

    pd3dDevice->CreateShaderResourceView(g_pFresnelMap, &srv_desc, &g_pSRV_Fresnel);
    assert(g_pSRV_Fresnel);
}

void loadTextures(ID3D11Device* pd3dDevice)
{
    WCHAR strPath[MAX_PATH];
    DXUTFindDXSDKMediaFileCch(strPath, MAX_PATH, (WCHAR *)GetFilePath::GetFilePath(_T("Media\\perlin_noise.dds")).c_str() );
    D3DX11CreateShaderResourceViewFromFile(pd3dDevice, strPath, NULL, NULL, &g_pSRV_Perlin, NULL);
    assert(g_pSRV_Perlin);

    DXUTFindDXSDKMediaFileCch(strPath, MAX_PATH, (WCHAR *)GetFilePath::GetFilePath(_T("Media\\reflect_cube.dds")).c_str() );
    D3DX11CreateShaderResourceViewFromFile(pd3dDevice, strPath, NULL, NULL, &g_pSRV_ReflectCube, NULL);
    assert(g_pSRV_ReflectCube);
}

bool checkNodeVisibility(const QuadNode& quad_node, const CBaseCamera& camera,int *visibility)
{
//    // Plane equation setup
//    const D3DXMATRIX &matProj = *camera.GetProjMatrix();
//
//    // Left plane
//    float fov_x = atan(1.0f / matProj(0, 0));
//	*visibility = false;
//#ifdef NV_STEREO
//    // Expand the frustum a little for stereo vision
//    fov_x += 0.1745329278f;
//#endif
//	/*
//	  *在下面的平面方程的推导中用到了标准视锥体的一些隐含的性质
//	  *matProj[0][0] = nearZ/width
//	  *matProj[1][1] = nearZ/height
//	  *经过(0,0,0)点
//	 */
//    D3DXVECTOR4 plane_left(cos(fov_x), 0, sin(fov_x), 0);
//    // Right plane
//    D3DXVECTOR4 plane_right(-cos(fov_x), 0, sin(fov_x), 0);
//
//    // Bottom plane
//    float fov_y = atan(1.0f / matProj(1, 1));
//    D3DXVECTOR4 plane_bottom(0, cos(fov_y), sin(fov_y), 0);
//    // Top plane
//    D3DXVECTOR4 plane_top(0, -cos(fov_y), sin(fov_y), 0);
//
//    // Test quad corners against view frustum in view space
//    D3DXVECTOR4 corner_verts[4];
//    corner_verts[0] = D3DXVECTOR4(quad_node.bottom_left.x, quad_node.bottom_left.y, 0, 1);
//    corner_verts[1] = corner_verts[0] + D3DXVECTOR4(quad_node.length, 0, 0, 0);
//    corner_verts[2] = corner_verts[0] + D3DXVECTOR4(quad_node.length, quad_node.length, 0, 0);
//    corner_verts[3] = corner_verts[0] + D3DXVECTOR4(0, quad_node.length, 0, 0);
//	//乘以模型矩阵,Y/Z坐标交换,变换到XOZ平面
//    const D3DXMATRIX matView = D3DXMATRIX(1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1) * *camera.GetViewMatrix();
//    D3DXVec4Transform(&corner_verts[0], &corner_verts[0], &matView);
//    D3DXVec4Transform(&corner_verts[1], &corner_verts[1], &matView);
//    D3DXVec4Transform(&corner_verts[2], &corner_verts[2], &matView);
//    D3DXVec4Transform(&corner_verts[3], &corner_verts[3], &matView);
//
//    // Test against eye plane
//    if (corner_verts[0].z < 0 && corner_verts[1].z < 0 && corner_verts[2].z < 0 && corner_verts[3].z < 0)
//        return false;
//
//    // Test against left plane
//    float dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_left);
//    float dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_left);
//    float dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_left);
//    float dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_left);
//    if (dist_0 < 0 && dist_1 < 0 && dist_2 < 0 && dist_3 < 0)
//        return false;
//
//    // Test against right plane
//    dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_right);
//    dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_right);
//    dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_right);
//    dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_right);
//    if (dist_0 < 0 && dist_1 < 0 && dist_2 < 0 && dist_3 < 0)
//        return false;
//
//    // Test against bottom plane
//    dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_bottom);
//    dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_bottom);
//    dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_bottom);
//    dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_bottom);
//    if (dist_0 < 0 && dist_1 < 0 && dist_2 < 0 && dist_3 < 0)
//        return false;
//
//    // Test against top plane
//    dist_0 = D3DXVec4Dot(&corner_verts[0], &plane_top);
//    dist_1 = D3DXVec4Dot(&corner_verts[1], &plane_top);
//    dist_2 = D3DXVec4Dot(&corner_verts[2], &plane_top);
//    dist_3 = D3DXVec4Dot(&corner_verts[3], &plane_top);
//    if (dist_0 < 0 && dist_1 < 0 && dist_2 < 0 && dist_3 < 0)
//        return false;
//
//	*visibility = 1;
//    return true;

	*visibility = 0;
	const D3DXMATRIX   matView = D3DXMATRIX(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1) * *camera.GetViewMatrix();
	//节点的四个顶点
	D3DXVECTOR3  quad_vertex[4] = {
		D3DXVECTOR3(quad_node.bottom_left.x,quad_node.bottom_left.y,0),
		D3DXVECTOR3(quad_node.bottom_left.x + quad_node.length,quad_node.bottom_left.y,0),
		D3DXVECTOR3(quad_node.bottom_left.x + quad_node.length,quad_node.bottom_left.y + quad_node.length,0),
		D3DXVECTOR3(quad_node.bottom_left.x,quad_node.bottom_left.y + quad_node.length,0),
	};
	//变换到摄像机空间
	for (int k = 0; k < 4; ++k)
		D3DXVec3TransformCoord(quad_vertex + k, quad_vertex + k, &matView);
	//初步检测是否在摄像机之后
	if (quad_vertex[0].z < 0 && quad_vertex[1].z < 0 && quad_vertex[2].z < 0 && quad_vertex[3].z < 0)
		return false;
	//视锥体裁剪
	const float *matrix_array = reinterpret_cast<const float*>(camera.GetProjMatrix());
	//视锥体左侧平面裁剪
	float   f = 1.0f / sqrtf(1 + matrix_array[0] * matrix_array[0]);
	D3DXVECTOR3     l_plane(f *matrix_array[0], 0, f);
	float  distance[4];
	for (int k = 0; k < 4; ++k)
		distance[k] = l_plane.x *quad_vertex[k].x + l_plane.y *quad_vertex[k].y + l_plane.z * quad_vertex[k].z;
	if (distance[0] < 0 && distance[1] < 0 && distance[2] < 0 && distance[3] < 0)
		return false;
	//右侧的平面
	D3DXVECTOR3     r_plane(-f*matrix_array[0], 0, f);
	for (int k = 0; k < 4; ++k)
		distance[k] = r_plane.x * quad_vertex[k].x + r_plane.y * quad_vertex[k].y + r_plane.z* quad_vertex[k].z;
	if (distance[0] < 0 && distance[1] < 0 && distance[2] < 0 && distance[3] < 0)
		return false;
	//下侧的平面裁剪
	f = 1.0f / sqrtf(1 + matrix_array[5] * matrix_array[5]);
	D3DXVECTOR3   b_plane(0, f*matrix_array[5], f);
	for (int k = 0; k < 4; ++k)
		distance[k] = b_plane.x*quad_vertex[k].x + b_plane.y*quad_vertex[k].y + b_plane.z*quad_vertex[k].z;
	if (distance[0] < 0 && distance[1] < 0 && distance[2] < 0 && distance[3] < 0)
		return false;
	//下侧的平面
	D3DXVECTOR3   t_plane(0, -f*matrix_array[5], f);
	for (int k = 0; k < 4; ++k)
		distance[k] = t_plane.x*quad_vertex[k].x + t_plane.y*quad_vertex[k].y + t_plane.z*quad_vertex[k].z;
	if (distance[0] < 0 && distance[1] < 0 && distance[2] < 0 && distance[3] < 0)
		return false;
	*visibility = 1;
	return true;
}
//评估网格覆盖
float estimateGridCoverage(const QuadNode& quad_node, const CBaseCamera& camera, float screen_area)
{
    // Estimate projected area

    // Test 16 points on the quad and find out the biggest one.
    const static float sample_pos[16][2] =
    {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1},
        {0.5f, 0.333f},
        {0.25f, 0.667f},
        {0.75f, 0.111f},
        {0.125f, 0.444f},
        {0.625f, 0.778f},
        {0.375f, 0.222f},
        {0.875f, 0.556f},
        {0.0625f, 0.889f},
        {0.5625f, 0.037f},
        {0.3125f, 0.37f},
        {0.8125f, 0.704f},
        {0.1875f, 0.148f},
    };

    const D3DXMATRIX &matProj = *camera.GetProjMatrix();
    D3DXVECTOR3 eye_point = *camera.GetEyePt();
	//这里简化了网格的坐标变换,直接将摄像机的坐标转换到了网格的局部坐标
    eye_point = D3DXVECTOR3(eye_point.x, eye_point.z, eye_point.y);
	//目前猜测,这里除以g_MeshDim的原因是,大致上4个g_MeshDim x g_MeshDim的网格平面覆盖掉整个屏幕空间
    float grid_len_world = quad_node.length / g_MeshDim;
	const float area_world = grid_len_world * grid_len_world;// * abs(eye_point.z) / sqrt(nearest_sqr_dist);
	const float nearZDvX = matProj(0, 0);
	const float nearYDvY = matProj(1, 1);
	const float  finalInterpolation = area_world *nearZDvX * nearYDvY;//换算成网格的缩放比例面积

    float max_area_proj = 0;
    for (int i = 0; i < 16; i++)
    {
        D3DXVECTOR3 test_point(quad_node.bottom_left.x + quad_node.length * sample_pos[i][0], 
															quad_node.bottom_left.y + quad_node.length * sample_pos[i][1], 0);
        D3DXVECTOR3 eye_vec = test_point - eye_point;
        float dist = D3DXVec3Length(&eye_vec);
        float area_proj = finalInterpolation / (dist * dist);
		//计算大概覆盖的网格的数目
        if (max_area_proj < area_proj)
            max_area_proj = area_proj;
    }
	//不明白为什么最后乘上一个系数 0.25,也许是四叉树中一个节点的面积.
    return max_area_proj * screen_area * 0.25f;
}

bool isLeaf(const QuadNode& quad_node)
{
	const int *nodes = quad_node.sub_node;
    return (nodes[0] == -1 && nodes[1] == -1 && nodes[2] == -1 && nodes[3] == -1);
}
/*
  *查找最小叶子节点,该节点包含了目标点的坐标
 */
int searchLeaf(const vector<QuadNode>& node_list, const D3DXVECTOR2& point)
{
    int index = -1;

    int size = (int)node_list.size();
    QuadNode node = node_list.back();

    while (!isLeaf(node))
    {
        bool found = false;

        for (int i = 0; i < 4; i++)
        {
            index = node.sub_node[i];
            if (index == -1)
                continue;

            const QuadNode &sub_node = node_list[index];
            if (point.x >= sub_node.bottom_left.x && point.x <= sub_node.bottom_left.x + sub_node.length &&
                point.y >= sub_node.bottom_left.y && point.y <= sub_node.bottom_left.y + sub_node.length)
            {
                node = sub_node;
                found = true;
                break;
            }
        }

        if (!found)
            return -1;
    }

    return index;
}

QuadRenderParam& selectMeshPattern(const QuadNode& quad_node)
{
	//每一个完整的网格块的尺寸为g_PatchLength
	float     position_percent = 0.49f;
    // Check 4 adjacent quad.
    D3DXVECTOR2 point_left = quad_node.bottom_left + 
				D3DXVECTOR2(-g_PatchLength * 0.5f, quad_node.length * 0.5f);
    int left_adj_index = searchLeaf(g_render_list, point_left);

    D3DXVECTOR2 point_right = quad_node.bottom_left + 
					D3DXVECTOR2(quad_node.length + g_PatchLength * 0.5f, quad_node.length * 0.5f);
    int right_adj_index = searchLeaf(g_render_list, point_right);

    D3DXVECTOR2 point_bottom = quad_node.bottom_left + 
					D3DXVECTOR2(quad_node.length * 0.5f, -g_PatchLength * 0.5f);
    int bottom_adj_index = searchLeaf(g_render_list, point_bottom);

    D3DXVECTOR2 point_top = quad_node.bottom_left + 
				D3DXVECTOR2(quad_node.length * 0.5f, quad_node.length + g_PatchLength * 0.5f);
    int top_adj_index = searchLeaf(g_render_list, point_top);
	//邻接Patch与目标Patch Lod 之间的比例
    int left_type = 0;
	//最外围的判断条件是为了保证不会出现 A与B比较,同时B又与A比较的情况
    if (left_adj_index != -1 && g_render_list[left_adj_index].length > quad_node.length * 0.999f)
    {
        const QuadNode &adj_node = g_render_list[left_adj_index];
		//关于邻接四边形scale参数课题以理解为:
		//四边形的实际的边长尺寸与屏幕显示上的长度比例
		//关于下方代码的含义,可以参考renderShade函数里面的 网格的缩放矩阵变换
        float scale = adj_node.length / quad_node.length * (g_MeshDim >> quad_node.lod) / (g_MeshDim >> adj_node.lod);
		//float scale = (adj_node.length * (1<< adj_node.lod)) / (quad_node.length *(1<< quad_node.lod));
        if (scale > 3.999f)
            left_type = 2;
        else if (scale > 1.999f)
            left_type = 1;
    }

    int right_type = 0;
    if (right_adj_index != -1 && g_render_list[right_adj_index].length > quad_node.length * 0.999f)
    {
        const QuadNode &adj_node = g_render_list[right_adj_index];
        float scale = adj_node.length / quad_node.length * (g_MeshDim >> quad_node.lod) / (g_MeshDim >> adj_node.lod);
        if (scale > 3.999f)
            right_type = 2;
        else if (scale > 1.999f)
            right_type = 1;
    }

    int bottom_type = 0;
    if (bottom_adj_index != -1 && g_render_list[bottom_adj_index].length > quad_node.length * 0.999f)
    {
        const QuadNode &adj_node = g_render_list[bottom_adj_index];
        float scale = adj_node.length / quad_node.length * (g_MeshDim >> quad_node.lod) / (g_MeshDim >> adj_node.lod);
        if (scale > 3.999f)
            bottom_type = 2;
        else if (scale > 1.999f)
            bottom_type = 1;
    }

    int top_type = 0;
    if (top_adj_index != -1 && g_render_list[top_adj_index].length > quad_node.length * 0.999f)
    {
        const QuadNode &adj_node = g_render_list[top_adj_index];
        float scale = adj_node.length / quad_node.length * (g_MeshDim >> quad_node.lod) / (g_MeshDim >> adj_node.lod);
        if (scale > 3.999f)
            top_type = 2;
        else if (scale > 1.999f)
            top_type = 1;
    }

    // Check lookup table, [Lod][L][R][B][T]
    return g_mesh_patterns[quad_node.lod][left_type][right_type][bottom_type][top_type];
}

// Return value: if successful pushed into the list, return the position. If failed, return -1.
int buildNodeList(QuadNode& quad_node, const CBaseCamera& camera,int *visibility)
{
    // Check against view frustum
    if (!checkNodeVisibility(quad_node, camera,visibility))
        return -1;

    // Estimate the min grid coverage
    UINT num_vps = 1;
    D3D11_VIEWPORT vp;
    DXUTGetD3D11DeviceContext()->RSGetViewports(&num_vps, &vp);
    float min_coverage = estimateGridCoverage(quad_node, camera, vp.Width * vp.Height);

    // Recursively attach sub-nodes.
    bool visible = true;
	int     visibilitys[4];
	/*
	  *在实际的运作过程中,也有可能会出现一种情况
	  *就是 对A可见性测试通过后,分别对其四个子节点做测试却是失败的
	 */
    if (min_coverage > g_UpperGridCoverage && quad_node.length > g_PatchLength)
    {
        // Recursive rendering for sub-quads.
        QuadNode sub_node_0 = {quad_node.bottom_left, quad_node.length / 2, 0, {-1, -1, -1, -1}};
        quad_node.sub_node[0] = buildNodeList(sub_node_0, camera, visibilitys);

        QuadNode sub_node_1 = {quad_node.bottom_left + D3DXVECTOR2(quad_node.length/2, 0), quad_node.length / 2, 0, {-1, -1, -1, -1}};
        quad_node.sub_node[1] = buildNodeList(sub_node_1, camera, visibilitys+1);

        QuadNode sub_node_2 = {quad_node.bottom_left + D3DXVECTOR2(quad_node.length/2, quad_node.length/2), quad_node.length / 2, 0, {-1, -1, -1, -1}};
        quad_node.sub_node[2] = buildNodeList(sub_node_2, camera, visibilitys+2);

        QuadNode sub_node_3 = {quad_node.bottom_left + D3DXVECTOR2(0, quad_node.length/2), quad_node.length / 2, 0, {-1, -1, -1, -1}};
        quad_node.sub_node[3] = buildNodeList(sub_node_3, camera, visibilitys+3);

        visible = !isLeaf(quad_node);//叶子节点不可见,是因为叶子节点不可见,也代表着这个父节点不可见
		//assert(visible);
    }

    if (visible)
    {
        // Estimate mesh LOD,确定最紧凑的细节层次
        int lod = 0;
        for (lod = 0; lod < g_Lods - 1; lod++)
        {
            if (min_coverage > g_UpperGridCoverage)
                break;
            min_coverage *= 4;
        }
        // We don't use 1x1 and 2x2 patch. So the highest level is g_Lods - 2.
        quad_node.lod = min(lod, g_Lods - 2);
    }
    else
        return -1;

    // Insert into the list
    int position = (int)g_render_list.size();
    g_render_list.push_back(quad_node);

    return position;
}
static bool s_isFirst2 = true;
void renderShaded(const CBaseCamera& camera, ID3D11ShaderResourceView* displacemnet_map, ID3D11ShaderResourceView* gradient_map, float time, ID3D11DeviceContext* pd3dContext)
{
	int visibilitys[4];
    // Build rendering list
	//if(s_isFirst2)
		g_render_list.clear();
	//海平面可以延伸的最远处
    float ocean_extent = g_PatchLength * (1 << g_FurthestCover);
	//建立四叉树的根节点,左下角为整个延展区域的西南边界(以向北为正)
    QuadNode root_node = {D3DXVECTOR2(-ocean_extent * 0.5f, -ocean_extent * 0.5f), ocean_extent, 0, {-1,-1,-1,-1}};
	//if(s_isFirst2)
		buildNodeList(root_node, camera, visibilitys);
	//s_isFirst2 = false;
    // Matrices/model+view
    D3DXMATRIX matView = D3DXMATRIX(1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1) * *camera.GetViewMatrix();
    const D3DXMATRIX &matProj = *camera.GetProjMatrix();

    // VS & PS
    pd3dContext->VSSetShader(g_pOceanSurfVS, nullptr, 0);
    pd3dContext->PSSetShader(g_pOceanSurfPS, nullptr, 0);

    // Textures
    ID3D11ShaderResourceView* vs_srvs[2] = {displacemnet_map, g_pSRV_Perlin};
    pd3dContext->VSSetShaderResources(0, 2, &vs_srvs[0]);

    ID3D11ShaderResourceView* ps_srvs[4] = {g_pSRV_Perlin, gradient_map, g_pSRV_Fresnel, g_pSRV_ReflectCube};
    pd3dContext->PSSetShaderResources(1, 4, &ps_srvs[0]);

    // Samplers
    ID3D11SamplerState* vs_samplers[2] = {g_pHeightSampler, g_pPerlinSampler};
    pd3dContext->VSSetSamplers(0, 2, &vs_samplers[0]);

    ID3D11SamplerState* ps_samplers[4] = {g_pPerlinSampler, g_pGradientSampler, g_pFresnelSampler, g_pCubeSampler};
    pd3dContext->PSSetSamplers(1, 4, &ps_samplers[0]);

    // IA setup
    pd3dContext->IASetIndexBuffer(g_pMeshIB, DXGI_FORMAT_R32_UINT, 0);

    ID3D11Buffer* vbs[1] = {g_pMeshVB};
    UINT strides[1] = {sizeof(ocean_vertex)};
    UINT offsets[1] = {0};
    pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);

    pd3dContext->IASetInputLayout(g_pMeshLayout);

    // State blocks
    pd3dContext->RSSetState(g_pRSState_Solid);

    // Constants
    ID3D11Buffer* cbs[1] = {g_pShadingCB};
    pd3dContext->VSSetConstantBuffers(2, 1, cbs);
    pd3dContext->PSSetConstantBuffers(2, 1, cbs);

    // We assume the center of the ocean surface at (0, 0, 0).
    for (int i = 0; i < (int)g_render_list.size(); i++)
	//for (int i = 21; i < 22; i++)
    {
        QuadNode& node = g_render_list[i];

        if (!isLeaf(node))
            continue;

        // Check adjacent patches and select mesh pattern
        QuadRenderParam& render_param = selectMeshPattern(node);

        // Find the right LOD to render
		assert(node.lod>=0);
        int level_size = g_MeshDim >> node.lod ;
        //for (int lod = 0; lod < node.lod; lod++)
       //     level_size >>= 1;
		
        // Matrices and constants
        Const_Per_Call call_consts;

        // Expand of the local coordinate to world space patch size
        D3DXMATRIX matScale;
        D3DXMatrixScaling(&matScale, node.length / level_size, node.length / level_size, 1);
        D3DXMatrixTranspose(&call_consts.g_matLocal, &matScale);

        // WVP matrix
        D3DXMATRIX matWorld;
        D3DXMatrixTranslation(&matWorld, node.bottom_left.x, node.bottom_left.y, 0);
        D3DXMATRIX matWVP = matWorld * matView * matProj;
        D3DXMatrixTranspose(&call_consts.g_matWorldViewProj, &matWVP);

        // Texcoord for perlin noise
        D3DXVECTOR2 uv_base = node.bottom_left / g_PatchLength * g_PerlinSize;
        call_consts.g_UVBase = uv_base;

        // Constant g_PerlinSpeed need to be adjusted manually
        D3DXVECTOR2 perlin_move = -g_WindDir * time * g_PerlinSpeed;
        call_consts.g_PerlinMovement = perlin_move;

        // Eye point
        D3DXMATRIX matInvWV = matWorld * matView;
        D3DXMatrixInverse(&matInvWV, nullptr, &matInvWV);
        D3DXVECTOR3 vLocalEye(0, 0, 0);
        D3DXVec3TransformCoord(&vLocalEye, &vLocalEye, &matInvWV);
        call_consts.g_LocalEye = vLocalEye;

        // Update constant buffer
        D3D11_MAPPED_SUBRESOURCE mapped_res;            
        pd3dContext->Map(g_pPerCallCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
        assert(mapped_res.pData);
        *(Const_Per_Call*)mapped_res.pData = call_consts;
        pd3dContext->Unmap(g_pPerCallCB, 0);

        cbs[0] = g_pPerCallCB;
        pd3dContext->VSSetConstantBuffers(4, 1, cbs);
        pd3dContext->PSSetConstantBuffers(4, 1, cbs);

        // Perform draw call
        if (render_param.num_inner_faces > 0)
        {
            // Inner mesh of the patch
            pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            pd3dContext->DrawIndexed(render_param.num_inner_faces + 2, render_param.inner_start_index, 0);
        }

        if (render_param.num_boundary_faces > 0)
        {
            // Boundary mesh of the patch
            pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            pd3dContext->DrawIndexed(render_param.num_boundary_faces * 3, render_param.boundary_start_index, 0);
        }
    }

    // Unbind
    vs_srvs[0] = nullptr;
    vs_srvs[1] = nullptr;
    pd3dContext->VSSetShaderResources(0, 2, &vs_srvs[0]);

    ps_srvs[0] = nullptr;
    ps_srvs[1] = nullptr;
    ps_srvs[2] = nullptr;
    ps_srvs[3] = nullptr;
    pd3dContext->PSSetShaderResources(1, 4, &ps_srvs[0]);
}
static bool s_isFirst = true;
void renderWireframe(const CBaseCamera& camera, ID3D11ShaderResourceView* displacemnet_map, float time, ID3D11DeviceContext* pd3dContext)
{
	int visibilitys[4];
    // Build rendering list
	//if(s_isFirst)
		g_render_list.clear();
    float ocean_extent = g_PatchLength * (1 << g_FurthestCover);
    QuadNode root_node = {D3DXVECTOR2(-ocean_extent * 0.5f, -ocean_extent * 0.5f), ocean_extent, 0, {-1,-1,-1,-1}};
	//if(s_isFirst)
		buildNodeList(root_node, camera, visibilitys);
	//s_isFirst = false;
    // Matrices
    D3DXMATRIX matView = D3DXMATRIX(1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1) * *camera.GetViewMatrix();
    const D3DXMATRIX &matProj = *camera.GetProjMatrix();

    // VS & PS
    pd3dContext->VSSetShader(g_pOceanSurfVS, nullptr, 0);
    pd3dContext->PSSetShader(g_pWireframePS, nullptr, 0);

    // Textures
    ID3D11ShaderResourceView* vs_srvs[2] = {displacemnet_map, g_pSRV_Perlin};
    pd3dContext->VSSetShaderResources(0, 2, &vs_srvs[0]);

    // Samplers
    ID3D11SamplerState* vs_samplers[2] = {g_pHeightSampler, g_pPerlinSampler};
    pd3dContext->VSSetSamplers(0, 2, &vs_samplers[0]);

    ID3D11SamplerState* ps_samplers[4] = { nullptr, nullptr, nullptr, nullptr };
    pd3dContext->PSSetSamplers(1, 4, &ps_samplers[0]);

    // IA setup
    pd3dContext->IASetIndexBuffer(g_pMeshIB, DXGI_FORMAT_R32_UINT, 0);

    ID3D11Buffer* vbs[1] = {g_pMeshVB};
    UINT strides[1] = {sizeof(ocean_vertex)};
    UINT offsets[1] = {0};
    pd3dContext->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);

    pd3dContext->IASetInputLayout(g_pMeshLayout);

    // State blocks
    pd3dContext->RSSetState(g_pRSState_Wireframe);

    // Constants
    ID3D11Buffer* cbs[1] = {g_pShadingCB};
    pd3dContext->VSSetConstantBuffers(2, 1, cbs);
    pd3dContext->PSSetConstantBuffers(2, 1, cbs);
	D3DXCOLOR    colors[2] = { {1.0f,0.0f,0.0f,1.0f},{0.0f,1.0f,0.0f,1.0f} };
    // We assume the center of the ocean surface is at (0, 0, 0).
   // for (int i = 20; i < g_render_list.size(); i++)
	//for (int k = 0; k < g_render_list.size(); ++k)
	//{
	//	auto &node = g_render_list[k];
	//	printf("location:x:%d,y:%d,length:%d,child->%d+%d+%d+%d\n",(int)node.bottom_left.x,(int)node.bottom_left.y,(int)node.length,node.sub_node[0],node.sub_node[1],node.sub_node[2],node.sub_node[3]);
	//}
	//for (int i = 21; i < 22; i++)
	for (int i = 0; i < g_render_list.size(); i++)
    {
        QuadNode& node = g_render_list[i];

        if (!isLeaf(node))
            continue;

        // Check adjacent patches and select mesh pattern
        QuadRenderParam& render_param = selectMeshPattern(node);

        // Find the right LOD to render
        int level_size = g_MeshDim >> node.lod;
        //for (int lod = 0; lod < node.lod; lod++)
        //    level_size >>= 1;

        // Matrices and constants
        Const_Per_Call call_consts;

        // Expand of the local coordinate to world space patch size
        D3DXMATRIX matScale;
        D3DXMatrixScaling(&matScale, node.length / level_size, node.length / level_size, 0);
        D3DXMatrixTranspose(&call_consts.g_matLocal, &matScale);

        // WVP matrix
        D3DXMATRIX matWorld;
        D3DXMatrixTranslation(&matWorld, node.bottom_left.x, node.bottom_left.y, 0);
        D3DXMATRIX matWVP = matWorld * matView * matProj;
        D3DXMatrixTranspose(&call_consts.g_matWorldViewProj, &matWVP);

        // Texcoord for perlin noise
        D3DXVECTOR2 uv_base = node.bottom_left / g_PatchLength * g_PerlinSize;
        call_consts.g_UVBase = uv_base;

        // Constant g_PerlinSpeed need to be adjusted mannually
        D3DXVECTOR2 perlin_move = -g_WindDir * time * g_PerlinSpeed;
        call_consts.g_PerlinMovement = perlin_move;

        // Eye point
        D3DXMATRIX matInvWV = matWorld * matView;
        D3DXMatrixInverse(&matInvWV, nullptr, &matInvWV);
        D3DXVECTOR3 vLocalEye(0, 0, 0);
        D3DXVec3TransformCoord(&vLocalEye, &vLocalEye, &matInvWV);
        call_consts.g_LocalEye = vLocalEye;
		call_consts.g_WireColor = colors[i & 0x1];
        // Update constant buffer
        D3D11_MAPPED_SUBRESOURCE mapped_res;            
        pd3dContext->Map(g_pPerCallCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
        assert(mapped_res.pData);
        *(Const_Per_Call*)mapped_res.pData = call_consts;
        pd3dContext->Unmap(g_pPerCallCB, 0);

        cbs[0] = g_pPerCallCB;
        pd3dContext->VSSetConstantBuffers(4, 1, cbs);
        pd3dContext->PSSetConstantBuffers(4, 1, cbs);

        // Perform draw call
        if (render_param.num_inner_faces > 0)
        {
            // Inner mesh of the patch
            pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            pd3dContext->DrawIndexed(render_param.num_inner_faces + 2, render_param.inner_start_index, 0);
        }

        if (render_param.num_boundary_faces > 0)
        {
            // Boundary mesh of the patch
            pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            pd3dContext->DrawIndexed(render_param.num_boundary_faces * 3, render_param.boundary_start_index, 0);
        }
    }

    // Unbind
    vs_srvs[0] = NULL;
    vs_srvs[1] = NULL;
    pd3dContext->VSSetShaderResources(0, 2, &vs_srvs[0]);

    // Restore states
    pd3dContext->RSSetState(g_pRSState_Solid);
}

void renderLogo(ID3D11DeviceContext* pd3dContext)
{
    // IA setup
    pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    // Shaders
    pd3dContext->VSSetShader( g_pLogoVS, NULL, 0 );
    pd3dContext->PSSetShader( g_pLogoPS, NULL, 0 );

    // Sampler
    pd3dContext->PSSetSamplers( 3, 1, &g_pFresnelSampler );
    //pd3dContext->PSSetShaderResources( 5, 1, &g_pSRV_Logo );

    // State blocks
    ID3D11DepthStencilState* pDepthStencilStateStored11 = NULL;
    UINT StencilRef;
    pd3dContext->OMGetDepthStencilState( &pDepthStencilStateStored11, &StencilRef );
    pd3dContext->OMSetDepthStencilState( g_pDSState_Disable, 0 );

    ID3D11BlendState* pBlendStateStored11 = NULL;
    FLOAT BlendFactor[4];
    UINT SampleMask;
    pd3dContext->OMGetBlendState( &pBlendStateStored11, &BlendFactor[0], &SampleMask );
    pd3dContext->OMSetBlendState( g_pBState_Transparent, &BlendFactor[0], 0xffffffff );

    // Constant
    D3D11_VIEWPORT vp;
    UINT num_vps = 1;
    pd3dContext->RSGetViewports(&num_vps, &vp);

    Const_Per_Frame frame_consts;
    frame_consts.g_LogoPlacement.x = g_LogoWidth / (float)vp.Width * 2;
    frame_consts.g_LogoPlacement.y = g_LogoHeight / (float)vp.Height * 2;
    frame_consts.g_LogoPlacement.z = (float)(vp.Width / 2 - 15 - g_LogoWidth) / vp.Width * 2;
    frame_consts.g_LogoPlacement.w = (float)(15 - vp.Height / 2) / vp.Height * 2;

    D3D11_MAPPED_SUBRESOURCE mapped_res;            
    pd3dContext->Map(g_pPerFrameCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
    assert(mapped_res.pData);
    *(Const_Per_Frame*)mapped_res.pData = frame_consts;
    pd3dContext->Unmap(g_pPerFrameCB, 0);

    ID3D11Buffer* cbs[1] = {g_pPerFrameCB};
    pd3dContext->VSSetConstantBuffers(3, 1, cbs);

    // Draw call
    pd3dContext->Draw( 4, 0 );

    pd3dContext->OMSetDepthStencilState( pDepthStencilStateStored11, StencilRef );
    SAFE_RELEASE(pDepthStencilStateStored11);

    pd3dContext->OMSetBlendState( pBlendStateStored11, &BlendFactor[0], SampleMask );
    SAFE_RELEASE(pBlendStateStored11);
}
