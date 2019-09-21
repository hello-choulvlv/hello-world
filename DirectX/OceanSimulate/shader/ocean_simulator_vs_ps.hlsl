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

#define PI 3.1415926536f
#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16


//---------------------------------------- Vertex Shaders ------------------------------------------
struct VS_QUAD_OUTPUT
{
    float4 Position  : SV_POSITION; // vertex position
    float2 TexCoord  : TEXCOORD0;   // vertex texture coords 
};

VS_QUAD_OUTPUT QuadVS(float4 vPos : POSITION)
{
    VS_QUAD_OUTPUT Output;

    Output.Position = vPos;
	//Output.TexCoord.x = mad(vPos.x, 0.5f, 0.5f);//0.5f + vPos.x * 0.5f; 
	//Output.TexCoord.y =  mad(vPos.y, -0.5f, 0.5f);//0.5f - vPos.y * 0.5f;

	Output.TexCoord = mad(vPos.xy,float2(0.5f,-0.5f),float2(0.5f,0.5f));
    return Output;
}

//----------------------------------------- Pixel Shaders ------------------------------------------

// Textures and sampling states
Texture2D g_samplerDisplacementMap : register(t0);

SamplerState LinearSampler : register(s0);

// Constants
//cbuffer cbImmutable : register(b0)
//{
//    uint g_ActualDim;
//    uint g_InWidth;
//    uint g_OutWidth;
//    uint g_OutHeight;
//    uint g_DxAddressOffset;
//    uint g_DyAddressOffset;
//};

cbuffer  Immutable : register(b0)
{
	uint   g_ActualDim;
	uint   g_InputWidth;
	uint   g_InputHeight;
	uint   g_DxOffset;
	uint   g_DyOffset;
	float  g_PixelWidth;
	float  g_PixelHeight;
	float  g_ChoppyScale;
};

//cbuffer cbChangePerFrame : register(b1)
//{
//    float g_Time;
//    float g_ChoppyScale;
//    float g_GridLen;//512/2000
//};

// The following three should contains only real numbers. But we have only C2C FFT now.
ByteAddressBuffer g_InputDxyz : register(t0);

// Post-FFT data wrap up: Dx, Dy, Dz -> Displacement
float4 UpdateDisplacementPS(VS_QUAD_OUTPUT In) : SV_TARGET
{
    uint index_x = (uint)(In.TexCoord.x * g_InputWidth);
    uint index_y = (uint)(In.TexCoord.y * g_InputHeight);
    uint addr = g_InputWidth * index_y + index_x;

    // cos(pi * (m1 + m2))
    int sign_correction = ((index_x + index_y) & 1) ? -1 : 1;

    float dx = asfloat(g_InputDxyz.Load((addr + g_DxOffset) << 3)) * sign_correction * g_ChoppyScale;
    float dy = asfloat(g_InputDxyz.Load((addr + g_DyOffset) << 3)) * sign_correction * g_ChoppyScale;
    float dz = asfloat(g_InputDxyz.Load(addr << 3)) * sign_correction;

    return float4(dx, dy, dz, 1);
}

// Displacement -> Normal, Folding
float4 GenGradientFoldingPS(VS_QUAD_OUTPUT In) : SV_TARGET
{
    // Sample neighbour texels
    //float2 one_texel = float2(1.0f / (float)g_OutWidth, 1.0f / (float)g_OutHeight);

    //float2 tc_left  = float2(In.TexCoord.x - one_texel.x, In.TexCoord.y);
    //float2 tc_right = float2(In.TexCoord.x + one_texel.x, In.TexCoord.y);
    //float2 tc_back  = float2(In.TexCoord.x, In.TexCoord.y - one_texel.y);
    //float2 tc_front = float2(In.TexCoord.x, In.TexCoord.y + one_texel.y);

    float3 displace_left  = g_samplerDisplacementMap.Sample(LinearSampler, In.TexCoord - float2(g_PixelWidth,0)).xyz;
    float3 displace_right = g_samplerDisplacementMap.Sample(LinearSampler, In.TexCoord + float2(g_PixelWidth,0)).xyz;
    float3 displace_back  = g_samplerDisplacementMap.Sample(LinearSampler, In.TexCoord - float2(0,g_PixelHeight)).xyz;
    float3 displace_front = g_samplerDisplacementMap.Sample(LinearSampler, In.TexCoord + float2(0,g_PixelHeight)).xyz;

    // Do not store the actual normal value. Using gradient instead, which preserves two differential values.
    //float2 gradient = {-(displace_right.z - displace_left.z), -(displace_front.z - displace_back.z)};
    // Calculate Jacobian corelation from the partial differential of height field
    //float2 Dx = (displace_right.xy - displace_left.xy) * g_ChoppyScale * g_GridLen;
   // float2 Dy = (displace_front.xy - displace_back.xy) * g_ChoppyScale * g_GridLen;
	//solve Jacobian determinate
   // float J = (1.0f + Dx.x) * (1.0f + Dy.y) - Dx.y * Dy.x;

    // Practical subsurface scale calculation: max[0, (1 - J) + Amplitude * (2 * Coverage - 1)].
   // float fold = max(1.0f - J, 0);

    // Output
    //return float4(gradient, 0, 1.0f);
	return float4(displace_left.z - displace_right.z, displace_back.z - displace_front.z,0,1.0f);
}
