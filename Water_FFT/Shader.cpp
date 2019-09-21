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
	if(ShaderAttrib) return E_FAIL;  //˵����SHADER�Ѵ�����������RELEASE�������ٴ���

	char *p = "Vertex Shader����ɹ���";

	memcpy(szErrorInfo, p, strlen(p));

	//����FP����VS����Ҫ��������
	if(S_OK!=d3ddevice->CreateVertexDeclaration(decl, &Declaration))
	{
		memset(szErrorInfo, 0, 500);
		memcpy(szErrorInfo, "������������", strlen("������������"));
		OutputDebugString("\nVertex Shader������������\n");
		return E_FAIL;
	}
	ShaderAttrib = 2;

	if(Filename)      //�����ɱ��VS���̶����ߵľ�����
	{
		if(FAILED(D3DXAssembleShaderFromFile(Filename, pMacro, NULL, D3DXSHADER_DEBUG, &CompiledShader, &ComplicationErrors)))
		{
			//����д��󣬾͵õ�������Ϣ
			if(ComplicationErrors)
			{
				p = (char *)ComplicationErrors->GetBufferPointer();
				memcpy(szErrorInfo, p, ComplicationErrors->GetBufferSize());
			}
			else
			{
				OutputDebugString("\nVertex Shader�����ļ���ȡʧ�ܣ�\n");
				memcpy(szErrorInfo, "Vertex Shader�����ļ���ȡʧ�ܣ�", strlen("Vertex Shader�����ļ���ȡʧ�ܣ�"));
			}

			OutputDebugString("\nVertex Shader������󣡴�����Ϣ");
			OutputDebugString(szErrorInfo);
			return E_FAIL;
		}

		if(FAILED(d3ddevice->CreateVertexShader((DWORD*)CompiledShader->GetBufferPointer(), &Handle)))
		{
			OutputDebugString("\nVertex Shader����ɹ����������������豸�Ƿ�֧�ֻ��豸�Ƿ񴴽��ɹ���\n");
			memcpy(szErrorInfo, "Vertex Shader����ɹ����������������豸�Ƿ�֧�ֻ��豸�Ƿ񴴽��ɹ���", strlen("Vertex Shader����ɹ����������������豸�Ƿ�֧�ֻ��豸�Ƿ񴴽��ɹ���"));
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
	
	//�Զ�����������ꡢ������������������70-72
	if(FAILED(d3ddevice->SetVertexShaderConstantF(70,(float *)&Eye,1)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShaderConstantF(71,(float *)&Temp1,1)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShaderConstantF(72,(float *)&Temp2,1)))
		return E_FAIL;
	
	//���ʹ��ָ��Զ�������World Space�������76--79����ʱ�Ĵ���
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&VSWorldMatrix);
	D3DXMatrixTranspose(&oPostemp, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(76,(float *)&oPostemp,4)))
		return E_FAIL;

	//���ʹ��ָ��Զ�������camera space�任�������80--84����ʱ�Ĵ���
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&VSWorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixInverse(&oPostemp, NULL, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(80,(float *)&oPostemp,4)))
		return E_FAIL;
	
	//���ʹ��ָ��Զ�������camera space�任�������84--87����ʱ�Ĵ���
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&VSWorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixTranspose(&oPostemp,&oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(84,(float *)&oPostemp,4)))
		return E_FAIL;

	//���ʹ��ָ��Զ�������World space�任�������88--91����ʱ�Ĵ���
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&VSWorldMatrix);
	D3DXMatrixInverse(&oPostemp, NULL, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(88,(float *)&oPostemp,4)))
		return E_FAIL;

	//���ʹ��ָ��Զ�������proj space�任�������92--95����ʱ�Ĵ���
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&VSWorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myproj);	
	D3DXMatrixTranspose(&oPostemp,&oPostemp);        //�м����е�����任����VS�����Ĵ����ж�Ҫ��ת��
	return d3ddevice->SetVertexShaderConstantF(92,(float *)&oPostemp,4);
}



// ȫ�־�̬����������Global��SetTranform��������ǣ���κ�VERTEXSHADER���󣨳����Ĵ����������ȫ�ֹ�����
HRESULT VERTEXSHADER::SetGlobalTransform(D3DXMATRIX *Matrix)
{
	D3DXMATRIX oPostemp, WorldMatrix;
	D3DXMatrixIdentity(&WorldMatrix);
	if(Matrix!=NULL) D3DXMatrixMultiply(&WorldMatrix,&WorldMatrix,Matrix);
	
	D3DXVECTOR4 Eye=D3DXVECTOR4(CameraChange.Eye.x,CameraChange.Eye.y,CameraChange.Eye.z,1);
	D3DXVECTOR4 Temp1=D3DXVECTOR4(-2,-1,-0.5,0);
	D3DXVECTOR4 Temp2=D3DXVECTOR4(0,0.5,1,2);
	
	//�Զ�����������ꡢ������������������70-72
	if(FAILED(d3ddevice->SetVertexShaderConstantF(70,(float *)&Eye,1)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShaderConstantF(71,(float *)&Temp1,1)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetVertexShaderConstantF(72,(float *)&Temp2,1)))
		return E_FAIL;
	
	//���ʹ��ָ��Զ�������World Space�������76--79����ʱ�Ĵ���
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&WorldMatrix);
	D3DXMatrixTranspose(&oPostemp, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(76,(float *)&oPostemp,4)))
		return E_FAIL;
	
	//���ʹ��ָ��Զ�������camera space�任�������80--84����ʱ�Ĵ���
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&WorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixInverse(&oPostemp, NULL, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(80,(float *)&oPostemp,4)))
		return E_FAIL;
	
	//���ʹ��ָ��Զ�������camera space�任�������84--87����ʱ�Ĵ���
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&WorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixTranspose(&oPostemp,&oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(84,(float *)&oPostemp,4)))
		return E_FAIL;
	
	//���ʹ��ָ��Զ�������World space�任�������88--91����ʱ�Ĵ���
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&WorldMatrix);
	D3DXMatrixInverse(&oPostemp, NULL, &oPostemp);
	if(FAILED(d3ddevice->SetVertexShaderConstantF(88,(float *)&oPostemp,4)))
		return E_FAIL;
	
	//���ʹ��ָ��Զ�������proj space�任�������92--95����ʱ�Ĵ���
	D3DXMatrixIdentity(&oPostemp);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&WorldMatrix);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myview);
	D3DXMatrixMultiply(&oPostemp,&oPostemp,&myproj);	
	D3DXMatrixTranspose(&oPostemp,&oPostemp);        //�м����е�����任����VS�����Ĵ����ж�Ҫ��ת��
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
	if(ShaderAttrib||Filename==NULL) return E_FAIL;  //˵����SHADER�Ѵ�����������RELEASE�������ٴ���
	
	DWORD i = d3dcaps.PixelShaderVersion & 0xffff;
	if(i==0) return E_FAIL;   // Ӳ����֧�ֵľͷ��أ���ʵ��û��Ҫ���������ã���Ϊ�ʼ�Ѿ����������

	char *p = "Pixel Shader����ɹ���";
	memcpy(szErrorInfo, p, strlen(p));

		if(FAILED(D3DXAssembleShaderFromFile(Filename, pMacro, NULL, D3DXSHADER_DEBUG, &CompiledShader, &ComplicationErrors)))
		{
			//����д��󣬾͵õ�������Ϣ
			if(ComplicationErrors)
			{
				p = (char *)ComplicationErrors->GetBufferPointer();
				memcpy(szErrorInfo, p, ComplicationErrors->GetBufferSize());
			}
			else
			{
				OutputDebugString("\nPixel Shader�����ļ���ȡʧ�ܣ�\n");
				memcpy(szErrorInfo, "Pixel Shader�����ļ���ȡʧ�ܣ�", strlen("Pixel Shader�����ļ���ȡʧ�ܣ�"));
			}

			OutputDebugString("\nPixel Shader������󣡴�����Ϣ");
			OutputDebugString(szErrorInfo);
			return E_FAIL;
		}

		if(FAILED(d3ddevice->CreatePixelShader((DWORD*)CompiledShader->GetBufferPointer(), &Handle)))
		{
			OutputDebugString("\nPixel Shader����ɹ����������������豸�Ƿ�֧�ֻ��豸�Ƿ񴴽��ɹ���\n");
			memcpy(szErrorInfo, "Pixel Shader����ɹ����������������豸�Ƿ�֧�ֻ��豸�Ƿ񴴽��ɹ���", strlen("Pixel Shader����ɹ����������������豸�Ƿ�֧�ֻ��豸�Ƿ񴴽��ɹ���"));
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