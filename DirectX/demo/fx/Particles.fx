/*
  *2018/3/21
  *@author:xiaohuaxiong
  *Flare
 */
cbuffer BufferConstantStream
{
	float3        g_EyePosition;
	float3        g_EmitterPosition;
	float         g_TotalTime;
	float         g_TimeInterval;
	float4x4      g_ViewProjMatrix;
};

cbuffer BufferFixedConstant
{
	float3      g_Accelerate = float3(0.0f,7.8f,0.0f);
	float2      g_QuadFragCoord[4] = {float2(0.0f,1.0f),float2(0.0f,0.0f),float2(1.0f,1.0f),float2(1.0f,0.0f)};
};

Texture2D     g_BaseTexture;
Texture1D     g_RandomTexture;

SamplerState  SamplerNormal
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

DepthStencilState  DisableDepth
{
	DepthEnable        = false;
	DepthWriteMask = ZERO;
};

DepthStencilState NoDepthWrite
{
	DepthEnable        = TRUE;
	DepthWriteMask = ZERO;
};

RasterizerState NoCull
{
	CullMode = None;
};

DepthStencilState LessEqualDSS
{
	// Make sure the depth function is LESS_EQUAL and not just LESS.  
	// Otherwise, the normalized depth values at z = 1 (NDC) will 
	// fail the depth test if the depth buffer was cleared to 1.
	DepthFunc = LESS_EQUAL;
};

BlendState   FlareBlendState
{
	AlphaToCoverageEnable = false;
	BlendEnable[0] = true;
	SrcBlend = SRC_ALPHA;
	DestBlend = ONE;
	BlendOp = ADD;
	SrcBlendAlpha = ZERO;
	DestBlendAlpha = ONE;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0xF;
};

float3   randomFloat3(float offset)
{
	float3  randVec3 = g_RandomTexture.SampleLevel(SamplerNormal, offset + g_TotalTime,0).xyz;
	return normalize(randVec3);
}

#define PARTICLE_TYPE_EMMITER   0
#define PARTICLE_TYPE_FLARE          1

struct ParticleVertex
{
	float3      originPosition : POSITION;
	float3      velocity : VELOCITY;
	float2      size        : SIZE;
	float        age : AGE;
	uint         type : TYPE;
};

ParticleVertex  stream_vs_main(ParticleVertex vsIn)
{
	return vsIn;
}
//Geometry Shader
[MaxVertexCount(2)]  
void stream_gs_main(point ParticleVertex gsIn[1], inout PointStream<ParticleVertex> pointOut)
{
	gsIn[0].age += g_TimeInterval;
	if (gsIn[0].type == PARTICLE_TYPE_EMMITER)
	{
		if (gsIn[0].age > 0.005f)
		{
			float3  randVec3 = randomFloat3(0.0f) * float3(0.5f, 1.0f, 0.5f);
			ParticleVertex   otherVertex;
			otherVertex.originPosition = g_EmitterPosition;
			otherVertex.velocity = randVec3 * 4.0f;
			otherVertex.size = float2(3.0f, 3.0f);
			otherVertex.age = 0.0f;
			otherVertex.type = PARTICLE_TYPE_FLARE;

			pointOut.Append(otherVertex);
			gsIn[0].age = 0.0f;
		}
		pointOut.Append(gsIn[0]);
	}
	else
		if(gsIn[0].age <= 1.0f)
			pointOut.Append(gsIn[0]);
}

GeometryShader  gsStreamShader = ConstructGSWithSO(CompileShader(gs_4_0, stream_gs_main()),
	"POSITION.xyz;VELOCITY.xyz;SIZE.xy;AGE.x;TYPE.x");

technique10 StreamTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, stream_vs_main()));
		SetGeometryShader(gsStreamShader);
		SetPixelShader(NULL);

		SetDepthStencilState(DisableDepth,0);
	}
}
////////////////////////////////Normal-Render-Shader/////////////////
struct VS_STREAM_OUT
{
	float3      position : POSITION;
	float2      size         :SIZE;
	float4      color      :COLOR;
	uint        type       :TYPE;
};

struct GS_STREAM_OUT
{
	float4 position : SV_POSITION;
	float4 color       : COLOR;
	float2 fragCoord : TEXCOORD;
};

VS_STREAM_OUT    vs_main(ParticleVertex vsIn)
{
	VS_STREAM_OUT   gsOut;
	gsOut.position = vsIn.originPosition + 0.5f * vsIn.age * vsIn.age * g_Accelerate + vsIn.velocity * vsIn.age;
	gsOut.color = float4(1.0f,1.0f,1.0f,1.0f - smoothstep(0.0f,1.0f,vsIn.age));
	gsOut.size = vsIn.size;
	gsOut.type = vsIn.type;
	return gsOut;
}

[MaxVertexCount(4)]
void  gs_main(point VS_STREAM_OUT  gsIn[1],inout TriangleStream<GS_STREAM_OUT> gsStreamOut)
{
	if(gsIn[0].type != PARTICLE_TYPE_EMMITER)
	{
		GS_STREAM_OUT gsOut;

		float3   zNormal = normalize(gsIn[0].position - g_EyePosition);
		float3   xNormal = normalize(cross(float3(0.0f,1.0f,0.0f),zNormal));
		float3   yNormal = cross(zNormal,xNormal);

		float4x4 rotate_matrix;
		rotate_matrix[0] = float4(xNormal,0.0f);
		rotate_matrix[1] = float4(yNormal,0.0f);
		rotate_matrix[2] = float4(zNormal,0.0f);
		rotate_matrix[3] = float4(gsIn[0].position,1.0f);

		float4x4 mvp_matrix = mul(rotate_matrix , g_ViewProjMatrix);
		float2   halfSize = gsIn[0].size * 0.5f;

		float4   vertex_quad[4] = {
			float4(-halfSize.x,-halfSize.y,0.0f,1.0f),//left bottom
			float4(-halfSize.x,halfSize.y,0.0f,1.0f),//left top
			float4(halfSize.x,-halfSize.y,0.0f,1.0f),//right bottom
			float4(halfSize.x,halfSize.y,0.0f,1.0f),
		};
		[unroll]
		for(int k=0; k <4 ;++k)
		{
			gsOut.position  = mul(vertex_quad[k],mvp_matrix);
			gsOut.color     = gsIn[0].color;
			gsOut.fragCoord = g_QuadFragCoord[k];

			gsStreamOut.Append(gsOut);
		}
	}
}

float4  ps_main(GS_STREAM_OUT   gsIn) : SV_TARGET
{
	return g_BaseTexture.Sample(SamplerNormal,gsIn.fragCoord) * gsIn.color;
}

technique10 ColorTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0,vs_main()));
		SetGeometryShader(CompileShader(gs_4_0,gs_main()));
		SetPixelShader(CompileShader(ps_4_0,ps_main()));

		SetBlendState(FlareBlendState,float4(0.0f,0.0f,0.0f,0.0f),0xFFFFFFFF);
		SetDepthStencilState(NoDepthWrite,0);
	}
}
