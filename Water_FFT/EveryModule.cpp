#include "Myd3d.h"
#include "EveryModule.h"

EVERYMODULE::EVERYMODULE()
{
	m_FVF = 0;
	EachPointSize = 3*sizeof(float);
	for(int i=0;i<MYD3D_MODULETEXTURENUM;i++) Texture[i] = NULL;
	TextureNum = 0;
	CreateAttrib = 0;
	ZeroMemory(&Material, sizeof(D3DMATERIAL9));
	IndexBuffer = NULL; VertexBuffer = NULL;
	Mesh = NULL;
	VertexShader = NULL; OldVertexShader = NULL;
	Declaration = NULL; OldDeclaration = NULL;
	VertexNum = 0; IndexVertexNum = 0;
	m_bSetVertexShader = false;
}


void EVERYMODULE::Release()
{
	//释放纹理
	for(UINT i=0;i<TextureNum;i++) SAFE_RELEASE(Texture[i]);
	//释放索引缓冲区
	SAFE_RELEASE(IndexBuffer);
	//释放顶点缓冲区
	SAFE_RELEASE(VertexBuffer);
	//释放模型
	SAFE_RELEASE(Mesh);
	//释放Shader
	if(!m_bSetVertexShader)
	{
		SAFE_RELEASE(Declaration);
		SAFE_RELEASE(VertexShader);	
	}
	else
	{
		VertexShader = NULL;
		Declaration = NULL;
	}
	SAFE_RELEASE(OldDeclaration);
	SAFE_RELEASE(OldVertexShader);
	
	
	m_bSetVertexShader = false;
	
	m_FVF = 0;
	TextureNum = 0;
	CreateAttrib = 0;
	VertexNum = 0; IndexVertexNum = 0;
}

EVERYMODULE::~EVERYMODULE()
{
	Release();
}

float EVERYMODULE::GetTime()
{
	return (GetTickCount()-Time)/18.2f;
}


HRESULT EVERYMODULE::InitVertexBuffer(BYTE* BufferPoint, UINT PointNum, UINT EPointSize)
{
	if(CreateAttrib || m_FVF==0 || m_FVF==0xffffffff) return E_FAIL;
	VOID* VertexStream;         //顶点缓冲区数据流指针，创建时拷贝数据用
	EachPointSize = EPointSize;
	VertexNum = PointNum;
	
	//创建顶点缓冲区，将数据拷入
    if(FAILED(d3ddevice->CreateVertexBuffer(PointNum*EachPointSize, D3DUSAGE_WRITEONLY, m_FVF, D3DPOOL_DEFAULT, &VertexBuffer, NULL)))
		return E_FAIL;

	if(VertexBuffer->Lock(0, PointNum*EachPointSize, &VertexStream, 0))
		return E_FAIL;
	memcpy(VertexStream, BufferPoint, PointNum*EachPointSize);
	VertexBuffer->Unlock();

	//初始化MESH
/*
	if(FAILED(D3DXCreateMeshFVF(PointNum-2, PointNum, D3DXMESH_VB_MANAGED, m_FVF, d3ddevice, &Mesh)))
		return E_FAIL;
	Mesh->LockVertexBuffer(0, &VertexStream);
	memcpy(VertexStream, BufferPoint, PointNum*EachPointSize);
	Mesh->UnlockVertexBuffer();
*/	
	CreateAttrib = 1;
	return S_OK;
}


HRESULT EVERYMODULE::InitIndexVertexBuffer(BYTE* VertexBufferPoint, WORD* IndexBufferPoint, UINT PointNum, UINT IndexNum, UINT EPointSize)
{
	if(CreateAttrib || m_FVF==0 || m_FVF==0xffffffff) return E_FAIL;
	VOID* VertexStream;         //顶点缓冲区数据流指针，创建时拷贝数据用
	VOID* IndexStream;          //索引缓冲区数据流指针，创建时拷贝数据用
	EachPointSize = EPointSize;
	VertexNum = PointNum;
	IndexVertexNum = IndexNum;

	//创建顶点缓冲区，将数据拷入
    if(FAILED(d3ddevice->CreateVertexBuffer(PointNum*EachPointSize, D3DUSAGE_WRITEONLY, m_FVF, D3DPOOL_DEFAULT, &VertexBuffer, NULL)))
		return E_FAIL;

	if(VertexBuffer->Lock(0, PointNum*EachPointSize, &VertexStream, 0))
		return E_FAIL;
	memcpy(VertexStream, VertexBufferPoint, PointNum*EachPointSize);
	VertexBuffer->Unlock();

	//创建索引缓冲区，将数据拷入
	if(FAILED(d3ddevice->CreateIndexBuffer(IndexNum*sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &IndexBuffer, NULL)))
		return E_FAIL;

	if(FAILED(IndexBuffer->Lock(0, IndexNum*sizeof(WORD), &IndexStream, 0)))
		return E_FAIL;
	memcpy(IndexStream, (BYTE *)IndexBufferPoint, IndexNum*sizeof(WORD));
	IndexBuffer->Unlock();

	//初始化MESH
/*	if(FAILED(D3DXCreateMeshFVF(IndexNum/3, PointNum, D3DXMESH_MANAGED, m_FVF, d3ddevice, &Mesh)))
		return E_FAIL;
	Mesh->LockVertexBuffer(0, &VertexStream);
	memcpy(VertexStream, VertexBufferPoint, PointNum*EachPointSize);
	Mesh->UnlockVertexBuffer();

	Mesh->LockIndexBuffer(0, &IndexStream);
	memcpy(IndexStream, (BYTE *)IndexBufferPoint, IndexNum*sizeof(WORD));
	Mesh->UnlockIndexBuffer();
*/
	CreateAttrib = 2;
	return S_OK;
}


HRESULT EVERYMODULE::InitVertexBuffer(LPDIRECT3DVERTEXBUFFER9 UserVertexBuffer, UINT EPointSize)
{
	EachPointSize = EPointSize;
	
	//设定自定义顶点缓冲区
    if(UserVertexBuffer) VertexBuffer=UserVertexBuffer;
	else return E_FAIL;
	CreateAttrib = 1;
	return S_OK;
}

HRESULT EVERYMODULE::InitIndexBuffer(LPDIRECT3DINDEXBUFFER9 UserIndexBuffer)
{
	//设定自定义索引缓冲区
    if(UserIndexBuffer) IndexBuffer=UserIndexBuffer;
	else return E_FAIL;
	CreateAttrib = 2;
	return S_OK;
}

HRESULT EVERYMODULE::GetVertexBuffer(BYTE **p, UINT VertexNum)
{
	if(CreateAttrib==0) return E_FAIL;
	return VertexBuffer->Lock(0, VertexNum*EachPointSize, (VOID **)p, 0);
}

HRESULT EVERYMODULE::UnlockVertexBuffer()
{
	return VertexBuffer->Unlock();
}

HRESULT EVERYMODULE::GetIndexBuffer(WORD **p, UINT IndexNum)
{
	if(CreateAttrib!=2) return E_FAIL;
	return IndexBuffer->Lock(0, IndexNum*sizeof(WORD), (VOID **)p, 0);
}

HRESULT EVERYMODULE::UnlockIndexBuffer()
{
	return IndexBuffer->Unlock();
}


HRESULT EVERYMODULE::CreateTextureFromFile(LPSTR Filename, D3DFORMAT Format /* = D3DFMT_A8R8G8B8 */)
{
	if(TextureNum==MYD3D_MODULETEXTURENUM) return E_FAIL;
	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, Filename, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, D3DUSAGE_0, Format, D3DPOOL_DEFAULT, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0xffffffff, NULL, NULL, &(Texture[TextureNum]))))
		return E_FAIL;
	else TextureNum++;
	return S_OK;
}


HRESULT EVERYMODULE::SetTexture(DWORD Stage, UINT TextureNo)
{
	if(TextureNo>=TextureNum) return E_FAIL;
	return d3ddevice->SetTexture(Stage, Texture[TextureNo]);
}

void EVERYMODULE::SetVertexShader(LPDIRECT3DVERTEXSHADER9 vs, LPDIRECT3DVERTEXDECLARATION9 decl)
{
	SAFE_RELEASE(OldDeclaration);
	SAFE_RELEASE(OldVertexShader);
	d3ddevice->GetVertexDeclaration(&OldDeclaration);
	d3ddevice->GetVertexShader(&OldVertexShader);
	VertexShader = vs;
	Declaration = decl;
	m_bSetVertexShader = true;
}

void EVERYMODULE::RestoreVertexShader()
{
	d3ddevice->SetVertexDeclaration(OldDeclaration);
	d3ddevice->SetVertexShader(OldVertexShader);
	SAFE_RELEASE(OldDeclaration);
	SAFE_RELEASE(OldVertexShader);
}

void EVERYMODULE::SetMaterial(float AmbientR, float AmbientG, float AmbientB, float AmbientA, float DiffuseR, float DiffuseG, float DiffuseB, float DiffuseA, float SpecularR, float SpecularG, float SpecularB, float SpecularA, float EmissiveR, float EmissiveG, float EmissiveB, float EmissiveA, float Power)
{
	Material.Diffuse.r = DiffuseR;
	Material.Diffuse.g = DiffuseG;
	Material.Diffuse.b = DiffuseB;
	Material.Diffuse.a = DiffuseA;
	Material.Ambient.r = AmbientR;  //一般材质反射和漫反射光线颜色相同，如果不同的等调用完函数后再自己设置
	Material.Ambient.g = AmbientG;
	Material.Ambient.b = AmbientB;
	Material.Ambient.a = AmbientA;
	Material.Specular.r= SpecularR;
	Material.Specular.g= SpecularG;
	Material.Specular.b= SpecularB;
	Material.Specular.a= SpecularA;
	Material.Emissive.r= EmissiveR;
	Material.Emissive.g= EmissiveG;
	Material.Emissive.b= EmissiveB;
	Material.Emissive.a= EmissiveA;
	Material.Power     = Power;
}

HRESULT EVERYMODULE::SetStreamSource(UINT StreamNo)
{
	return d3ddevice->SetStreamSource(StreamNo, VertexBuffer, 0, EachPointSize);
}

HRESULT EVERYMODULE::SetIndices()
{
	return d3ddevice->SetIndices(IndexBuffer);
}



HRESULT EVERYMODULE::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	HRESULT att=S_OK;
	if(CreateAttrib!=1 || m_FVF==0 || m_FVF==0xffffffff) return E_FAIL;
	d3ddevice->SetStreamSource(0, VertexBuffer, 0, EachPointSize);
	if(VertexShader && Declaration)
	{
		d3ddevice->SetVertexDeclaration(Declaration);
		d3ddevice->SetVertexShader(VertexShader);
	}
	else
	{
//		d3ddevice->SetVertexDeclaration(NULL);
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetFVF(m_FVF);
	}
	d3ddevice->SetMaterial(&Material);
	d3ddevice->BeginScene();
	if(FAILED(d3ddevice->DrawPrimitive(PrimitiveType ,StartVertex ,PrimitiveCount)))
		att=E_FAIL;
	d3ddevice->EndScene();
	return att;
}

HRESULT EVERYMODULE::DrawIndexPrimitive(UINT StartIndex, UINT PrimitiveCount, UINT iVertexNum)
{
	HRESULT att=S_OK;
	if(CreateAttrib!=2 || m_FVF==0 || m_FVF==0xffffffff) return E_FAIL;
	d3ddevice->SetStreamSource(0, VertexBuffer, 0, EachPointSize);
	d3ddevice->SetIndices(IndexBuffer);
	if(VertexShader && Declaration)
	{
		d3ddevice->SetVertexDeclaration(Declaration);
		d3ddevice->SetVertexShader(VertexShader);
	}
	else
	{
//		d3ddevice->SetVertexDeclaration(NULL);
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetFVF(m_FVF);
	}
	d3ddevice->SetMaterial(&Material);
	d3ddevice->BeginScene();
	if(FAILED(d3ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, iVertexNum, StartIndex ,PrimitiveCount)))
		att=E_FAIL;
	d3ddevice->EndScene();
	return att;
}

HRESULT EVERYMODULE::DrawMesh(LPD3DXMESH newmesh, UINT EPointSize)
{

	if(!Mesh)
		return E_FAIL;
	HRESULT att=S_OK;
	LPDIRECT3DVERTEXBUFFER9 vertexbuffer;
	LPDIRECT3DINDEXBUFFER9 indexbuffer;
	if(newmesh==NULL)
	{
		if(CreateAttrib==1)
		{
			Mesh->GetVertexBuffer(&vertexbuffer);
			d3ddevice->SetStreamSource(0, vertexbuffer, 0, EachPointSize);
			if(VertexShader && Declaration)
			{
				d3ddevice->SetVertexDeclaration(Declaration);
				d3ddevice->SetVertexShader(VertexShader);
			}
			else
			{
//				d3ddevice->SetVertexDeclaration(NULL);
				d3ddevice->SetVertexShader(NULL);
				d3ddevice->SetFVF(m_FVF);
			}
			d3ddevice->SetMaterial(&Material);
			d3ddevice->BeginScene();
			if(FAILED(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, VertexNum-2)))
				att=E_FAIL;
			d3ddevice->EndScene();
		}
		else if(CreateAttrib==2)
		{
			Mesh->GetVertexBuffer(&vertexbuffer);
			Mesh->GetIndexBuffer(&indexbuffer);
			d3ddevice->SetStreamSource(0, vertexbuffer, 0, EachPointSize);
			d3ddevice->SetIndices(indexbuffer);
			if(VertexShader && Declaration)
			{
				d3ddevice->SetVertexDeclaration(Declaration);
				d3ddevice->SetVertexShader(VertexShader);
			}
			else
			{
//				d3ddevice->SetVertexDeclaration(NULL);
				d3ddevice->SetVertexShader(NULL);
				d3ddevice->SetFVF(m_FVF);
			}
			d3ddevice->SetMaterial(&Material);
			d3ddevice->BeginScene();
			if(FAILED(d3ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VertexNum, 0, IndexVertexNum/3)))
				att=E_FAIL;
			d3ddevice->EndScene();
		}
	}
	else
	{
		if(CreateAttrib==1)
		{
			newmesh->GetVertexBuffer(&vertexbuffer);
			d3ddevice->SetStreamSource(0, vertexbuffer, 0, EPointSize);
			if(VertexShader && Declaration)
			{
				d3ddevice->SetVertexDeclaration(Declaration);
				d3ddevice->SetVertexShader(VertexShader);
			}
			else
			{
//				d3ddevice->SetVertexDeclaration(NULL);
				d3ddevice->SetVertexShader(NULL);
				d3ddevice->SetFVF(m_FVF);
			}
			d3ddevice->SetMaterial(&Material);
			d3ddevice->BeginScene();
			if(FAILED(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, VertexNum-2)))
				att=E_FAIL;
			d3ddevice->EndScene();
		}
		else if(CreateAttrib==2)
		{
			newmesh->GetVertexBuffer(&vertexbuffer);
			newmesh->GetIndexBuffer(&indexbuffer);
			d3ddevice->SetStreamSource(0, vertexbuffer, 0, EPointSize);
			d3ddevice->SetIndices(indexbuffer);
			if(VertexShader && Declaration)
			{
				d3ddevice->SetVertexDeclaration(Declaration);
				d3ddevice->SetVertexShader(VertexShader);
			}
			else
			{
//				d3ddevice->SetVertexDeclaration(NULL);
				d3ddevice->SetVertexShader(NULL);
				d3ddevice->SetFVF(m_FVF);
			}
			d3ddevice->SetMaterial(&Material);
			d3ddevice->BeginScene();
			if(FAILED(d3ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VertexNum, 0, IndexVertexNum/3)))
				att=E_FAIL;
			d3ddevice->EndScene();
		}
	}
	return att;
}