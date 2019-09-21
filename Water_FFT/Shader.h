#pragma once

//�������÷�������������SetVertexShaderŶ
//			  1����STREAM����������SetVertexShader(VS.Handle, VS.Decl)��
//Ȼ��VS.SetTransform���������ã���������NULL�������������.DrawPrimitive����(�Զ����������VertexShader��Declaration)��
//����Ҫ����.RestoreVertexShader
//            2����STREAM��������SetStreamSource�������ţ�Ȼ��VS.SetTransform���������ã���������NULL����
//���ֱ����VS.DrawPrimitive����(�Զ�����VS.VertexShader)
//����ע��vsh�ļ���һ��Ҫ��m4x4 oPos, v0, c92�����ֱ任�����ʵ��������룬��һ����C92��
//����80��84�ֱ��ŶԷ��ߺͶ����cameraspace�任����88��92�ֱ��ŷ��ߺͶ����CLIPCAMERASPACE����ֱ��m4x4�˼���

/*��ģ��
D3DXMACRO Macro[] = {
{"POS", "v0"},
{"NORMAL", "v1"},
{"TEXCOORD", "v2"}
};*/


class VERTEXSHADER
{
public:
	char szErrorInfo[500];  //������ʾ��������Ϣ��
	IDirect3DVertexShader9  *Handle;
	IDirect3DVertexDeclaration9 *Declaration;

	VERTEXSHADER();
	~VERTEXSHADER();
	//����ָ���ļ�������VS������ļ���Ϊ�ձ�ʾ�����̶������е�VS���м�Ҫ����Ƿ�ɹ�
	HRESULT InitVertexShader(LPSTR Filename, D3DVERTEXELEMENT9 *decl, D3DXMACRO *pMacro = NULL);
	//���ó����Ĵ������÷�ͬdevice->SetConstant�������BOOL��int����ôRegNo��ʾStartRegNo����Ϊ�������Ĵ���ֻ��һ�����������Ի�ռ�ö���Ĵ���
	HRESULT SetConstant(DWORD RegNo, const void* pConstantData, DWORD ConstantCount);
	HRESULT SetConstantB(DWORD RegNo, const BOOL* pConstantData, DWORD ConstantCount);
	HRESULT SetConstantI(DWORD RegNo, const int* pConstantData, DWORD ConstantCount);
	
	//��������任����(��ΪNULL��ʾ�ޱ任)��������ת�ã���Ϊ�����Զ�ת�õģ���ʼΪ1
	//�õ�Streamʱ�������ã���Ϊ����Ҫ��VIEW��PROJ�任�����������û��ͼ���
	//ֻ������ָ��VS�����Զ������硢�۲��͸�Ӿ���ϳɣ�����C92-C95
	HRESULT SetTransform(D3DXMATRIX *Matrix);
	// ͬ�ϣ�ֻ�������Ǿ�̬�����������κ�VSԼ������������ȫ�ֹ���ĳ����Ĵ������ã�����ShadowMap
	// ����ȫ�ֵ�SetTranform��������ǣ���κ�VERTEXSHADER���󣨳����Ĵ����������ȫ�ֹ�����
	static HRESULT SetGlobalTransform(D3DXMATRIX *Matrix);

	//����VS��Ⱦ����������Ҫ��ָ��Stream��Texture��Constant��Transform
	HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	// �����������ָ��VertexNum������������A���ϳ�����ʵ�ϱ�����Ǵ���ģ�ֻ��N����ǿ��ִ�ж��ѣ�����DX DEBUG��֪���ˣ�
	HRESULT DrawIndexPrimitive(UINT StartIndex, UINT PrimitiveCount, UINT iVertexNum);

	void Release();

private:
	ID3DXBuffer *ComplicationErrors, *CompiledShader;
	unsigned char ShaderAttrib;  //���ԣ�0��ʾδ������1��ʾ������ָ��VS��2��ʾʹ�ù̶����ߵ�VS
	D3DXMATRIX VSWorldMatrix;  //����任�������ú�ֱ�������˼��ú�ͶӰ����õ����ձ任�������ڼ���VS
};

class PIXELSHADER
{
public:
	char szErrorInfo[500];   //������ʾ��������Ϣ��
	LPDIRECT3DPIXELSHADER9 Handle, OldHandle;

	PIXELSHADER();
	~PIXELSHADER();
	void Release();
	//����ָ���ļ�������PS���ļ�������Ϊ�գ��м�Ҫ����Ƿ�ɹ�
	HRESULT InitPixelShader(LPSTR Filename, D3DXMACRO *pMacro = NULL);
	//���ó����Ĵ������÷�ͬdevice->SetConstant�������BOOL��int����ôRegNo��ʾStartRegNo����Ϊ�������Ĵ���ֻ��һ�����������Ի�ռ�ö���Ĵ���
	HRESULT SetConstant(DWORD RegNo, CONST void* pConstantData, DWORD ConstantCount);
	HRESULT SetConstantB(DWORD RegNo, CONST BOOL* pConstantData, DWORD ConstantCount);
	HRESULT SetConstantI(DWORD RegNo, CONST int* pConstantData, DWORD ConstantCount);
	//���ô�����PS��ͬʱ�ὫԭPS���浽OLDHANDLE
	HRESULT SetPixelShader();
	//�ָ�ԭ����PS(Setʱ�õ�)���ǵ�����Ч������Ⱦ���ָ��̶����ߵ�PS
	HRESULT RestorePixelShader();
	
private:
	ID3DXBuffer *ComplicationErrors, *CompiledShader;
	unsigned char ShaderAttrib;  //���ԣ�0��ʾδ������1��ʾ����
};