#pragma once

//该文件是描述并处理每个自定义顶点物体的，函数调用顺序如下：
//初始化：先自定义顶点类型，然后初始化顶点缓冲或索引缓冲，读入纹理
//渲染前：设置纹理，设置材质
//注意创建顶点缓冲时传入的数据指针是一个指向struct数组的指针，索引缓冲是一个WORD数组的指针，还有一定要先设置正确的D3D_FVF，就算你后边要改用VS，也必须先设置，因为初始化的时候会用到

/*顶点声明模板
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
	DWORD m_FVF;         //自定义顶点格式，用D3DFVF_
	UINT TextureNum;            //已创建的纹理数，初始为0
	D3DMATERIAL9 Material;        //材质

	EVERYMODULE();
	~EVERYMODULE();
	//得到本模型从开始到现在的时间，单位秒（GETTICKCOUNT返回值1个单位约为55毫秒）
	float GetTime();
	void Release();

	//将指定顶点数据拷入显存内的顶点缓冲区，BufferPoint是顶点数据数组的指针，
	//PointNum是表示有多少个顶点，EPointSize是每个顶点数据的大小，一般是sizeof(CUSTOMVERTEX)，
	//即VertexShader所定义的顶点的储存大小
	HRESULT InitVertexBuffer(BYTE* BufferPoint, UINT PointNum, UINT EPointSize);
	//将指定顶点数据和索引数据拷入显存内的顶点缓冲区和索引缓冲区，BufferPoint是顶点数据数组的指针，PointNum是表示有多少个顶点，IndexNum表示有多少个索引，EPointSize是每个顶点数据的大小，一般是sizeof(CUSTOMVERTEX)，即VertexShader所定义的顶点的储存大小
	HRESULT InitIndexVertexBuffer(BYTE* VertexBufferPoint, WORD* IndexBufferPoint, UINT PointNum, UINT IndexNum, UINT EPointSize);

	//重载函数，用户可自定义已创建的顶点缓冲区，需要指定每个顶点的存储大小，执行DRAWPRIMITIVE方法时自动渲染自定义的顶点数据
	HRESULT InitVertexBuffer(LPDIRECT3DVERTEXBUFFER9 UserVertexBuffer, UINT EPointSize);
	//用户可自定义已创建的索引缓冲区，可以和上面的自定义顶点缓冲区函数连用来渲染MESH
	HRESULT InitIndexBuffer(LPDIRECT3DINDEXBUFFER9 UserIndexBuffer);

	//得到顶点缓冲区的数据流指针，第二个参数是LOCK多少个顶点
	HRESULT GetVertexBuffer(BYTE **p, UINT VertexNum);
	HRESULT UnlockVertexBuffer();
	//得到索引缓冲区的数据流指针，第二个参数是LOCK多少个索引
	HRESULT GetIndexBuffer(WORD **p, UINT IndexNum);
	HRESULT UnlockIndexBuffer();

	//创建第TEXTURENUM个纹理，纹理总数自动加1，不能超过最大可创建纹理数
	HRESULT CreateTextureFromFile(LPSTR Filename, D3DFORMAT Format = D3DFMT_A8R8G8B8);
	//设置纹理，将第NO(从0开始)个纹理设置为第STAGE层，超出已创建纹理个数则失败
	HRESULT SetTexture(DWORD Stage, UINT TextureNo);
	//根据参数给该物体的材质赋值，但注意并不设置材质
	void SetMaterial(float AmbientR, float AmbientG, float AmbientB, float AmbientA, float DiffuseR, float DiffuseG, float DiffuseB, float DiffuseA, float SpecularR, float SpecularG, float SpecularB, float SpecularA, float EmissiveR, float EmissiveG, float EmissiveB, float EmissiveA, float Power);
	//设置VertexShader，用于VS，因为FVF和VS分开了，所以渲染完要恢复，跟PIXELSHADER用法一样
	void SetVertexShader(LPDIRECT3DVERTEXSHADER9 vs, LPDIRECT3DVERTEXDECLARATION9 decl);
	void RestoreVertexShader();
	//设置该物体的顶点数据为StreamNo号流，用于VS。你可以把一个物体分为几个EVERYMODULE，每个MODULE存储一部分顶点数据，并用这个函数来设置
	//注意如果使用了此函数，就不要用下面的渲染函数DrawPrimitive了。
	HRESULT SetStreamSource(UINT StreamNo);
	HRESULT SetIndices();
	//设置材质、顶点缓冲、顶点格式，然后渲染该物体，参数和D3DDEVICE的函数参数完全相同
	//如果使用了VS，就不要用这个函数来渲染
	HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	//注意同上，第一、二个参数是顶点和索引的起始值，第三个是顶点缓冲区中的渲染顶点数
	//不需要TYPE，因为索引渲染必定是TRIANGLELIST
	// 这个函数必须指定VertexNum，否则会造成在A卡上出错（事实上本身就是错误的，只是N卡会强制执行而已，用下DX DEBUG就知道了）
	HRESULT DrawIndexPrimitive(UINT StartIndex, UINT PrimitiveCount, UINT iVertexNum);
	
	HRESULT DrawMesh(LPD3DXMESH newmesh, UINT EPointSize);

private:
	bool m_bSetVertexShader;	//如果是设置的VERTEXSHADER类中的HANDLE和DECL，这里要置位，释放时就不能释放它们，否则重复释放会出问题
	DWORD Time; //时间控制
	UINT EachPointSize;   //每个顶点所占存储空间的大小，一般是sizeof(CUSTOMVERTEX)
	UINT VertexNum, IndexVertexNum;  //顶点数量和索引数量
	UINT CreateAttrib;  //属性，0表示未初始化，1表示顶点缓冲，2表示索引顶点缓冲
	LPDIRECT3DVERTEXSHADER9 VertexShader, OldVertexShader;
	LPDIRECT3DVERTEXDECLARATION9 Declaration, OldDeclaration;
	LPDIRECT3DVERTEXBUFFER9 VertexBuffer;  //顶点缓冲区
	LPDIRECT3DINDEXBUFFER9 IndexBuffer;  //索引缓冲区
	LPDIRECT3DTEXTURE9 Texture[MYD3D_MODULETEXTURENUM];   //纹理，最多8个，和层数无关
};
