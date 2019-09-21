#include "Myd3d.h"
#include "Shader.h"


VERTEXSHADER::VERTEXSHADER()
{
	ComplicationErrors=NULL; CompiledShader=NULL;
	ShaderAttrib=0;
	memset(szErrorInfo, 0, 500);
}

VERTEXSHADER::~VERTEXSHADER()
{
	Release();
}


HRESULT VERTEXSHADER::InitVertexShader(LPSTR Filename, D3DVERTEXELEMENT9 *decl, D3DXMACRO *pMacro)
{
	if(ShaderAttrib) return E_FAIL;  //说明该SHADER已创建，必须先RELEASE，才能再创建

	char *p = "Vertex Shader编译成功！";

	memcpy(szErrorInfo, p, strlen(p));

	//无论FP还是VS，都要创建声明
	if(S_OK!=d3ddevice->CreateVertexDeclaration(decl, &Declaration))
	{
		memset(szErrorInfo, 0, 500);
		memcpy(szErrorInfo, "顶点声明错误！", strlen("顶点声明错误！"));
		OutputDebugString("\nVertex Shader顶点声明错误！\n");
		return E_FAIL;
	}
	ShaderAttrib = 2;

	if(Filename)      //创建可编程VS，固定管线的就跳过
	{
		if(FAILED(D3DXAssembleShaderFromFile(Filename, pMacro, NULL, D3DXSHADER_DEBUG, &CompiledShader, &ComplicationErrors)))
		{
			//如果有错误，就得到错误信息
			if(ComplicationErrors)
			{
				p = (char *)ComplicationErrors->GetBufferPointer();
				memcpy(szErrorInfo, p, ComplicationErrors->GetBufferSize());
			}
			else
			{
				OutputDebugString("\nVertex Shader程序文件读取失败！\n");
				memcpy(szErrorInfo, "Vertex Shader程序文件读取失败！", strlen("Vertex Shader程序文件读取失败！"));
			}

			OutputDebugString("\nVertex Shader编译错误！错误信息");
			OutputDebugString(szErrorInfo);
			return E_FAIL;
		}

		if(FAILED(d3ddevice->CreateVertexShader((DWORD*)CompiledShader->GetBufferPointer(), &Handle)))
		{
			OutputDebugString("\nVertex Shader编译成功但创建错误！请检查设备是否支持或设备是否创建成功！\n");
			memcpy(szErrorInfo, "Vertex Shader编译成功但创建错误！请检查设备是否支持或设备是否创建成功！", strlen("Vertex Shader编译成功但创建错误！请检查设备是否支持或设备是否创建成功！"));
			return E_FAIL;
		}

		ShaderAttrib=1;
	}

	return S_OK;
}


HRESULT VERTEXSHADER::SetConstant(DWORD RegNo, const void* pConstantData, DWORD ConstantCount)
{
	if(ShaderAttrib == 0) return E_FAIL;
	return d3ddevice->SetVertexShaderConstantF(RegNo, (float *)pConstantData, ConstantCount);
}

HRESULT VERTEXSHADER::SetConstantB(DWORD RegNo, const BOOL* pConstantData, DWORD ConstantCount)
{
	if(ShaderAttrib == 0) return E_FAIL;
	return d3ddevice->SetVertexShaderConstantB(RegNo, pConstantData, ConstantCount);
}

HRESULT VERTEXSHADER::SetConstantI(DWORD RegNo, const int* pConstantData, DWORD ConstantCount)
{
	if(ShaderAttrib == 0) return E_FAIL;
	return d3ddevice->SetVertexShaderConstantI(RegNo, pConstantData, ConstantCount);
}


HRESULT VERTEXSHADER::SetTransform(D3DXMATRIX *Matrix)
{
	D3DXMATRIX oPostemp;
	if(ShaderAttrib!=1) return E_FAIL;
	D3DXMatrixIdentity(&VSWorldMatrix);
	if(Matrix!=NULL) D3DXMatrixMultiply(&VSWorldMatrix,&VSWorldMatrix,Matrix);

	D3DXVECTOR4 Eye=D3DXVECTOR4(CameraChange.Eye.x,CameraChange.Eye.y,CameraChange.Eye.z,1);
	D3DXVECTOR4 Temp1=D3DXVECTOR4(-2,-1,-0.5,0);
	D3DXVECTOR4 Temp2=D3DXVECTOR4(0,0.5,1,2);
	
	//自动将摄像机坐标、负常数和正常数存入70-72
	if(FAILED(d3ddevice->SetVertexShaderConstantF(70,(float *)&Eye,1)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShaderConstantF(71,(float *)&Temp1,1)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShaderConstantF(72,(float *)&Temp2,1)))
		return E_FAIL;
	
	//如果使用指令，自动将顶点World Space矩阵存入76--79号临时寄存器
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&VSWorldMatrix);
	D3DXMatrixTranspose(&oPostemp, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(76,(float *)&oPostemp,4)))
		return E_FAIL;

	//如果使用指令，自动将法线camera space变换矩阵存入80--84号临时寄存器
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&VSWorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixInverse(&oPostemp, NULL, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(80,(float *)&oPostemp,4)))
		return E_FAIL;
	
	//如果使用指令，自动将坐标camera space变换矩阵存入84--87号临时寄存器
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&VSWorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixTranspose(&oPostemp,&oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(84,(float *)&oPostemp,4)))
		return E_FAIL;

	//如果使用指令，自动将法线World space变换矩阵存入88--91号临时寄存器
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&VSWorldMatrix);
	D3DXMatrixInverse(&oPostemp, NULL, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(88,(float *)&oPostemp,4)))
		return E_FAIL;

	//如果使用指令，自动将坐标proj space变换矩阵存入92--95号临时寄存器
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&VSWorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myproj);	
	D3DXMatrixTranspose(&oPostemp,&oPostemp);        //切记所有的世界变换矩阵到VS常量寄存器中都要先转置
	return d3ddevice->SetVertexShaderConstantF(92,(float *)&oPostemp,4);
}



// 全局静态变量，用于Global的SetTranform，而不用牵扯任何VERTEXSHADER对象（常量寄存器本身就是全局共享的嘛）
HRESULT VERTEXSHADER::SetGlobalTransform(D3DXMATRIX *Matrix)
{
	D3DXMATRIX oPostemp, WorldMatrix;
	D3DXMatrixIdentity(&WorldMatrix);
	if(Matrix!=NULL) D3DXMatrixMultiply(&WorldMatrix,&WorldMatrix,Matrix);
	
	D3DXVECTOR4 Eye=D3DXVECTOR4(CameraChange.Eye.x,CameraChange.Eye.y,CameraChange.Eye.z,1);
	D3DXVECTOR4 Temp1=D3DXVECTOR4(-2,-1,-0.5,0);
	D3DXVECTOR4 Temp2=D3DXVECTOR4(0,0.5,1,2);
	
	//自动将摄像机坐标、负常数和正常数存入70-72
	if(FAILED(d3ddevice->SetVertexShaderConstantF(70,(float *)&Eye,1)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShaderConstantF(71,(float *)&Temp1,1)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShaderConstantF(72,(float *)&Temp2,1)))
		return E_FAIL;
	
	//如果使用指令，自动将顶点World Space矩阵存入76--79号临时寄存器
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&WorldMatrix);
	D3DXMatrixTranspose(&oPostemp, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(76,(float *)&oPostemp,4)))
		return E_FAIL;
	
	//如果使用指令，自动将法线camera space变换矩阵存入80--84号临时寄存器
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&WorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixInverse(&oPostemp, NULL, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(80,(float *)&oPostemp,4)))
		return E_FAIL;
	
	//如果使用指令，自动将坐标camera space变换矩阵存入84--87号临时寄存器
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&WorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixTranspose(&oPostemp,&oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(84,(float *)&oPostemp,4)))
		return E_FAIL;
	
	//如果使用指令，自动将法线World space变换矩阵存入88--91号临时寄存器
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&WorldMatrix);
	D3DXMatrixInverse(&oPostemp, NULL, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(88,(float *)&oPostemp,4)))
		return E_FAIL;
	
	//如果使用指令，自动将坐标proj space变换矩阵存入92--95号临时寄存器
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&WorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myproj);	
	D3DXMatrixTranspose(&oPostemp,&oPostemp);        //切记所有的世界变换矩阵到VS常量寄存器中都要先转置
	return d3ddevice->SetVertexShaderConstantF(92,(float *)&oPostemp,4);
}





HRESULT VERTEXSHADER::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	if(ShaderAttrib == 0) return E_FAIL;
	HRESULT att=S_OK;
	if(FAILED(d3ddevice->SetVertexDeclaration(Declaration)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShader(Handle)))
		return E_FAIL;
	d3ddevice->BeginScene();
	if(FAILED(d3ddevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount)))
		att=E_FAIL;
	d3ddevice->EndScene();
	return att;
}

HRESULT VERTEXSHADER::DrawIndexPrimitive(UINT StartIndex, UINT PrimitiveCount, UINT iVertexNum)
{
	if(ShaderAttrib == 0) return E_FAIL;
	HRESULT att=S_OK;
	if(FAILED(d3ddevice->SetVertexDeclaration(Declaration)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShader(Handle)))
		return E_FAIL;
	d3ddevice->BeginScene();
	if(FAILED(d3ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, iVertexNum, StartIndex, PrimitiveCount)))
		att=E_FAIL;
	d3ddevice->EndScene();
	return att;
}


void VERTEXSHADER::Release()
{
	if(ShaderAttrib)
	{
		SAFE_RELEASE(Handle);
		SAFE_RELEASE(Declaration);
		Handle = NULL;
		ShaderAttrib=0;
	}
	SAFE_RELEASE(ComplicationErrors);
	SAFE_RELEASE(CompiledShader);

	memset(szErrorInfo, 0, 500);
}




















PIXELSHADER::PIXELSHADER()
{
	ComplicationErrors=NULL; CompiledShader=NULL;
	ShaderAttrib=0;
	OldHandle = NULL;
	memset(szErrorInfo, 0, 500);
}

PIXELSHADER::~PIXELSHADER()
{
	Release();
}


HRESULT PIXELSHADER::InitPixelShader(LPSTR Filename, D3DXMACRO *pMacro)
{
	if(ShaderAttrib||Filename==NULL) return E_FAIL;  //说明该SHADER已创建，必须先RELEASE，才能再创建
	
	DWORD i = d3dcaps.PixelShaderVersion & 0xffff;
	if(i==0) return E_FAIL;   // 硬件不支持的就返回，事实上没必要在这里设置，因为最开始已经做过检测了

	char *p = "Pixel Shader编译成功！";
	memcpy(szErrorInfo, p, strlen(p));

		if(FAILED(D3DXAssembleShaderFromFile(Filename, pMacro, NULL, D3DXSHADER_DEBUG, &CompiledShader, &ComplicationErrors)))
		{
			//如果有错误，就得到错误信息
			if(ComplicationErrors)
			{
				p = (char *)ComplicationErrors->GetBufferPointer();
				memcpy(szErrorInfo, p, ComplicationErrors->GetBufferSize());
			}
			else
			{
				OutputDebugString("\nPixel Shader程序文件读取失败！\n");
				memcpy(szErrorInfo, "Pixel Shader程序文件读取失败！", strlen("Pixel Shader程序文件读取失败！"));
			}

			OutputDebugString("\nPixel Shader编译错误！错误信息");
			OutputDebugString(szErrorInfo);
			return E_FAIL;
		}

		if(FAILED(d3ddevice->CreatePixelShader((DWORD*)CompiledShader->GetBufferPointer(), &Handle)))
		{
			OutputDebugString("\nPixel Shader编译成功但创建错误！请检查设备是否支持或设备是否创建成功！\n");
			memcpy(szErrorInfo, "Pixel Shader编译成功但创建错误！请检查设备是否支持或设备是否创建成功！", strlen("Pixel Shader编译成功但创建错误！请检查设备是否支持或设备是否创建成功！"));
			return E_FAIL;
		}

	ShaderAttrib=1;
	return S_OK;
}

HRESULT PIXELSHADER::SetConstant(DWORD RegNo, CONST void* pConstantData, DWORD ConstantCount)
{
	if(ShaderAttrib == 0) return E_FAIL;
	return d3ddevice->SetPixelShaderConstantF(RegNo, (float *)pConstantData, ConstantCount);
}

HRESULT PIXELSHADER::SetConstantB(DWORD RegNo, CONST BOOL* pConstantData, DWORD ConstantCount)
{
	if(ShaderAttrib == 0) return E_FAIL;
	return d3ddevice->SetPixelShaderConstantB(RegNo, pConstantData, ConstantCount);
}

HRESULT PIXELSHADER::SetConstantI(DWORD RegNo, CONST int* pConstantData, DWORD ConstantCount)
{
	if(ShaderAttrib == 0) return E_FAIL;
	return d3ddevice->SetPixelShaderConstantI(RegNo, pConstantData, ConstantCount);
}



HRESULT PIXELSHADER::SetPixelShader()
{
	if(ShaderAttrib == 0) return E_FAIL;
	SAFE_RELEASE(OldHandle);
	d3ddevice->GetPixelShader(&OldHandle);
	return d3ddevice->SetPixelShader(Handle);
}

HRESULT PIXELSHADER::RestorePixelShader()
{
	if(ShaderAttrib == 0) return E_FAIL;
	return d3ddevice->SetPixelShader(OldHandle);
	SAFE_RELEASE(OldHandle);
}

void PIXELSHADER::Release()
{
	if(ShaderAttrib)
	{
		SAFE_RELEASE(Handle);
		Handle=0;
		ShaderAttrib=0;
	}
	SAFE_RELEASE(OldHandle);
	SAFE_RELEASE(ComplicationErrors);
	SAFE_RELEASE(CompiledShader);

	memset(szErrorInfo, 0, 500);
}