#pragma once

//有两种用法：都不用设置SetVertexShader哦
//			  1、单STREAM，先用物体SetVertexShader(VS.Handle, VS.Decl)，
//然后VS.SetTransform（必须设置！！哪怕是NULL），最后用物体.DrawPrimitive即可(自动设置物体的VertexShader和Declaration)，
//用完要物体.RestoreVertexShader
//            2、多STREAM，用物体SetStreamSource设置流号，然后VS.SetTransform（必须设置！！哪怕是NULL），
//最后直接用VS.DrawPrimitive即可(自动设置VS.VertexShader)
//另外注意vsh文件中一定要加m4x4 oPos, v0, c92即三种变换（可适当调整代码，但一定是C92）
//还有80和84分别存放对法线和顶点的cameraspace变换矩阵，88和92分别存放法线和顶点的CLIPCAMERASPACE矩阵，直接m4x4乘即可

/*宏模板
D3DXMACRO Macro[] = {
{"POS", "v0"},
{"NORMAL", "v1"},
{"TEXCOORD", "v2"}
};*/


class VERTEXSHADER
{
public:
	char szErrorInfo[500];  //编译提示（错误信息）
	IDirect3DVertexShader9  *Handle;
	IDirect3DVertexDeclaration9 *Declaration;

	VERTEXSHADER();
	~VERTEXSHADER();
	//编译指定文件并创建VS，如果文件名为空表示创建固定管线中的VS，切记要检查是否成功
	HRESULT InitVertexShader(LPSTR Filename, D3DVERTEXELEMENT9 *decl, D3DXMACRO *pMacro = NULL);
	//设置常量寄存器，用法同device->SetConstant，如果是BOOL或int，那么RegNo表示StartRegNo，因为这两个寄存器只有一个分量，所以会占用多个寄存器
	HRESULT SetConstant(DWORD RegNo, const void* pConstantData, DWORD ConstantCount);
	HRESULT SetConstantB(DWORD RegNo, const BOOL* pConstantData, DWORD ConstantCount);
	HRESULT SetConstantI(DWORD RegNo, const int* pConstantData, DWORD ConstantCount);
	
	//设置世界变换矩阵(设为NULL表示无变换)，不用设转置，因为它会自动转置的，初始为1
	//用单Stream时必须设置，因为它还要用VIEW和PROJ变换，否则出来是没有图像的
	//只能用于指令VS，会自动将世界、观察和透视矩阵合成，存入C92-C95
	HRESULT SetTransform(D3DXMATRIX *Matrix);
	// 同上，只不过它是静态变量，不受任何VS约束，可以用于全局共享的常量寄存器设置，比如ShadowMap
	// 用于全局的SetTranform，而不用牵扯任何VERTEXSHADER对象（常量寄存器本身就是全局共享的嘛）
	static HRESULT SetGlobalTransform(D3DXMATRIX *Matrix);

	//根据VS渲染，不过必须要先指定Stream，Texture、Constant和Transform
	HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	// 这个函数必须指定VertexNum，否则会造成在A卡上出错（事实上本身就是错误的，只是N卡会强制执行而已，用下DX DEBUG就知道了）
	HRESULT DrawIndexPrimitive(UINT StartIndex, UINT PrimitiveCount, UINT iVertexNum);

	void Release();

private:
	ID3DXBuffer *ComplicationErrors, *CompiledShader;
	unsigned char ShaderAttrib;  //属性：0表示未创建，1表示正常的指令VS，2表示使用固定管线的VS
	D3DXMATRIX VSWorldMatrix;  //世界变换矩阵，设置后直接用它乘剪裁和投影矩阵得到最终变换矩阵，用于加速VS
};

class PIXELSHADER
{
public:
	char szErrorInfo[500];   //编译提示（错误信息）
	LPDIRECT3DPIXELSHADER9 Handle, OldHandle;

	PIXELSHADER();
	~PIXELSHADER();
	void Release();
	//编译指定文件并创建PS，文件名不能为空，切记要检查是否成功
	HRESULT InitPixelShader(LPSTR Filename, D3DXMACRO *pMacro = NULL);
	//设置常量寄存器，用法同device->SetConstant，如果是BOOL或int，那么RegNo表示StartRegNo，因为这两个寄存器只有一个分量，所以会占用多个寄存器
	HRESULT SetConstant(DWORD RegNo, CONST void* pConstantData, DWORD ConstantCount);
	HRESULT SetConstantB(DWORD RegNo, CONST BOOL* pConstantData, DWORD ConstantCount);
	HRESULT SetConstantI(DWORD RegNo, CONST int* pConstantData, DWORD ConstantCount);
	//设置创建的PS，同时会将原PS保存到OLDHANDLE
	HRESULT SetPixelShader();
	//恢复原来的PS(Set时得到)，记得在特效物体渲染完后恢复固定管线的PS
	HRESULT RestorePixelShader();
	
private:
	ID3DXBuffer *ComplicationErrors, *CompiledShader;
	unsigned char ShaderAttrib;  //属性：0表示未创建，1表示正常
};