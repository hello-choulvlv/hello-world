#pragma once

//���ļ�������������ÿ���Զ��嶥������ģ���������˳�����£�
//��ʼ�������Զ��嶥�����ͣ�Ȼ���ʼ�����㻺����������壬��������
//��Ⱦǰ�������������ò���
//ע�ⴴ�����㻺��ʱ���������ָ����һ��ָ��struct�����ָ�룬����������һ��WORD�����ָ�룬����һ��Ҫ��������ȷ��D3D_FVF����������Ҫ����VS��Ҳ���������ã���Ϊ��ʼ����ʱ����õ�

/*��������ģ��
D3DVERTEXELEMENT9 dclr[] = {
	{0,0,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0,12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	{0,12+12,D3DDECLTYPE_D3DCOLOR ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{0,12+12+4,D3DDECLTYPE_FLOAT2 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	{0,12+12+4+8,D3DDECLTYPE_FLOAT2 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},

	{1,0,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1},
	{1,12,D3DDECLTYPE_FLOAT3 ,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 1},
	D3DDECL_END()
};*/
class EVERYMODULE
{
public:
	LPD3DXMESH Mesh;
	DWORD m_FVF;         //�Զ��嶥���ʽ����D3DFVF_
	UINT TextureNum;            //�Ѵ���������������ʼΪ0
	D3DMATERIAL9 Material;        //����

	EVERYMODULE();
	~EVERYMODULE();
	//�õ���ģ�ʹӿ�ʼ�����ڵ�ʱ�䣬��λ�루GETTICKCOUNT����ֵ1����λԼΪ55���룩
	float GetTime();
	void Release();

	//��ָ���������ݿ����Դ��ڵĶ��㻺������BufferPoint�Ƕ������������ָ�룬
	//PointNum�Ǳ�ʾ�ж��ٸ����㣬EPointSize��ÿ���������ݵĴ�С��һ����sizeof(CUSTOMVERTEX)��
	//��VertexShader������Ķ���Ĵ����С
	HRESULT InitVertexBuffer(BYTE* BufferPoint, UINT PointNum, UINT EPointSize);
	//��ָ���������ݺ��������ݿ����Դ��ڵĶ��㻺������������������BufferPoint�Ƕ������������ָ�룬PointNum�Ǳ�ʾ�ж��ٸ����㣬IndexNum��ʾ�ж��ٸ�������EPointSize��ÿ���������ݵĴ�С��һ����sizeof(CUSTOMVERTEX)����VertexShader������Ķ���Ĵ����С
	HRESULT InitIndexVertexBuffer(BYTE* VertexBufferPoint, WORD* IndexBufferPoint, UINT PointNum, UINT IndexNum, UINT EPointSize);

	//���غ������û����Զ����Ѵ����Ķ��㻺��������Ҫָ��ÿ������Ĵ洢��С��ִ��DRAWPRIMITIVE����ʱ�Զ���Ⱦ�Զ���Ķ�������
	HRESULT InitVertexBuffer(LPDIRECT3DVERTEXBUFFER9 UserVertexBuffer, UINT EPointSize);
	//�û����Զ����Ѵ��������������������Ժ�������Զ��嶥�㻺����������������ȾMESH
	HRESULT InitIndexBuffer(LPDIRECT3DINDEXBUFFER9 UserIndexBuffer);

	//�õ����㻺������������ָ�룬�ڶ���������LOCK���ٸ�����
	HRESULT GetVertexBuffer(BYTE **p, UINT VertexNum);
	HRESULT UnlockVertexBuffer();
	//�õ�������������������ָ�룬�ڶ���������LOCK���ٸ�����
	HRESULT GetIndexBuffer(WORD **p, UINT IndexNum);
	HRESULT UnlockIndexBuffer();

	//������TEXTURENUM���������������Զ���1�����ܳ������ɴ���������
	HRESULT CreateTextureFromFile(LPSTR Filename, D3DFORMAT Format = D3DFMT_A8R8G8B8);
	//������������NO(��0��ʼ)����������Ϊ��STAGE�㣬�����Ѵ������������ʧ��
	HRESULT SetTexture(DWORD Stage, UINT TextureNo);
	//���ݲ�����������Ĳ��ʸ�ֵ����ע�Ⲣ�����ò���
	void SetMaterial(float AmbientR, float AmbientG, float AmbientB, float AmbientA, float DiffuseR, float DiffuseG, float DiffuseB, float DiffuseA, float SpecularR, float SpecularG, float SpecularB, float SpecularA, float EmissiveR, float EmissiveG, float EmissiveB, float EmissiveA, float Power);
	//����VertexShader������VS����ΪFVF��VS�ֿ��ˣ�������Ⱦ��Ҫ�ָ�����PIXELSHADER�÷�һ��
	void SetVertexShader(LPDIRECT3DVERTEXSHADER9 vs, LPDIRECT3DVERTEXDECLARATION9 decl);
	void RestoreVertexShader();
	//���ø�����Ķ�������ΪStreamNo����������VS������԰�һ�������Ϊ����EVERYMODULE��ÿ��MODULE�洢һ���ֶ������ݣ������������������
	//ע�����ʹ���˴˺������Ͳ�Ҫ���������Ⱦ����DrawPrimitive�ˡ�
	HRESULT SetStreamSource(UINT StreamNo);
	HRESULT SetIndices();
	//���ò��ʡ����㻺�塢�����ʽ��Ȼ����Ⱦ�����壬������D3DDEVICE�ĺ���������ȫ��ͬ
	//���ʹ����VS���Ͳ�Ҫ�������������Ⱦ
	HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	//ע��ͬ�ϣ���һ�����������Ƕ������������ʼֵ���������Ƕ��㻺�����е���Ⱦ������
	//����ҪTYPE����Ϊ������Ⱦ�ض���TRIANGLELIST
	// �����������ָ��VertexNum������������A���ϳ�����ʵ�ϱ�����Ǵ���ģ�ֻ��N����ǿ��ִ�ж��ѣ�����DX DEBUG��֪���ˣ�
	HRESULT DrawIndexPrimitive(UINT StartIndex, UINT PrimitiveCount, UINT iVertexNum);
	
	HRESULT DrawMesh(LPD3DXMESH newmesh, UINT EPointSize);

private:
	bool m_bSetVertexShader;	//��������õ�VERTEXSHADER���е�HANDLE��DECL������Ҫ��λ���ͷ�ʱ�Ͳ����ͷ����ǣ������ظ��ͷŻ������
	DWORD Time; //ʱ�����
	UINT EachPointSize;   //ÿ��������ռ�洢�ռ�Ĵ�С��һ����sizeof(CUSTOMVERTEX)
	UINT VertexNum, IndexVertexNum;  //������������������
	UINT CreateAttrib;  //���ԣ�0��ʾδ��ʼ����1��ʾ���㻺�壬2��ʾ�������㻺��
	LPDIRECT3DVERTEXSHADER9 VertexShader, OldVertexShader;
	LPDIRECT3DVERTEXDECLARATION9 Declaration, OldDeclaration;
	LPDIRECT3DVERTEXBUFFER9 VertexBuffer;  //���㻺����
	LPDIRECT3DINDEXBUFFER9 IndexBuffer;  //����������
	LPDIRECT3DTEXTURE9 Texture[MYD3D_MODULETEXTURENUM];   //�������8�����Ͳ����޹�
};
