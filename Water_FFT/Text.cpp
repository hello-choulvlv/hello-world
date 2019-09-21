#include "Myd3d.h"
#include "Text.h"


MYTEXT::MYTEXT()
{
	TextFormat=DT_NOCLIP;
	Create2DFontAttrib = 0;
	DFont = NULL;
	Mesh = NULL;
}

MYTEXT::~MYTEXT()
{
	Release();
}

void MYTEXT::Release()
{
	SAFE_RELEASE(DFont);
	SAFE_RELEASE(Mesh);
	Mesh = NULL;
	Create2DFontAttrib = 0;
	DFont = NULL;
}

HRESULT MYTEXT::InitFont2D(UINT FontSize, LPSTR FontName)
{
	if(Create2DFontAttrib == TRUE) return S_OK;
	HRESULT hr = D3DXCreateFont( d3ddevice, FontSize, FontSize/2, 0, 0, FALSE, ANSI_CHARSET, OUT_TT_ONLY_PRECIS, 1, 0, FontName, &DFont ); //���������崴���ӿ�
	if(SUCCEEDED(hr)) Create2DFontAttrib = TRUE;
	return hr;
}


void MYTEXT::Set2DStyle(UINT left, UINT right, UINT up, UINT down, DWORD Format)
{
	Rect.left=left; Rect.right=right; Rect.top=up; Rect.bottom=down;
	TextFormat=Format;
}


HRESULT MYTEXT::DrawText2D(LPSTR Content, UINT Length, UINT Left, UINT Up, unsigned char r, unsigned char g, unsigned char b)
{
	if(Create2DFontAttrib == FALSE) return E_FAIL;
	HRESULT att=S_OK;
	Rect.left=Left; Rect.top=Up;
	if(Length==0) Length=getcharnum(Content);
	d3ddevice->BeginScene();
	if(FAILED(DFont->DrawText(NULL, Content, Length, &Rect, //�ı�����ʾ����ʹ�ô�������
			            	  TextFormat, //��ʾ��ʽ�������
							  D3DCOLOR_XRGB(r,g,b))))
		att=E_FAIL;
	d3ddevice->EndScene();
	return att;
}


HRESULT MYTEXT::DrawText2D(LPSTR Content, UINT Length, UINT Left, UINT Up, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	if(Create2DFontAttrib == FALSE) return E_FAIL;
	HRESULT att=S_OK;
	Rect.left=Left; Rect.top=Up;
	if(Length==0) Length=getcharnum(Content);
	d3ddevice->BeginScene();
	if(FAILED(DFont->DrawText(NULL, Content, Length, &Rect, //�ı�����ʾ����ʹ�ô�������
			            	  TextFormat, //��ʾ��ʽ�������
							  D3DCOLOR_ARGB(a,r,g,b))))
		att=E_FAIL;
	d3ddevice->EndScene();
	return att;
}



//3D���岿��
HRESULT MYTEXT::InitFont3D(UINT FontSize, LPSTR FontName, LPSTR Content, float deviation, float extrusion)
{
	Deviation=deviation; Extrusion=extrusion;
	HDC memdc;
	memdc=CreateCompatibleDC( NULL );
	//�����ﶨ����������ԣ�����б�塢������»��ߣ����忴SDK
	HFONT hFont = CreateFont(FontSize, 0, 0, 0, 0, 0, 0, 0,	ANSI_CHARSET, 0, 0, 0, 0, FontName);
	HFONT OldFont = (HFONT)SelectObject(memdc, hFont);

	//������ά�ı���Mesh����
	HRESULT hr = D3DXCreateText(d3ddevice, memdc, Content,
								Deviation, //����������������Բ���̶ȣ�ȡֵԽС������ԽԲ��
								Extrusion, //�ı���Z�᷽���ϵĺ��
								&Mesh, NULL, NULL );

	SelectObject(memdc, OldFont);
	DeleteObject( hFont );
	DeleteDC( memdc );
	return hr;
}

void MYTEXT::SetMaterial(float DiffuseR, float DiffuseG, float DiffuseB, float SpecularR, float SpecularG, float SpecularB, float EmissiveR, float EmissiveG, float EmissiveB, float Power)
{
	Material.Diffuse.r = DiffuseR;
	Material.Diffuse.g = DiffuseG;
	Material.Diffuse.b = DiffuseB;
	Material.Ambient.r = DiffuseR;  //һ����ʷ���������������ɫ��ͬ�������ͬ�ĵȵ����꺯�������Լ�����
	Material.Ambient.g = DiffuseG;
	Material.Ambient.b = DiffuseB;
	Material.Specular.r= SpecularR;
	Material.Specular.g= SpecularG;
	Material.Specular.b= SpecularB;
	Material.Emissive.r= EmissiveR;
	Material.Emissive.g= EmissiveG;
	Material.Emissive.b= EmissiveB;
	Material.Power     = Power;

	return;
}

HRESULT MYTEXT::DrawText3D()
{
	d3ddevice->BeginScene();
	d3ddevice->SetMaterial(&Material);
	HRESULT hr = Mesh->DrawSubset(0);
	d3ddevice->EndScene();
	return hr;
}






/******************IMAGE********************/
MYIMAGE::MYIMAGE()
{
	Time = GetTickCount();
	VertexShader = D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX2;
	EachPointSize = sizeof(MYIMAGEVERTEXTYPE);
	for(int i=0;i<MYD3D_MODULETEXTURENUM;i++) Texture[i] = NULL;
	for(int i=0;i<4;i++) {MyImageVertex[i].z=0; MyImageVertex[i].rhw=1; MyImageVertex[i].diffuse=0xffffffff;}
	TextureNum = 0;
	AlphaEnable = 0;
	ymcolor = 0;
	alphabmpenable=0;
	VertexBuffer = NULL;
	CreateAttrib = 0;
}


void MYIMAGE::Release()
{
	//�ͷ�����
	for(UINT i=0;i<TextureNum;i++) SAFE_RELEASE(Texture[i]);
	//����ͷŶ��㻺����
	SAFE_RELEASE(VertexBuffer);
	VertexBuffer = NULL;
	TextureNum = 0;
	AlphaEnable = 0;
	ymcolor = 0;
	alphabmpenable=0;
	CreateAttrib = 0;	
}

MYIMAGE::~MYIMAGE()
{
	Release();
}

float MYIMAGE::GetTime()
{
	return (GetTickCount()-Time)/18.2f;
}


HRESULT MYIMAGE::Init()
{
	if(CreateAttrib) return E_FAIL;
	
	//�������㻺����
    if(FAILED(d3ddevice->CreateVertexBuffer(4*EachPointSize, D3DUSAGE_WRITEONLY, VertexShader, D3DPOOL_DEFAULT, &VertexBuffer, NULL)))
		return E_FAIL;
	CreateAttrib = 1;
	return S_OK;
}




HRESULT MYIMAGE::Loadbmp(LPSTR Filename)
{
	if(CreateAttrib == 0) return E_FAIL;
	if(TextureNum==MYD3D_MODULETEXTURENUM) return E_FAIL;
	if(FAILED(D3DXCreateTextureFromFile(d3ddevice, Filename, &(Texture[TextureNum]))))
		return E_FAIL;
	else TextureNum++;
	return S_OK;
}


HRESULT MYIMAGE::Setymcolor(unsigned char a)
{
	if(CreateAttrib == 0) return E_FAIL;
	ymcolor=a;
	return d3ddevice->SetRenderState(D3DRS_ALPHAREF, ymcolor);
}
	
void MYIMAGE::Setcolor(unsigned char r0, unsigned char g0, unsigned char b0, unsigned char a0, unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1, unsigned char r2, unsigned char g2, unsigned char b2, unsigned char a2, unsigned char r3, unsigned char g3, unsigned char b3, unsigned char a3)
{
	DWORD rr,gg,bb,aa;
	aa=a0; rr=r0; gg=g0; bb=b0;
	MyImageVertex[1].diffuse=(aa<<24)|(rr<<16)|(gg<<8)|bb;
	aa=a1; rr=r1; gg=g1; bb=b1;
	MyImageVertex[3].diffuse=(aa<<24)|(rr<<16)|(gg<<8)|bb;
	aa=a2; rr=r2; gg=g2; bb=b2;
	MyImageVertex[0].diffuse=(aa<<24)|(rr<<16)|(gg<<8)|bb;
	aa=a3; rr=r3; gg=g3; bb=b3;
	MyImageVertex[2].diffuse=(aa<<24)|(rr<<16)|(gg<<8)|bb;
}



HRESULT MYIMAGE::Setalphabmp(UINT picno, int picx, int picy, int width, int height)
{
	int ext0sign=0;
	D3DSURFACE_DESC desc;
	if(CreateAttrib!=1||picno>=TextureNum) return E_FAIL;
	Texture[picno]->GetLevelDesc(0, &desc);

	//�õ����ú�ı߽����ݣ���Ҫ�Ǽ���PIC�����߽磬��Ļ����ı߽�ϵͳ���Զ�����
	if(width==0&&height==0) ext0sign=1;
	if(picx<0||picy<0||picx>=(int)desc.Width||picy>=(int)desc.Height) return E_FAIL;  //�б����
	if(ext0sign) {width=desc.Width; height=desc.Height;}
	if(picx+width>=(int)desc.Width) width=desc.Width-picx;
	if(picy+height>=(int)desc.Height) height=desc.Height-picy;
	if(width<=0||height<=0) return E_FAIL;

	//ת��Ϊ��������
	MyImageVertex[1].atu=(float)picx/desc.Width; MyImageVertex[1].atv=(float)picy/desc.Height;
	MyImageVertex[0].atu=(float)picx/desc.Width; MyImageVertex[0].atv=(float)(picy+height)/desc.Height;
	MyImageVertex[3].atu=(float)(picx+width)/desc.Width; MyImageVertex[3].atv=(float)picy/desc.Height;
	MyImageVertex[2].atu=(float)(picx+width)/desc.Width; MyImageVertex[2].atv=(float)(picy+height)/desc.Height;
	
	d3ddevice->GetTexture(1, &AlphaTexture);
	d3ddevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &AlphaOP);
	d3ddevice->GetTextureStageState(1, D3DTSS_ALPHAOP, &AlphaOP_1);
	d3ddevice->GetTextureStageState(1, D3DTSS_ALPHAARG1, &AlphaArg1_1);
	//��������״̬����Ⱦ״̬
	d3ddevice->SetTexture(1, Texture[picno]);
	d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	alphabmpenable=1;
	return S_OK;
}


void MYIMAGE::Restorealphabmp()
{
	if(CreateAttrib == 0) return;
	alphabmpenable=0;
	d3ddevice->SetTexture(1, AlphaTexture);
	d3ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP, AlphaOP);
	d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAOP, AlphaOP_1);
	d3ddevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, AlphaArg1_1);
}


void MYIMAGE::Save()
{
	if(CreateAttrib == 0) return;
	d3ddevice->GetTexture(0, &BackupTexture);
	d3ddevice->GetTextureStageState(0, D3DTSS_COLOROP, &ColorOP);
	d3ddevice->GetTextureStageState(0, D3DTSS_COLORARG1, &ColorArg1);
	d3ddevice->GetTextureStageState(0, D3DTSS_COLORARG2, &ColorArg2);
	d3ddevice->GetTextureStageState(1, D3DTSS_COLOROP, &ColorOP_1);
	d3ddevice->GetTextureStageState(1, D3DTSS_COLORARG1, &ColorArg1_1);
	d3ddevice->GetTextureStageState(1, D3DTSS_COLORARG2, &ColorArg2_1);

	d3ddevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &AlphaBlend);
	d3ddevice->GetRenderState(D3DRS_SRCBLEND, &SrcBlend);
	d3ddevice->GetRenderState(D3DRS_DESTBLEND, &DestBlend);
	d3ddevice->GetRenderState(D3DRS_ALPHATESTENABLE, &AlphaTest);
	d3ddevice->GetRenderState(D3DRS_ALPHAFUNC, &TestFunc);
}

void MYIMAGE::Restore()
{
	if(CreateAttrib == 0) return;
	d3ddevice->SetTexture(0, BackupTexture);
	d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP, ColorOP);
	d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1, ColorArg1);
	d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG2, ColorArg2);
	d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP, ColorOP_1);
	d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG1, ColorArg1_1);
	d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG2, ColorArg2_1);

	d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, AlphaBlend);
	d3ddevice->SetRenderState(D3DRS_SRCBLEND, SrcBlend);
	d3ddevice->SetRenderState(D3DRS_DESTBLEND, DestBlend);
	d3ddevice->SetRenderState(D3DRS_ALPHATESTENABLE, AlphaTest);
	d3ddevice->SetRenderState(D3DRS_ALPHAFUNC, TestFunc);
	SAFE_RELEASE(BackupTexture);
}


HRESULT MYIMAGE::Putcolor(int startx, int starty, int endx, int endy, UINT ym)
{
	if(CreateAttrib == 0) return E_FAIL;
	HRESULT result = S_OK;
	if(startx>=(int)d3ddm.Width||starty>=(int)d3ddm.Height||endx<startx||endy<starty||endx<0||endy<0) return E_FAIL;  //�б����

	//ת��Ϊ��������
	MyImageVertex[1].x=(float)startx; MyImageVertex[1].y=(float)starty;
	MyImageVertex[0].x=(float)startx; MyImageVertex[0].y=(float)endy;
	MyImageVertex[3].x=(float)endx; MyImageVertex[3].y=(float)starty;
	MyImageVertex[2].x=(float)endx; MyImageVertex[2].y=(float)endy;
	
	//��䶥�㻺����
	VOID* VertexStream;
	if(VertexBuffer->Lock(0, 4*EachPointSize, &VertexStream, 0))
		return E_FAIL;
	memcpy(VertexStream, MyImageVertex, 4*EachPointSize);
	VertexBuffer->Unlock();

	//��ʼ��Ⱦ
	if(FAILED(d3ddevice->SetStreamSource(0, VertexBuffer, 0, EachPointSize)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetFVF(VertexShader)))
		return E_FAIL;

  
	//����TSS���ã�COLOROP ALPHAOP ALPHABLENDING ALPHATEST
	Save();

	if(alphabmpenable==0) d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	else
	{
		d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	}
	//��������״̬����Ⱦ״̬
	if(AlphaEnable) 
	{
		d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		d3ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		d3ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	else d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	if(ym) 
	{
		d3ddevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		d3ddevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
	}
	else d3ddevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	d3ddevice->BeginScene();
	if(FAILED(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2))) result=E_FAIL;
	d3ddevice->EndScene();

	Restore();
	return result;
}


HRESULT MYIMAGE::Putbmp(UINT picno, int startx, int starty, int endx, int endy, int picx, int picy, int width, int height, UINT ym)
{
	if(CreateAttrib == 0) return E_FAIL;
	HRESULT result = S_OK;
	int ext0sign=0;
	D3DSURFACE_DESC desc;
	if(CreateAttrib!=1||picno>=TextureNum) return E_FAIL;
	Texture[picno]->GetLevelDesc(0, &desc);

	//�õ����ú�ı߽����ݣ���Ҫ�Ǽ���PIC�����߽磬��Ļ����ı߽�ϵͳ���Զ�����
	if(width==0&&height==0) ext0sign=1;
	if(startx>=(int)d3ddm.Width||starty>=(int)d3ddm.Height||endx<startx||endy<starty||endx<0||endy<0||picx<0||picy<0||picx>=(int)desc.Width||picy>=(int)desc.Height) return E_FAIL;  //�б����
	if(ext0sign) {width=desc.Width; height=desc.Height;}
	if(picx+width>=(int)desc.Width) width=desc.Width-picx;
	if(picy+height>=(int)desc.Height) height=desc.Height-picy;
	if(width<=0||height<=0) return E_FAIL;

	//ת��Ϊ��������
	MyImageVertex[1].x=(float)startx; MyImageVertex[1].y=(float)starty;
	MyImageVertex[1].tu=(float)picx/desc.Width; MyImageVertex[1].tv=(float)picy/desc.Height;
	MyImageVertex[0].x=(float)startx; MyImageVertex[0].y=(float)endy;
	MyImageVertex[0].tu=(float)picx/desc.Width; MyImageVertex[0].tv=(float)(picy+height)/desc.Height;
	MyImageVertex[3].x=(float)endx; MyImageVertex[3].y=(float)starty;
	MyImageVertex[3].tu=(float)(picx+width)/desc.Width; MyImageVertex[3].tv=(float)picy/desc.Height;
	MyImageVertex[2].x=(float)endx; MyImageVertex[2].y=(float)endy;
	MyImageVertex[2].tu=(float)(picx+width)/desc.Width; MyImageVertex[2].tv=(float)(picy+height)/desc.Height;
	
	//��䶥�㻺����
	VOID* VertexStream;
	if(VertexBuffer->Lock(0, 4*EachPointSize, &VertexStream, 0))
		return E_FAIL;
	memcpy(VertexStream, MyImageVertex, 4*EachPointSize);
	VertexBuffer->Unlock();

	//��ʼ��Ⱦ
	if(FAILED(d3ddevice->SetStreamSource(0, VertexBuffer, 0, EachPointSize)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetFVF(VertexShader)))
		return E_FAIL;

	//����TSS���ã�COLOROP ALPHAOP ALPHABLENDING ALPHATEST
	Save();

	if(FAILED(d3ddevice->SetTexture(0, Texture[picno])))
	return E_FAIL;

	//��������״̬����Ⱦ״̬
	d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	if(AlphaEnable)
	{
		d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		d3ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		d3ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	else d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	if(ym) 
	{
		d3ddevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		d3ddevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
	}
	else d3ddevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	d3ddevice->BeginScene();
	if(FAILED(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2))) result=E_FAIL;
	d3ddevice->EndScene();

	Restore();
	return result;
}



HRESULT MYIMAGE::Putcolorbmp(UINT picno, int startx, int starty, int endx, int endy, int picx, int picy, int width, int height, UINT ym)
{
	if(CreateAttrib == 0) return E_FAIL;
	HRESULT result = S_OK;
	int ext0sign=0;
	D3DSURFACE_DESC desc;
	if(CreateAttrib!=1||picno>=TextureNum) return E_FAIL;
	Texture[picno]->GetLevelDesc(0, &desc);

	//�õ����ú�ı߽����ݣ���Ҫ�Ǽ���PIC�����߽磬��Ļ����ı߽�ϵͳ���Զ�����
	if(width==0&&height==0) ext0sign=1;
	if(startx>=(int)d3ddm.Width||starty>=(int)d3ddm.Height||endx<startx||endy<starty||endx<0||endy<0||picx<0||picy<0||picx>=(int)desc.Width||picy>=(int)desc.Height) return E_FAIL;  //�б����
	if(ext0sign) {width=desc.Width; height=desc.Height;}
	if(picx+width>=(int)desc.Width) width=desc.Width-picx;
	if(picy+height>=(int)desc.Height) height=desc.Height-picy;
	if(width<=0||height<=0) return E_FAIL;

	//ת��Ϊ��������
	MyImageVertex[1].x=(float)startx; MyImageVertex[1].y=(float)starty;
	MyImageVertex[1].tu=(float)picx/desc.Width; MyImageVertex[1].tv=(float)picy/desc.Height;
	MyImageVertex[0].x=(float)startx; MyImageVertex[0].y=(float)endy;
	MyImageVertex[0].tu=(float)picx/desc.Width; MyImageVertex[0].tv=(float)(picy+height)/desc.Height;
	MyImageVertex[3].x=(float)endx; MyImageVertex[3].y=(float)starty;
	MyImageVertex[3].tu=(float)(picx+width)/desc.Width; MyImageVertex[3].tv=(float)picy/desc.Height;
	MyImageVertex[2].x=(float)endx; MyImageVertex[2].y=(float)endy;
	MyImageVertex[2].tu=(float)(picx+width)/desc.Width; MyImageVertex[2].tv=(float)(picy+height)/desc.Height;
	
	//��䶥�㻺����
	VOID* VertexStream;
	if(VertexBuffer->Lock(0, 4*EachPointSize, &VertexStream, 0))
		return E_FAIL;
	memcpy(VertexStream, MyImageVertex, 4*EachPointSize);
	VertexBuffer->Unlock();

	//��ʼ��Ⱦ
	if(FAILED(d3ddevice->SetStreamSource(0, VertexBuffer, 0, EachPointSize)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetFVF(VertexShader)))
		return E_FAIL;

	//��������ָ���TSS���ã�COLOROP ALPHAOP ALPHABLENDING ALPHATEST
	Save();

	if(FAILED(d3ddevice->SetTexture(0, Texture[picno])))
		return E_FAIL;
	
	//��������״̬����Ⱦ״̬
	d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3ddevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	if(AlphaEnable) 
	{
		d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		d3ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		d3ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	else d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	if(ym) 
	{
		d3ddevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		d3ddevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
	}
	else d3ddevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	d3ddevice->BeginScene();
	if(FAILED(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2))) result=E_FAIL;
	d3ddevice->EndScene();

	Restore();
	return result;
}















/*******************************ϵͳ�ӿ�*********************************/
KPostProcess::KPostProcess()
{
	m_iCreateAttrib = 0;
	m_iMRTAttrib	= 0;
	m_pVertexBuffer = m_pBilinearVertexBuffer = NULL;
	FreeTex();
}

KPostProcess::~KPostProcess()
{
	Release();
}

void KPostProcess::Release()
{
	m_iCreateAttrib = 0;
	m_iMRTAttrib	= 0;

	FreeTex();
	
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pBilinearVertexBuffer);
	SAFE_RELEASE(m_pStateBlock);
	m_GlareData.Release();

	// �ͷ�Ĭ�ϵ�RT��Depth
	SAFE_RELEASE(m_pOldRT);
	SAFE_RELEASE(m_pOldDepth);

	m_VSDrawQuad.Release();
	m_VSMultiTextureBlend.Release();
	m_VSBilinearResize.Release();
	
	m_PSGaussianH.Release();
	m_PSGaussianV.Release();
	m_PSGaussian5x5.Release();

	m_PSDownScale4x4.Release();
	m_PSDownScale2x2.Release();
	m_PSBilinearResize.Release();
	m_PSCopyResize.Release();

	m_PSLumLogFirst.Release();
	m_PSLumLogFirstWithoutDS4x4.Release();
	m_PSLumLogLast.Release();
	m_PSCalcAdapt.Release();
	
	m_PSSobel3x3.Release();
	m_PSNormal2x2.Release();
	m_PSDepth2x2.Release();
	m_PSEdgeIncrWhite.Release();
	m_PSEdgeDecrWhite.Release();
	m_PSEdgeIncrBlack.Release();
	m_PSEdgeDecrBlack.Release();

	m_PSToneMapping.Release();
	m_PSBrightPass.Release();
	m_PSBrightPassLDR.Release();

	m_PSDOF.Release();
}

void KPostProcess::FreeTex()
{
	// ���ͷ����е�RT��Depth���Ա����´�������Ϊÿ֡��Ҫ���øú�����
	SAFE_RELEASE(m_pSourceSceneRT);
	SAFE_RELEASE(m_pScaleSceneRT);
	SAFE_RELEASE(m_p256x256SceneRT);
	SAFE_RELEASE(m_p256x256TempRT);
	
	SAFE_RELEASE(m_pDepthSceneRT);
	SAFE_RELEASE(m_pNormalSceneRT);
	SAFE_RELEASE(m_pEdgeTemp1RT);
	SAFE_RELEASE(m_pEdgeTemp2RT);
	SAFE_RELEASE(m_pDepthEdgeRT);
	SAFE_RELEASE(m_pTemp1RT);
	SAFE_RELEASE(m_pTemp2RT);
	SAFE_RELEASE(m_pLumAdaptPrevFrameRT);
	SAFE_RELEASE(m_pLumAdaptCurrentFrameRT);
	SAFE_RELEASE(m_pToneMapRT);
	
	for(int i = 0; i < MAX_GLARE_LINE_NUM; i++)
		SAFE_RELEASE(m_ppGlareRT[i]);

	SAFE_RELEASE(m_pGlareTemp1RT);
	SAFE_RELEASE(m_pGlareTemp2RT);

	SAFE_RELEASE(m_pDOFRT);

	for(int i=0; i<4; i++)
		SAFE_RELEASE(m_ppTexDownTo1[i]);
	
	// �ͷ���Ȼ���
	SAFE_RELEASE(m_pFullSceneDepth);
	
	// ���ü���ָ��
	m_pTexture = NULL;
	m_pRT = NULL;
	m_pDepth = NULL;

	// ���ñ��
	m_bMeasureLuminance = m_bCalcAdapt = m_bCalcEdge = m_bCalcToneMap = false;
	m_bHDR = m_bMRT = false;
	m_bGlare = false;
}












/*******************************����ӿ�*********************************/
HRESULT KPostProcess::Init()
{
	if(m_iCreateAttrib)
		return E_FAIL;

	ZeroMemory(&m_DescRT, sizeof(D3DSURFACE_DESC));
	ZeroMemory(&m_DescDepth, sizeof(D3DSURFACE_DESC));

	// ��ʼ����Ƭ�Ķ��㻺��
	if(FAILED(d3ddevice->CreateVertexBuffer(4*sizeof(MYIMAGEVERTEXTYPE), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL)))
		return E_FAIL;
	if(FAILED(d3ddevice->CreateVertexBuffer(4*sizeof(MYIMAGEVERTEXTYPE), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_DEFAULT, &m_pBilinearVertexBuffer, NULL)))
		return E_FAIL;
	
	MYIMAGEVERTEXTYPE MyImageVertex[4] = 
	{
		{-1,-1,0,	0,1},
		{-1,1,0,	0,0},
		{1,-1,0,	1,1},
		{1,1,0,	1,0}
	};
	
	//��䶥�㻺����
	VOID* pVertexStream;
	if(m_pVertexBuffer->Lock(0, 4*sizeof(MYIMAGEVERTEXTYPE), &pVertexStream, 0))
		return E_FAIL;
	memcpy(pVertexStream, MyImageVertex, 4*sizeof(MYIMAGEVERTEXTYPE));
	m_pVertexBuffer->Unlock();

	// ��ʼ��Shader
	D3DVERTEXELEMENT9 Dclr[] = {
		{0,0,D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0,12,D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	
	if(FAILED(m_VSDrawQuad.InitVertexShader("shader\\PostProcessing\\VS\\DrawFullSceneQuad.vsh", Dclr)))
		return E_FAIL;
	if(FAILED(m_VSMultiTextureBlend.InitVertexShader("shader\\PostProcessing\\VS\\MultiTextureBlend.vsh", Dclr)))
		return E_FAIL;
	if(FAILED(m_VSBilinearResize.InitVertexShader("shader\\PostProcessing\\VS\\Bilinear-Resize.vsh", Dclr)))
		return E_FAIL;
	
	if(FAILED(m_PSGaussianH.InitPixelShader("shader\\PostProcessing\\common\\gaussianH.psh")))
		return E_FAIL;
	if(FAILED(m_PSGaussianV.InitPixelShader("shader\\PostProcessing\\common\\gaussianV.psh")))
		return E_FAIL;
	if(FAILED(m_PSGaussian5x5.InitPixelShader("shader\\PostProcessing\\common\\gaussian5x5.psh")))
		return E_FAIL;
	
	if(FAILED(m_PSDownScale2x2.InitPixelShader("shader\\PostProcessing\\common\\DownScale2x2.psh")))
		return E_FAIL;
	if(FAILED(m_PSDownScale4x4.InitPixelShader("shader\\PostProcessing\\common\\DownScale4x4.psh")))
		return E_FAIL;
	if(FAILED(m_PSBilinearResize.InitPixelShader("shader\\PostProcessing\\common\\Bilinear-Resize.psh")))
		return E_FAIL;

	if(FAILED(m_PSCopyResize.InitPixelShader("shader\\PostProcessing\\common\\Copy-Resize.psh")))
		return E_FAIL;
	
	
	if(CheckPS2xSupport(16, FALSE))
	{
		if(FAILED(m_PSLumLogFirst.InitPixelShader("shader\\PostProcessing\\HDR\\LumLogFirst.psh")))
			return E_FAIL;	
	}
	else if(ConvertShaderVersion(D3DXGetPixelShaderProfile(d3ddevice)) >= ConvertShaderVersion("ps.2.0"))
	{
		if(FAILED(m_PSLumLogFirstWithoutDS4x4.InitPixelShader("shader\\PostProcessing\\HDR\\LumLogFirst Without DS4x4.psh")))
			return E_FAIL;
	}
	else
	{
		mymessage("�Կ���֧��Pixel Shader 2.0���޷���������Post Processing��");
		return E_FAIL;
	}
	
	if(FAILED(m_PSLumLogLast.InitPixelShader("shader\\PostProcessing\\HDR\\LumLogLast.psh")))
		return E_FAIL;
	if(FAILED(m_PSCalcAdapt.InitPixelShader("shader\\PostProcessing\\HDR\\CalcAdapt.psh")))
		return E_FAIL;

	if(FAILED(m_PSBrightPassLDR.InitPixelShader("shader\\PostProcessing\\HDR\\BrightPassLDR.psh")))
		return E_FAIL;
	if(FAILED(m_PSBrightPass.InitPixelShader("shader\\PostProcessing\\HDR\\BrightPass.psh")))
		return E_FAIL;

	if(FAILED(m_PSGlare.InitPixelShader("shader\\PostProcessing\\HDR\\Glare.psh")))
		return E_FAIL;
	if(FAILED(m_PSAddGlare.InitPixelShader("shader\\PostProcessing\\HDR\\AddGlare.psh")))
		return E_FAIL;

	if(FAILED(m_PSToneMapping.InitPixelShader("shader\\PostProcessing\\HDR\\ToneMapping.psh")))
		return E_FAIL;
	
	if(FAILED(m_PSSobel3x3.InitPixelShader("shader\\PostProcessing\\Edge\\Sobel3x3.psh")))
		return E_FAIL;
	if(FAILED(m_PSNormal2x2.InitPixelShader("shader\\PostProcessing\\Edge\\Normal2x2.psh")))
		return E_FAIL;
	if(FAILED(m_PSDepth2x2.InitPixelShader("shader\\PostProcessing\\Edge\\Depth2x2.psh")))
		return E_FAIL;
	if(FAILED(m_PSEdgeIncrWhite.InitPixelShader("shader\\PostProcessing\\Edge\\EdgeIncrWhite.psh")))
		return E_FAIL;
	if(FAILED(m_PSEdgeIncrBlack.InitPixelShader("shader\\PostProcessing\\Edge\\EdgeIncrBlack.psh")))
		return E_FAIL;
	if(FAILED(m_PSEdgeDecrWhite.InitPixelShader("shader\\PostProcessing\\Edge\\EdgeDecrWhite.psh")))
		return E_FAIL;
	if(FAILED(m_PSEdgeDecrBlack.InitPixelShader("shader\\PostProcessing\\Edge\\EdgeDecrBlack.psh")))
		return E_FAIL;

	// Depth Of Field�������֧���㹻��ps2.a���Ͳ��ܱ���shader
	if(CheckPS2xSupport(21, FALSE))
	{
		if(FAILED(m_PSDOF.InitPixelShader("shader\\PostProcessing\\DOF\\DOFCombine.psh")))
			return E_FAIL;
	}

	// ����
	m_iCreateAttrib = 1;
	return S_OK;
}

HRESULT KPostProcess::StartRender(bool bHDR /* = false */, bool bMRT /* = false */)
{
	if(!m_iCreateAttrib || !m_pVertexBuffer)
		return E_FAIL;

	// HDR��MRT�����ø�����������Ԥ�ȶ��壬���δ����FP16/32�꣬�ͳ�������������ˣ�FP16����
#ifndef USE_FP16
#ifndef USE_FP32
	if(bHDR || bMRT)
	{
		OutputDebugString("\n����Ҫʹ��HDR����MRT�������ȶ���ʹ��FP16��FP32����\n");
		return D3DERR_INVALIDCALL;
	}
#endif
#endif


	int i = 0;
	LPDIRECT3DSURFACE9 pSurf = NULL;
	D3DSURFACE_DESC Desc, DescDepth;	// ���ں�ԭ���ĸ�ʽ���Աȣ�����ֱ��ʻ����ظ�ʽ����һ�������ϣ��������ͷ��ؽ�
	
// �õ��ɵĺ�̨�������Ȼ��弰����
	Desc.Width = m_DescRT.Width;
	Desc.Height = m_DescRT.Height;
	Desc.Format = m_DescRT.Format;
	DescDepth.Width = m_DescDepth.Width;
	DescDepth.Height = m_DescDepth.Height;
	DescDepth.Format = m_DescDepth.Format;

	if(FAILED(d3ddevice->GetRenderTarget(0, &m_pOldRT)))
		return E_FAIL;
	if(FAILED(m_pOldRT->GetDesc(&m_DescRT)))
		return E_FAIL;

	if(FAILED(d3ddevice->GetDepthStencilSurface(&m_pOldDepth)))
		return E_FAIL;
	if(FAILED(m_pOldDepth->GetDesc(&m_DescDepth)))
		return E_FAIL;

	// ���ÿ�ζ����·����̫��ʱ��
	// ���ԭʼRT�ķֱ��ʻ����ظ�ʽ�ı��ˣ������·��䡣���ڶ�ε��øú�����Ƕ�׵���
	// ���HDR��MRT������Ҳ���ϴβ�ͬ�ˣ��������·��䣬����ͬһ��KPost�����Ⱥ����ز�ͬ��Ҫ�ĳ���������û�з����Ӧ������
	if(m_bHDR != bHDR || m_bMRT != bMRT || Desc.Width!=m_DescRT.Width || Desc.Height!=m_DescRT.Height || Desc.Format!=m_DescRT.Format)
	{
		FreeTex();

		// �������Ȼ��嶼Ҫ����FreeTex������
		m_bHDR = bHDR;
		m_bMRT = bMRT;
		
		// �����HDR�����ø�����������ʹ��FP16�͹��ˣ��������Ҫ��������
		// ��Ҫ���Ի��ˣ�m_DescRTֻ��Ϊ���жϺ�̨�����ʽ�Ƿ�ı䣬���о���������Ϊԭ��1/4��С�Ĵ������ݣ����������ظ�ʽ�������ڲ���HDR�����ʽ����ֱ�ӹ�ϵ
#ifdef USE_FP16
		D3DFORMAT Format = m_bHDR ? D3DFMT_A16B16G16R16F : D3DFMT_A8R8G8B8;
		// ע���������õ�ͨ��FP16������G16R16������R16����ΪGF6/7����֧��R16F
		D3DFORMAT SingleChannelTexFormat = m_bHDR ? D3DFMT_G16R16F : D3DFMT_A8;
		D3DFORMAT BiChannelTexFormat = D3DFMT_G16R16F;
#elif defined USE_FP32
		D3DFORMAT Format = m_bHDR ? D3DFMT_A32B32G32R32F : D3DFMT_A8R8G8B8;
		D3DFORMAT SingleChannelTexFormat = m_bHDR ? D3DFMT_R32F : D3DFMT_A8;
		D3DFORMAT BiChannelTexFormat = D3DFMT_G32R32F;
#else 
		D3DFORMAT Format = D3DFMT_A8R8G8B8;
		D3DFORMAT SingleChannelTexFormat = D3DFMT_A8;
		D3DFORMAT BiChannelTexFormat = D3DFMT_R8G8B8;
#endif
		// �����MRT��ǿ���ø�������Ϊ������RT��MRT��ʽƥ�䣩
		Format = m_bMRT ? D3DFMT_A16B16G16R16F : Format;


		// ��ʼ��ԭͼ��һ��ԭ��һ��1/4��������HDR
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pSourceSceneRT)))
			return E_FAIL;
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pScaleSceneRT)))
			return E_FAIL;

		// ��ʼ��Glare��ͼ
		for(i = 0; i < MAX_GLARE_LINE_NUM; i++)
		{
			if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_ppGlareRT[i])))
				return E_FAIL;
		}

		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pGlareTemp1RT)))
			return E_FAIL;
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pGlareTemp2RT)))
			return E_FAIL;

		// ��ʼ����ʱ��ͼ��1/4�󣬱ض�ΪLDR����HDR��صĶ���ר����ͼ����������ͳ�ơ���Ӧ��ToneMap�������������Ǵ��ӳ�䵽LDR���ͼ��
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTemp1RT)))
			return E_FAIL;
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTemp2RT)))
			return E_FAIL;
		
		// ��ʼ������ר����ͼ
		// DOF�����ض���LDR�ģ�����ǿ����D3DFMT_A8R8G8B8
		// ���ת��ͼ�����ø��㣬����Ҫָ��FP16��FP32��������������������ִ�����
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pDOFRT)))
			return E_FAIL;

		// ��ʼ��MRT������ͼ�����ͼΪ�˾�ȷ����ʹ�ø�������ת����ı�Եͼ������������
		if(m_bMRT)
		{
			if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pNormalSceneRT)))
				return E_FAIL;
			if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pEdgeTemp1RT)))
				return E_FAIL;
			if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pEdgeTemp2RT)))
				return E_FAIL;
			if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pDepthSceneRT)))
				return E_FAIL;
			if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pDepthEdgeRT)))
				return E_FAIL;

			m_iMRTAttrib = 1;
		}

		// ��ʼ��HDRר�õģ�������ͼ��ToneMap
		if(m_bHDR)
		{
			if(FAILED(D3DXCreateTexture(d3ddevice, 256, 256, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_p256x256SceneRT)))
				return E_FAIL;
			if(FAILED(D3DXCreateTexture(d3ddevice, 256, 256, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_p256x256TempRT)))
				return E_FAIL;

			// 64x64->1x1��ͳ��ȫ�������ã�ǿ���õ�ͨ������
			for(i=0; i<4; i++)
				if(FAILED(D3DXCreateTexture(d3ddevice, 64>>(i*2), 64>>(i*2), 1, D3DUSAGE_RENDERTARGET, SingleChannelTexFormat, D3DPOOL_DEFAULT, &m_ppTexDownTo1[i])))
					return E_FAIL;
			// ToneMap�ض���LDR�ģ�����ǿ����D3DFMT_A8R8G8B8
			if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pToneMapRT)))
				return E_FAIL;

			// ��ʼ������1x1����ͼ��������Ӧ�ã�ǿ���õ�ͨ������
			if(FAILED(D3DXCreateTexture(d3ddevice, 1, 1, 1, D3DUSAGE_RENDERTARGET, SingleChannelTexFormat, D3DPOOL_DEFAULT, &m_pLumAdaptCurrentFrameRT)))
				return E_FAIL;
			if(FAILED(D3DXCreateTexture(d3ddevice, 1, 1, 1, D3DUSAGE_RENDERTARGET, SingleChannelTexFormat, D3DPOOL_DEFAULT, &m_pLumAdaptPrevFrameRT)))
				return E_FAIL;

		}//end HDRר����ͼ
			
	}// end change for Desc


	// ������Ȼ��壬��ȻҪ�ñȽ���ɸѡ�����ǵ�RT�ķֱ��ʸı��ʱ��ͬʱҲ���ͷ�DEPTH���壬��������Ҫ���ָ���Ƿ���Ч�������Ч����ʹDEPTH��ʽδ�ı䣬ҲҪ���·���
	if(!m_pFullSceneDepth || DescDepth.Width!=m_DescDepth.Width || DescDepth.Height!=m_DescDepth.Height || DescDepth.Format!=m_DescDepth.Format)
	{
		// ��ʼ����Ȼ���	
		if(FAILED(d3ddevice->CreateDepthStencilSurface(m_DescDepth.Width, m_DescDepth.Height, m_DescDepth.Format, m_DescDepth.MultiSampleType, m_DescDepth.MultiSampleQuality, FALSE, &m_pFullSceneDepth, NULL)))
			return E_FAIL;
	}

	
	// ����SourceSceneRT��ΪRT����Ⱦ��󳡾��͵�������ȥ��
	V_RETURN(SetTexturedRenderTarget(0, m_pSourceSceneRT, m_pFullSceneDepth));

	// �����������Ⱦͼ�񣬾������һ�Σ���Ϊ��Ⱦͼ��һ������ȫ�������Ǻ����Image�����Ͳ�������ˣ�ȫ������ȫ���ǵ�
	if(FAILED(d3ddevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0)))
		return E_FAIL;
		
	m_iCreateAttrib = 2;

	return S_OK;
}



HRESULT KPostProcess::EndRender()
{
	if(m_iCreateAttrib!=2 || !m_pOldRT || !m_pOldDepth)
		return E_FAIL;
	
	// ������Ⱦ��ϣ��ָ�Ĭ��RT�����Ǽ�ס�����ͷţ����ǻ����ã�Ҫ�����EndProcess��ʱ����Ϊ���ƶ���
	if(FAILED(d3ddevice->SetRenderTarget(0, m_pOldRT)))
		return E_FAIL;
	if(m_bMRT)
	{
		if(FAILED(d3ddevice->SetRenderTarget(1, NULL)))
			return E_FAIL;	
		if(FAILED(d3ddevice->SetRenderTarget(2, NULL)))
			return E_FAIL;	
	}

	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pOldDepth)))
		return E_FAIL;


	//D3DXSaveTextureToFile("c:\\scene.hdr", D3DXIFF_HDR, m_pSourceSceneRT, NULL);
	
	// ���ڲ�ԭʼ���ݺ���ͼ
	// ������Scale���Ƶ���Ӧ��������
	// ��ʱ��ԭ�����Ѿ���SourceScene���ˣ���ԭ����DS4x4->SceneScale����һ������ʷ��ǰ���ķ�ʱ����Ϊԭ�����ǳ��Ĵ�DS4X4���ǲ��������ģ�ľ�а취��waiting��
	m_pTexture = m_pSourceSceneRT;
	m_pRT = m_pScaleSceneRT;
	m_pDepth = m_pFullSceneDepth;
	if(FAILED(DownScale4x4()))
		return E_FAIL;

	if(m_bHDR)
	{
		// ��ԭ����DS4x4���ֱ��Resize��256x256Scene����������ͳ�ƣ���������죬����ɫ����ʧҲ���٣���Ȼ�ڵͷֱ����±���640*480��1/4ֻ��160*120��Resize to 256x256���Ǻܺã��Ͳ������ŷ����ˣ�����������Զ����õ�
		m_pTexture = m_pScaleSceneRT;
		m_pRT = m_p256x256SceneRT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(Resize()))
			return E_FAIL;
	}//end bHDR
	
	m_iCreateAttrib = 3;
	
	return S_OK;
}




HRESULT KPostProcess::StartMRT(bool bNormal /* = true */, bool bDepth /* = true */)
{
	// ����MRT������ͼ�ڵ�һ�������ͼ�ڵڶ���
	if(m_iCreateAttrib!=2 || m_iMRTAttrib<5&&m_iMRTAttrib!=1 || !m_bMRT|| !m_pNormalSceneRT || !m_pDepthSceneRT)
		return E_FAIL;

	LPDIRECT3DSURFACE9 pSurf = NULL;

	if(bNormal)
	{
		m_pNormalSceneRT->GetSurfaceLevel(0, &pSurf);
		if(FAILED(d3ddevice->ColorFill(pSurf, NULL, D3DCOLOR_XRGB(255, 255, 255))))
		{
			SAFE_RELEASE(pSurf);
			return E_FAIL;
		}

		if(FAILED(d3ddevice->SetRenderTarget(1, pSurf)))
		{
			SAFE_RELEASE(pSurf);
			return E_FAIL;
		}
		SAFE_RELEASE(pSurf);
	}
	else
	{
		if(FAILED(d3ddevice->SetRenderTarget(1, NULL)))
			return E_FAIL;
	}
	if(bDepth)
	{
		m_pDepthSceneRT->GetSurfaceLevel(0, &pSurf);
		if(FAILED(d3ddevice->ColorFill(pSurf, NULL, D3DCOLOR_XRGB(255, 255, 255))))
		{
			SAFE_RELEASE(pSurf);
			return E_FAIL;
		}
		
		if(FAILED(d3ddevice->SetRenderTarget(2, pSurf)))
		{
			SAFE_RELEASE(pSurf);
			return E_FAIL;
		}
		SAFE_RELEASE(pSurf);
	}	
	else
	{
		if(FAILED(d3ddevice->SetRenderTarget(2, NULL)))
			return E_FAIL;
	}
		

	if(bNormal && !bDepth) m_iMRTAttrib = 2;
	if(!bNormal && bDepth) m_iMRTAttrib = 3;
	if(bNormal && bDepth) m_iMRTAttrib = 4;
	return S_OK;
}



HRESULT KPostProcess::EndMRT()
{
	if(m_iCreateAttrib!=2 || m_iMRTAttrib<2 || !m_bMRT)
		return E_FAIL;
	
	if(FAILED(d3ddevice->SetRenderTarget(1, NULL)))
		return E_FAIL;	
	if(FAILED(d3ddevice->SetRenderTarget(2, NULL)))
		return E_FAIL;	

	m_iMRTAttrib += 3;
	return S_OK;
}



HRESULT KPostProcess::StartProcess()
{
	if(m_iCreateAttrib!=3 || !m_pScaleSceneRT || !m_pTemp1RT || !m_pTemp2RT || !m_pFullSceneDepth)
		return E_FAIL;
	
	// ����StateBlock
	SAFE_RELEASE(m_pStateBlock);
	if(FAILED(d3ddevice->CreateStateBlock(D3DSBT_PIXELSTATE, &m_pStateBlock)))
		return E_FAIL;
	
	d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 1);
	d3ddevice->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, 1);
	
	// �����HDR���ǲ���ֱ��ʹ��ScaleScene�ģ�����Bloom����ToneMap������ֱ��ӳ�䵽LDR���Ū��TEMP�е�
	// ���������HDR������Ͳ��ý�ScaleSceneŪ��Temp1���ˣ����һ��
	if(!m_bHDR)
	{
		// ��Scale Copy->Temp1
		m_pTexture = m_pScaleSceneRT;
		m_pRT = m_pTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(Resize()))
			return E_FAIL;
	}
	
	// �ü���ָ��
	m_pTexture = m_pTemp1RT;
	m_pRT = m_pTemp2RT;
	m_pDepth = m_pFullSceneDepth;
	

	m_iCreateAttrib = 4;
	return S_OK;
}


HRESULT KPostProcess::ResetProcess()
{
	if(m_iCreateAttrib!=4 || !m_pScaleSceneRT || !m_pTemp1RT || !m_pTemp2RT || !m_pFullSceneDepth)
		return E_FAIL;
	
	// �����HDR���ǲ���ֱ��ʹ��ScaleScene�ģ�����Bloom����ToneMap������ֱ��ӳ�䵽LDR���Ū��TEMP�е�
	// ���������HDR������Ͳ��ý�ScaleSceneŪ��Temp1���ˣ����һ��
	if(!m_bHDR)
	{
		// ��Scale Copy->Temp1
		m_pTexture = m_pScaleSceneRT;
		m_pRT = m_pTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(Resize()))
			return E_FAIL;
	}
	
	// �ü���ָ��
	m_pTexture = m_pTemp1RT;
	m_pRT = m_pTemp2RT;
	m_pDepth = m_pFullSceneDepth;
	
	return S_OK;
}

HRESULT KPostProcess::GetCurrentImage(LPDIRECT3DTEXTURE9 *ppTex)
{
	if(m_iCreateAttrib != 4 || !m_pTexture)
		return E_FAIL;

	D3DSURFACE_DESC Desc;
	LPDIRECT3DSURFACE9 pSrcSurf = NULL, pDstSurf = NULL;
	
	if(FAILED(m_pTexture->GetLevelDesc(0, &Desc)))
		return E_FAIL;

	// ����ԭ��ģ��Ա�EndProcess������
	if(FAILED(d3ddevice->CreateTexture(m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, Desc.Format, D3DPOOL_DEFAULT, ppTex, NULL)))
		return D3DERR_OUTOFVIDEOMEMORY;

	// �����ԭ��ԭ�󣬾���������
	if(Desc.Width == m_DescRT.Width && Desc.Height == m_DescRT.Height)
	{
		if(FAILED(m_pTexture->GetSurfaceLevel(0, &pSrcSurf)))
			return E_FAIL;
		if(FAILED((*ppTex)->GetSurfaceLevel(0, &pDstSurf)))
			return E_FAIL;
		// �����ư��ˣ���ѡPOINT����
		if(FAILED(d3ddevice->StretchRect(pSrcSurf, NULL, pDstSurf, NULL, D3DTEXF_POINT)))
			return E_FAIL;
		SAFE_RELEASE(pSrcSurf);
		SAFE_RELEASE(pDstSurf);
	}
	// �����С��ͬ������Copy/Bilinear Resize
	else
	{
		m_pRT = *ppTex;
		Resize();
	}

	return S_OK;
}

HRESULT KPostProcess::ForceEndProcess()
{
	if( m_iCreateAttrib!= 4 || !m_pStateBlock || !m_pOldDepth || !m_pOldRT)
		return E_FAIL;

	// �ָ�StateBlock
	m_pStateBlock->Apply();
	
	// �ָ�ԭRT��֮���Էŵ�StateBlock������Ϊ�˸ı�RT������
	if(FAILED(d3ddevice->SetRenderTarget(0, m_pOldRT)))
		return E_FAIL;	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pOldDepth)))
		return E_FAIL;

	SAFE_RELEASE(m_pOldRT);
	SAFE_RELEASE(m_pOldDepth);
	
	m_bMeasureLuminance = m_bCalcAdapt = m_bCalcEdge = m_bCalcToneMap = false;
	m_iCreateAttrib = 3;
	return S_OK;
}



HRESULT KPostProcess::EndProcess(PIXELSHADER* pPS /* = NULL */, UINT iTexNum /* = 0 */, LPDIRECT3DTEXTURE9 *ppTexture /* = NULL */)
{
	// ��ȷ�Լ��
	if( m_iCreateAttrib != 4 || !m_pTexture || !m_pSourceSceneRT || !m_pOldRT || !m_pOldDepth )
		return E_FAIL;
	if( iTexNum > 7 )
		return D3DERR_INVALIDCALL;

	UINT i = 0;
	for(i=0; i<iTexNum; i++)
		if(!ppTexture[i])
			return E_FAIL;

	LPDIRECT3DSURFACE9 pSurf;
	D3DSURFACE_DESC Desc;

	// ���δָ��PixelShader���ͰѴ���õ�ͼ���Ƶ�ԭRT��ȥ
	if(!pPS)
	{
		// ��������HDR ToneMapping����ֱ�Ӹ���ToneMap����С��ͬ����Copy����
		if(m_bHDR && m_bCalcToneMap)
		{
			m_pTexture = m_pToneMapRT;
			if(FAILED(CopyResize(true)))
				return E_FAIL;
		}
		// ������ô���õ�ͼ�񣬴���õ�ͼ�����m_pTexture��
		else
		{
			m_pTexture->GetLevelDesc(0, &Desc);
			if(Desc.Format == D3DFMT_A16B16G16R16F || Desc.Format == D3DFMT_G16R16F || Desc.Format == D3DFMT_R16F || Desc.Format == D3DFMT_A32B32G32R32F || Desc.Format == D3DFMT_G32R32F || Desc.Format == D3DFMT_R32F)
			{
				if(FAILED(BilinearResize(true)))
					return E_FAIL;
			}
			else
			{
				m_pTexture->GetSurfaceLevel(0, &pSurf);
				d3ddevice->StretchRect(pSurf, NULL, m_pOldRT, NULL, D3DTEXF_LINEAR);
				SAFE_RELEASE(pSurf);
			}
		}
	}
	
	// �Զ���shader����ϵ�ǰ�������Զ��������RT
	else
	{
		// �������յ�RT���ѳ�����Ϊ��������ȥ������Ⱦ������Ⱦ������RT��
		LPDIRECT3DSURFACE9 pSurf = NULL;
		VERTEXSHADER *pVS = &m_VSMultiTextureBlend;
		
		if(FAILED(d3ddevice->SetRenderTarget(0, m_pOldRT)))
			return E_FAIL;
		SAFE_RELEASE(pSurf);
		
		if(FAILED(d3ddevice->SetDepthStencilSurface(m_pOldDepth)))
			return E_FAIL;
		
		
		// Final Blending, Now Start...
		if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
			return E_FAIL;
		
		if(FAILED(pVS->SetTransform(NULL)))
			return E_FAIL;
		if(FAILED(pPS->SetPixelShader()))
			return E_FAIL;
		
		// ��������ToneMap���Ͱ�ToneMap��Ϊԭ����������ͽ�SourceScene��Ϊԭ������������ʵ��һ���ģ�����Ϊ0������
		LPDIRECT3DTEXTURE9 pTexture;
		if(m_bHDR && m_bCalcToneMap && m_pToneMapRT)
			pTexture = m_pToneMapRT;
		else
			pTexture = m_pSourceSceneRT;

		if(FAILED(d3ddevice->SetTexture(0, pTexture)))
			return E_FAIL;
		
		// ������������ƫ�Ƶ�VS��PS��c100��c30
		pTexture->GetLevelDesc(0, &Desc);
		D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
		pVS->SetConstant(100, &TexCoordBias, 1);
		
		// ����Զ���������Ч�������õ���0�㣬�����ò�����VS c101�� PS c31
		if(ppTexture && iTexNum)
			for(i=0; i<iTexNum; i++)
			{
				d3ddevice->SetTexture(i+1, ppTexture[i]);
				d3ddevice->SetSamplerState(i+1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
				d3ddevice->SetSamplerState(i+1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
				d3ddevice->SetSamplerState(i+1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
				d3ddevice->SetSamplerState(i+1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
				
				ppTexture[i]->GetLevelDesc(0, &Desc);
				TexCoordBias = D3DXVECTOR4(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
				pVS->SetConstant(101+i, &TexCoordBias, 1);
			}
		
		// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
		d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		// ������һ�㲻��SourceScene����ToneMap������ԭ�󣬾���POINT����
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		
		// ����
		if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
			return E_FAIL;
		
		// ��Ⱦ��ϣ��ָ�
		pPS->RestorePixelShader();
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	}// end shader blend to RT


	// �ָ�StateBlock
	m_pStateBlock->Apply();
	
	
	// �ָ�ԭRT��֮���Էŵ�StateBlock������Ϊ�˲���StateBlock���ǵ�����ı�RT�����ò���
	if(FAILED(d3ddevice->SetRenderTarget(0, m_pOldRT)))
		return E_FAIL;	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pOldDepth)))
		return E_FAIL;
	
	SAFE_RELEASE(m_pOldRT);
	SAFE_RELEASE(m_pOldDepth);


	m_bMeasureLuminance = m_bCalcAdapt = m_bCalcEdge = m_bCalcToneMap = false;
	m_iCreateAttrib = 3;
	return S_OK;
}
















/*******************************�߼��ⲿ���ܽӿ�***************************/

HRESULT KPostProcess::HDR2LDR(float fKeyValue)
{
	if(!m_bHDR)
		OutputDebugString("\n����Ҫʹ��HDR2LDR��������StartRender��ָ��HDRΪtrue����\n");
	
	if(FAILED(MeasureLuminance()))
		return E_FAIL;

	if(FAILED(CalcAdapt()))
		return E_FAIL;

	return ToneMapping(fKeyValue);
}


HRESULT KPostProcess::Bloom(float fKeyValue, float fClampValue, float fOffset)
{
	if(m_bHDR)
	{
		if(FAILED(MeasureLuminance()))
			return E_FAIL;
		if(FAILED(CalcAdapt()))
			return E_FAIL;
	}
		
	// ���δָ��������Ϊ0��������HDR����LDR��ǰ���������ĵ���������BrightPass
	if(m_bHDR)
	{
		if(fKeyValue == 0)
			fKeyValue = 0.18f;
		if(fClampValue == 0)
			fClampValue = 0.5f;
		if(fOffset == 0)
			fOffset = 20.0f;
	}
	else
	{
		if(fKeyValue == 0)
			fKeyValue = 1.5f;
		if(fClampValue == 0)
			fClampValue = 0.0f;
	}

	if(FAILED(BrightPass(fKeyValue, fClampValue, fOffset)))
		return E_FAIL;

	// Glare����һ��ֻ���Glare���ӵ�BrightPass�ϣ�������Ӱ�������ģ���ȻSetGlare��Ż���Ч
	if(m_bGlare)
	{
		// ��BrightPass����ͼ��(m_pTexture��������ģ�����ģ������β������ݴ浽Glareר����ͼ������
		V_RETURN(Glare());
		// ������Glareר����ͼ�ϳɵ���ǰ��BrightPass��ͼ��m_pTexture����
		V_RETURN(AddGlare());
	}
	// ����Ϊ�����Ч�ʣ�������Glare���������ͬʱ��������Bloom�����ֻ��õ������Glare��������AddGlare����ȾĿ����Ϊm_pRT����ȥ�������Gaussian����



	if(FAILED(Gaussian1D(true)))
		return E_FAIL;
	if(FAILED(Gaussian1D(false)))
		return E_FAIL;
	return S_OK;
}




HRESULT KPostProcess::EdgeGlow(POSTPROCESSING_SOURCETYPE ProcRTType /* = POSTRTTYPE_NORMAL */, float fThreshold /* = 0.0f */, bool bEdgeColorWhite /* = true */)
{
	if(FAILED(EdgeDetection(ProcRTType, fThreshold, bEdgeColorWhite)))
		return E_FAIL;
	if(FAILED(EdgeEnhanced(true, bEdgeColorWhite)))
		return E_FAIL;

	// �����DEPTH���Ͷ���ǿ���Σ���ΪDepthһ�㶼�Ƚ�ϸ
	if(ProcRTType == POSTRTTYPE_DEPTH)
	{
		if(FAILED(EdgeEnhanced(true, bEdgeColorWhite)))
			return E_FAIL;
		if(FAILED(EdgeEnhanced(true, bEdgeColorWhite)))
			return E_FAIL;
		if(FAILED(EdgeEnhanced(true, bEdgeColorWhite)))
			return E_FAIL;
	}
	
	// ��Եͼ����EdgeTemp�У�ҪDS4x4��Temp��Ȼ��ģ������Ч���źã���Եͼ���Ƿ��ˣ�һ���С�˲��У�һ������˲��У�
	m_pDepth = m_pFullSceneDepth;
	m_pRT = m_pTemp1RT;
	if(FAILED(DownScale4x4()))
		return E_FAIL;
	
	m_pTexture = m_pTemp1RT;
	m_pRT = m_pTemp2RT;
	m_pDepth = m_pFullSceneDepth;
	if(FAILED(Gaussian1D(true)))
		return E_FAIL;
	if(FAILED(Gaussian1D(false)))
		return E_FAIL;

	return S_OK;
}



HRESULT KPostProcess::DOF(float fFocalPlane, float fNearPlane /* = 0.0f */, float fFarPlane /* = 0.0f */, float fBlurPower /* = 3.0f */)
{
	// Ĭ�����������򵥵Ĳ�������NearPlaneΪ0��FarPlane = 2 * FocalPlane
	if(fFarPlane == 0.0f)
		fFarPlane = fFocalPlane * 2.0f;
	if(fNearPlane > fFocalPlane || fFocalPlane > fFarPlane || fNearPlane > fFarPlane || fNearPlane < 0 || fFocalPlane < 0 || fFarPlane < 0)
		return D3DERR_INVALIDCALL;

	if(CheckPS2xSupport(21, FALSE))
	{
		mymessage("�Կ���֧��Pixel Shader 2.0a���޷���ȾDepth Of Field��")
		return E_FAIL;
	}


	// ��SourceScene��alphaͨ���ָ�Camera Depth��Blur Coef�����ǹ̶��Ĵ�SourceScene��DOFDepthInfoRT����������m_pTex/RT
	//if(FAILED(DOFConvertDepthInfoShader(fFocalPlane, fNearPlane, fFarPlane)))
	//	return E_FAIL;


	// ��HDR�У���ΪDOF��������ʾ��ͼ�����Բ�����ScaleScene��Ҫ��ToneMap
	if(m_bHDR)
	{
		// û����HDR��ǿ�ƽ���Tone Mapping
		if(!m_bCalcToneMap)
		{
			if(FAILED(HDR2LDR()))
				return E_FAIL;
		}

		// ��m_pToneMap DownScale4x4��TempRT�ϣ��Ա�Gaussian
		m_pTexture = m_pToneMapRT;
		m_pRT = m_pTemp1RT;
		if(FAILED(DownScale4x4()))
			return E_FAIL;
	}
	else
	{
		// ��ScaleScene Copy��TempRT�ϣ��Ա�Gaussian
		m_pTexture = m_pScaleSceneRT;
		m_pRT = m_pTemp1RT;
		if(FAILED(CopyResize()))
			return E_FAIL;
	}

	// �������HDR�ͷ�HDR�������Ѿ���1/4Сͼת����TempRT���ˣ��������ǹ��ò���
	// �õ�Gaussian��ģ��Сͼ��m_pTexture��
	m_pTexture = m_pTemp1RT;
	m_pRT = m_pTemp2RT;
	if(FAILED(Gaussian1D(true)))
		return E_FAIL;
	if(FAILED(Gaussian1D(false)))
		return E_FAIL;

	// �����HDR������ToneMap����֮��SourceScene����ԭͼ��ģ�����Сͼ�������DOF����
	m_pRT = m_pDOFRT;
	if(FAILED(DOFShader(m_bHDR ? m_pToneMapRT : m_pSourceSceneRT, m_pTexture, fFocalPlane, fNearPlane, fFarPlane, fBlurPower)))
		return E_FAIL;

	m_pTexture = m_pDOFRT;

	// Test
	//D3DXSaveTextureToFile("c:\\dof.tga", D3DXIFF_TGA, m_pDOFRT, NULL);
	return S_OK;
}






/*******************************����ڲ����ܽӿ�*********************************/

HRESULT KPostProcess::Resize()
{
	if(m_iCreateAttrib<2 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;

	// �ȵõ�m_pTexture�����ԣ������ж���Bilinear����Copy
	D3DSURFACE_DESC DescTex, DescRT;
	if(FAILED(m_pTexture->GetLevelDesc(0, &DescTex)))
		return E_FAIL;
	if(FAILED(m_pRT->GetLevelDesc(0, &DescRT)))
		return E_FAIL;

	// �����С��ȫ��ͬ���Ǿ�ֱ��Copy����ǣ��������ˣ�
	if(DescRT.Width == DescTex.Width && DescRT.Height == DescTex.Height)
		return CopyResize();
	// ��С��ͬ��Ҫ������������ˣ����Ϊ������������Shader����
	else if(DescTex.Format == D3DFMT_A16B16G16R16F || DescTex.Format == D3DFMT_G16R16F || DescTex.Format == D3DFMT_R16F || DescTex.Format == D3DFMT_A32B32G32R32F || DescTex.Format == D3DFMT_G32R32F || DescTex.Format == D3DFMT_R32F)
		return BilinearResize();
	// ��������Ļ�����Ӳ������
	else
		return CopyResize();
}




HRESULT KPostProcess::EdgeDetection(POSTPROCESSING_SOURCETYPE ProcRTType /* = POSTRTTYPE_NORMAL */, float fThreshold /* = 0.0f */, bool bEdgeColorWhite /* = true */)
{
	// ��Ϊ�����ڷ�MRT�£�Sobel����MRT�������ڣ���������ָ�벻����Ч�Լ�⣬��DetectionShaderȥ������
	if(m_iCreateAttrib!=4)
		return E_FAIL;
	if(m_bCalcEdge)
		return S_OK;

	// Ŀ�ľ���ת�浽EdgeTemp1

	// ��Sobel���ˣ�ֱ�Ӵ�Scale->Temp1����Ϊ���ǻ������ȵģ������HDR����Ҫclamp��1
	if(ProcRTType == POSTRTTYPE_SCENE)
	{
		m_pTexture = m_pScaleSceneRT;
		m_pRT = m_pEdgeTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(EdgeDetectionShader(ProcRTType, fThreshold, bEdgeColorWhite)))
			return E_FAIL;
	}

	// �÷���ͼ������Ե��⣬��ת��Ϊԭ���ԵͼNormalEdge����DS4x4��Temp
	else if(ProcRTType == POSTRTTYPE_NORMAL)
	{
		if(!m_bMRT || m_iMRTAttrib!=5 && m_iMRTAttrib!=7)
			return E_FAIL;

		// Ĭ����cos30��
		if(fThreshold == 0.0f)
			fThreshold = 0.866f;
		m_pTexture = m_pNormalSceneRT;
		m_pRT = m_pEdgeTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(EdgeDetectionShader(ProcRTType, fThreshold, bEdgeColorWhite)))
			return E_FAIL;
/*
		// �ȼ�⣬��DS4x4
		m_pTexture = m_pNormalEdgeRT;
		m_pRT = m_pTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(DownScale4x4()))
			return E_FAIL;
*/
	}

	// �����ͼ��������DS4x4��DepthEdge(1/4 FP)���ټ��
	else if(ProcRTType == POSTRTTYPE_DEPTH)
	{
		if(!m_bMRT || m_iMRTAttrib!=6 && m_iMRTAttrib!=7)
			return E_FAIL;

		// Ĭ�ϵ���ȱ仯����0.3
		if(fThreshold == 0.0f)
			fThreshold = 0.3f;
		
		m_pTexture = m_pDepthSceneRT;
/*		m_pRT = m_pDepthEdgeRT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(DownScale4x4()))
			return E_FAIL;

		// ��DS4x4���ٽ�����ȼ��
		m_pTexture = m_pDepthEdgeRT;
*/		m_pRT = m_pEdgeTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(EdgeDetectionShader(ProcRTType, fThreshold, bEdgeColorWhite)))
			return E_FAIL;
	}
	else
		return D3DERR_INVALIDCALL;

	// ǿ������ָ��
	m_pTexture = m_pEdgeTemp1RT;
	m_pRT = m_pEdgeTemp2RT;
	m_bCalcEdge = true;

	return S_OK;
}




HRESULT KPostProcess::MeasureLuminance()
{
	if(!m_bHDR)
		return D3DERR_INVALIDCALL;
	
	if(m_iCreateAttrib!=4)
		return E_FAIL;
	
	if(m_bMeasureLuminance)
		return S_OK;
	
	
	// �����Կ�pixel shader�汾�ж�ʹ���ĸ�����
	char p[10] = "";
	// ��֧�֣�����
	if(!D3DXGetPixelShaderProfile(d3ddevice))
		return E_FAIL;
	
	strcpy(p, D3DXGetPixelShaderProfile(d3ddevice));
	
	// ����ֻ֧��ps2.0���Կ�������ps2.0ר�ú�����ps2a ps2b������ᣬִ�б���������
	if(p[3] == '2' && p[5] == '0')
	{
		if(FAILED(MeasureLuminanceShaderPS20()))
			return E_FAIL;
	}
	else
	{
		if(FAILED(MeasureLuminanceShader()))
			return E_FAIL;
	}


/*��һ�����ڲ����������Զ���Ӧʱ��Tone Mapping����HDR2LDR�йر�CalcAdapt��ͬʱ��ҲҪ����δ����
	LPDIRECT3DSURFACE9 pSurfDst = NULL, pSurfSrc = NULL;

	// ��m_ppTexDownTo1[3]���Ƶ�CurAdaptRT���ڲ�����������Ӧ�������Ҳ����ӳ�䵽LDR��
	if(FAILED(m_ppTexDownTo1[3]->GetSurfaceLevel(0, &pSurfSrc)))
		return E_FAIL;
	if(FAILED(m_pLumAdaptCurrentFrameRT->GetSurfaceLevel(0, &pSurfDst)))
		return E_FAIL;
	if(FAILED(d3ddevice->StretchRect(pSurfSrc, NULL, pSurfDst, NULL, D3DTEXF_POINT)))
		return E_FAIL;
	SAFE_RELEASE(pSurfSrc);
	SAFE_RELEASE(pSurfDst);	
*/

	// �����ڲ�����������ʹ��m_pTex/RTָ�룬���Բ��ý���
	m_bMeasureLuminance = true;
	return S_OK;
}






/*******************************�����ڲ����ܽӿ�*********************************/
HRESULT KPostProcess::Glare()
{
	if(m_iCreateAttrib != 4 || !m_pGlareTemp1RT || !m_pGlareTemp2RT || !m_pTexture || !m_pDepth)
		return E_FAIL;

	// ����RT��ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSMultiTextureBlend;
	PIXELSHADER *pPS = &m_PSGlare;
	LPDIRECT3DTEXTURE9 pRT = NULL, pTex = NULL;
	D3DSURFACE_DESC Desc;


	// ���ù��ò���
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;

	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;

	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;


	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);


	// �����β���򣬷ֱ���Ⱦ��GlareRT����Ĳ�ͬԪ����
	for(UINT iTrailNo = 0; iTrailNo < m_GlareData.m_iLineNum; iTrailNo++)
	{
		// ͨ����β���ȼ����ģ�������Ͳ������࣬���в�������Ϊ�̶�ֵ8�������Ͳ���Ҫ��Shader������̬ѭ����
		UINT iSampleNum = 8;
			// �������������������ģ������������಻���Թ��󣬷�������ʧ��
		UINT iBlurNum = 1;
		UINT iSampleInterval = 8;	// ֱ�Ӹ����ֵ�������β���ȴ�������������ģ������
		if(m_GlareData.m_pLineData[iTrailNo].iLength > iSampleInterval)
			iBlurNum = m_GlareData.m_pLineData[iTrailNo].iLength / iSampleInterval;
		
		// ������ת���õ�ģ������
		D3DXVECTOR2 VecTrialDir;
		D3DXVec2TransformCoord(&VecTrialDir, &m_GlareData.m_pLineData[iTrailNo].VecDir, &m_GlareData.m_MatRotate);
		// ע���ǰ�������ģ��������ע�������Dir��2D���ϵ�xy����ϵ		
		D3DXVec2Normalize(&VecTrialDir, &-VecTrialDir);


		// ���㵥����β���ֶ����Ⱦ���м���Temp1/2���л������һ����Ⱦ��GlareRT��
		for(UINT iBlurNo = 0; iBlurNo < iBlurNum; iBlurNo++)
		{
			if(iBlurNo == 0)
			{
				pTex = m_pTexture;	// ÿ����β�ĵ�һ����Ⱦ����Դ�����Ѿ���Ⱦ�õ�BrightPassͼ
				pRT = m_pGlareTemp1RT;
			}
			else
			{
				// �ֻ�RT����Դ��ͼ
				if(pRT == m_pGlareTemp1RT)
				{
					pTex = m_pGlareTemp1RT;
					pRT = m_pGlareTemp2RT;
				}
				else if(pRT == m_pGlareTemp2RT)
				{
					pTex = m_pGlareTemp2RT;
					pRT = m_pGlareTemp1RT;
				}
			}

			// ���һ����Ⱦ��GlareRT��
			if(iBlurNo == iBlurNum - 1)
				pRT = m_ppGlareRT[iTrailNo];


			// ����RT
			pRT->GetSurfaceLevel(0, &pSurf);
			if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
				return E_FAIL;
			SAFE_RELEASE(pSurf);

			// ������Դ��ͼ
			if(FAILED(d3ddevice->SetTexture(0, pTex)))
				return E_FAIL;

			// ���ó����Ĵ���������ģ������Ĳ�������ƫ�ơ�1/2tapƫ��
			pTex->GetLevelDesc(0, &Desc);
			D3DXVECTOR4 TexCoordBias = D3DXVECTOR4(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
			pVS->SetConstant(100, &TexCoordBias, 1);

			for(UINT i = 0; i < iSampleNum; i++)
			{
				D3DXVECTOR4 PtOffset;
				PtOffset.x = 1.0f / (float)Desc.Width * iSampleInterval * i * VecTrialDir.x;
				PtOffset.y = 1.0f / (float)Desc.Height * iSampleInterval * i * VecTrialDir.y;
				// ����˥���ȵõ����ϵ��
				//PtOffset.w = 1.0f - (m_GlareData.m_pLineData[iTrailNo].fAttenuation - 1.0f) * (float)i / (float)iSampleNum;							//��һ��Ԫ��Ϊ1�����һ��Ԫ��ΪfAttenuation���м��Ԫ�ز�ֵ
				PtOffset.w = m_GlareData.m_pLineData[iTrailNo].fAttenuation;
				pPS->SetConstant(10 + i, PtOffset, 1);
			}

			// ������β��ɫ
			D3DXVECTOR4 VecColor;
			D3DCOLOR SrcColor = m_GlareData.m_pLineData[iTrailNo].Color;
			VecColor.w = 0.0f;		// �����в���Ӱ��Alphaֵ
			VecColor.x = ((SrcColor >> 16) & 0xff) / 255.0f;	// r
			VecColor.y = ((SrcColor >> 8) & 0xff) / 255.0f;	// g
			VecColor.z = (SrcColor & 0xff) / 255.0f;			// b
			pPS->SetConstant(30, &VecColor, 1);


			// ����
			if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
				return E_FAIL;
		}
	}



	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	return S_OK;
}





HRESULT KPostProcess::AddGlare()
{
	if(m_iCreateAttrib!=4 || !m_pTexture || !m_pDepth)
		return E_FAIL;

	// ����RT��ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSMultiTextureBlend;
	PIXELSHADER *pPS = &m_PSAddGlare;
	LPDIRECT3DTEXTURE9 pRT = m_pTexture;	// ���ӵ���ǰ����õ�BrightPassͼ��
	D3DSURFACE_DESC Desc;


	// ���ù��ò���
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;

	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;

	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;


	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	// ��β����
	UINT iTrailNum = m_GlareData.m_iLineNum;

	// ������Դ��ͼ������
	for(UINT i = 0; i < iTrailNum; i++)
	{
		d3ddevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		d3ddevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		if(FAILED(d3ddevice->SetTexture(i, m_ppGlareRT[i])))
			return E_FAIL;
	}

		
	for(int i = 0; i < 8; i++)
	{
		// ������������ƫ��
		m_ppGlareRT[i]->GetLevelDesc(0, &Desc);
		D3DXVECTOR4 TexCoordBias = D3DXVECTOR4(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
		pVS->SetConstant(100+i, &TexCoordBias, 1);

		// ����Ȩ�أ����������ڵ���β�㣨Ϊ0��������ʱ�Ͳ������Ӱ����
		float fCoef = i < iTrailNum ? 1.0f : 0.0f;
		pPS->SetConstant(i, &D3DXVECTOR4(fCoef, fCoef, fCoef, fCoef), 1);
	}


	// ����RT
	pRT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);


	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;

	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	return S_OK;
}













HRESULT KPostProcess::DOFShader(LPDIRECT3DTEXTURE9 pTexSource, LPDIRECT3DTEXTURE9 pTexBlurred, float fFocalPlane, float fNearPlane, float fFarPlane, float fBlurPower)
{
	if(m_iCreateAttrib!=4 || !pTexBlurred || !pTexSource || !m_pRT || !m_pDepth)
		return E_FAIL;

	// ����RT��ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSMultiTextureBlend;
	PIXELSHADER *pPS = &m_PSDOF;
		
	m_pRT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);
	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;
	
	
	// Post Processing, Now Start...
	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;
	
	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;
	
	// �������Image��ͼ����ͼ��С��0��1��2��ֱ�ΪDepth��ԭͼ��ģ��ͼ
	if(FAILED(d3ddevice->SetTexture(0, m_pSourceSceneRT)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(1, pTexSource)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(2, pTexBlurred)))
		return E_FAIL;
	

	// ����Circle Of Confusionϵ���������Ĵ���c0��c11
	D3DXVECTOR4 fCOFCoef[] = 
	{
		D3DXVECTOR4(-0.326212f, -0.40581f, 0, 0),
		D3DXVECTOR4(-0.840144f, -0.07358f, 0, 0),
		D3DXVECTOR4(-0.695914f, 0.457137f, 0, 0),
		D3DXVECTOR4(-0.203345f, 0.620716f, 0, 0),
		D3DXVECTOR4(0.96234f, -0.194983f, 0, 0),
		D3DXVECTOR4(0.473434f, -0.480026f, 0, 0),
		D3DXVECTOR4(0.519456f, 0.767022f, 0, 0),
		D3DXVECTOR4(0.185461f, -0.893124f, 0, 0),
		D3DXVECTOR4(0.507431f, 0.064425f, 0, 0),
		D3DXVECTOR4(0.89642f, 0.412458f, 0, 0),
		D3DXVECTOR4(-0.32194f, -0.932615f, 0, 0),
		D3DXVECTOR4(-0.791559f, -0.59771f, 0, 0)
	};

	// ��ǿģ��Ч��
	for(int i=0; i<12; i++)
		fCOFCoef[i] *= fBlurPower;
	pPS->SetConstant(0, fCOFCoef, 12);

	// ������������ƫ�Ƶ������Ĵ���
	D3DSURFACE_DESC Desc;

	m_pSourceSceneRT->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias = D3DXVECTOR4(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(99, &TexCoordBias, 1);
	pPS->SetConstant(29, &TexCoordBias, 1);

	pTexSource->GetLevelDesc(0, &Desc);
	TexCoordBias = D3DXVECTOR4(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	pTexBlurred->GetLevelDesc(0, &Desc);
	TexCoordBias = D3DXVECTOR4(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(101, &TexCoordBias, 1);
	pPS->SetConstant(31, &TexCoordBias, 1);

	// ����3��Plane
	pPS->SetConstant(12, &D3DXVECTOR4(fFocalPlane, fNearPlane, fFarPlane, 1), 1);
	
	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	
	return S_OK;
}



HRESULT KPostProcess::CalcAdaptShader(float fFrameTime, float fIdleTime)
{
	// ������ˣ�ֻ��CalcAdapt����ã���ⶼ�������������

	// ����RT��ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = &m_PSCalcAdapt;
	
	m_pLumAdaptCurrentFrameRT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);
	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;
	
	
	// Post Processing, Now Start...
	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;
	
	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;
	
	// �������Image��ͼ����ͼ��С��0��1��ֱ�ΪPrev��Global-Lum
	if(FAILED(d3ddevice->SetTexture(0, m_pLumAdaptPrevFrameRT)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(1, m_ppTexDownTo1[3])))
		return E_FAIL;

	// ����1x1�ģ�ֱ����ps�ж��峣���������꼴�ɣ�����������������ƫ�Ƶ������Ĵ�����
	pPS->SetConstant(31, &D3DXVECTOR4(fFrameTime, fFrameTime, fFrameTime, fFrameTime), 1);
	// PS��c30����Ϊ��ǰIdleʱ�䣨�룩�������ڸվ�ֹ��N���ڲ�������Ӧ���㣨N��shader�ж��壩����������ƶ���վ�ֹ��Ϊ0������ô��Ϊ�������ƶ���ʱ�򲻽�����Ӧ����
	if(fIdleTime == 0)
		fIdleTime = 32768;
	pPS->SetConstant(30, &D3DXVECTOR4(fIdleTime, fIdleTime, fIdleTime, fIdleTime), 1);


	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
	
	// �ڲ�ʹ�ã����Ҳ�ǣ��m_pTex/RT
	return S_OK;
}





HRESULT KPostProcess::EdgeDetectionShader(POSTPROCESSING_SOURCETYPE ProcRTType, float fThreshold, bool bEdgeColorWhite)
{
	if(m_bCalcEdge)
		return S_OK;
	if(m_iCreateAttrib!=4 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;

	// ��������ѡ��Shader
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = NULL;

	if(ProcRTType == POSTRTTYPE_SCENE)
		pPS = &m_PSSobel3x3;
	else if(ProcRTType == POSTRTTYPE_NORMAL)
		pPS = &m_PSNormal2x2;
	else if(ProcRTType == POSTRTTYPE_DEPTH)
		pPS = &m_PSDepth2x2;
	else
		return D3DERR_INVALIDCALL;


	// ����RT
	LPDIRECT3DSURFACE9 pSurf = NULL;
	m_pRT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);

	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;


	// Post Processing, Now Start...
	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;

	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;

	// �������Image��ͼ����ͼ��С
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;

	// ������������ƫ�Ƶ�VS��PS��c100��c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
		// fThreshold
	pPS->SetConstant(29, &D3DXVECTOR4(fThreshold, fThreshold, fThreshold, fThreshold), 1);
	if(bEdgeColorWhite)
		pPS->SetConstant(31, &D3DXVECTOR4(1, 1, 1, 1), 1);
	else
		pPS->SetConstant(31, &D3DXVECTOR4(-1, -1, -1, -1), 1);

	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0xffffffff);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;

	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);


	// ���Ǳ�Ե��⣬��Ҫ����ָ��
	return S_OK;
}




HRESULT KPostProcess::EdgeEnhanced(bool bIncr /* = true */, bool bEdgeColorWhite /* = true */)
{
	if(m_iCreateAttrib!=4 || !m_bCalcEdge || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;
	
	// ��������ѡ��Shader
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = NULL;
	if(bIncr)
		pPS = bEdgeColorWhite ? &m_PSEdgeIncrWhite : &m_PSEdgeIncrBlack;
	else
		pPS = bEdgeColorWhite ? &m_PSEdgeDecrWhite : &m_PSEdgeDecrBlack;
		
	
	// ����RT����ʱ��Եͼ�Ѿ�����pTex��ֱ����pTex����pRT����
	LPDIRECT3DSURFACE9 pSurf = NULL;
	m_pRT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);
	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;
	
	
	// Post Processing, Now Start...
	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;
	
	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;
	
	// �������Image��ͼ����ͼ��С
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;
	
	// ������������ƫ�Ƶ�VS��PS��c100��c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
 	
	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	// BorderColor���ǷǱ�Եɫ
	DWORD dwBorderColor = bEdgeColorWhite ? 0 : 0xffffffff;
	d3ddevice->SetSamplerState(0, D3DSAMP_BORDERCOLOR, dwBorderColor);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	
	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	

	// ��ͬsize�ģ�����ָ��
	LPDIRECT3DTEXTURE9 pTemp;
	pTemp = m_pTexture;
	m_pTexture = m_pRT;
	m_pRT = pTemp;

	return S_OK;
}





HRESULT KPostProcess::Gaussian1D(bool bHorizon, float fRadius)
{
	if(m_iCreateAttrib!=4 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;

	// ����RT
	LPDIRECT3DSURFACE9 pSurf = NULL;
	m_pRT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);

	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;


	// Post Processing, Now Start...
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = bHorizon ? &m_PSGaussianH : &m_PSGaussianV;

	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;

	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;

	// �������Image��ͼ����ͼ��С
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;

	// ������������ƫ�Ƶ�VS��PS��c100��c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);

	SetGaussian1D(fRadius);


	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;

	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);


	// ����ͬSize�ģ�����m_pTex/RT��ָ�룬depth���ý�������Ϊͬsize�
	LPDIRECT3DTEXTURE9 pTemp;
	pTemp = m_pTexture;
	m_pTexture = m_pRT;
	m_pRT = pTemp;
	return S_OK;
}






HRESULT KPostProcess::Gaussian5x5(float fRadius)
{
	if(m_iCreateAttrib!=4 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;

	// ����RT
	LPDIRECT3DSURFACE9 pSurf = NULL;
	m_pRT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);

	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;

	// Post Processing, Now Start...
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = &m_PSGaussian5x5;

	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;

	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;

	// �������Image��ͼ����ͼ��С
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;

	// ������������ƫ�Ƶ�VS��PS��c100��c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);

	SetGaussian5x5(fRadius);


	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;

	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	// ����ͬSize�ģ�����m_pTex/RT��ָ�룬depth���ý�������Ϊͬsize�
	LPDIRECT3DTEXTURE9 pTemp;
	pTemp = m_pTexture;
	m_pTexture = m_pRT;
	m_pRT = pTemp;
	return S_OK;
}
















/*******************************�߼��ڲ����ܽӿ�*********************************/

HRESULT KPostProcess::MeasureLuminanceShader()
{
	if(!m_bHDR)
		return D3DERR_INVALIDCALL;

	if(m_iCreateAttrib!=4 || !m_p256x256SceneRT || !m_pFullSceneDepth || !m_ppTexDownTo1[0] || !m_ppTexDownTo1[1] || !m_ppTexDownTo1[2] || !m_ppTexDownTo1[3])
		return E_FAIL;
	
	if(m_bMeasureLuminance)
		return S_OK;


	// �õ�ԭRT����ʱ������Ⱦ��ָ��������Ͳ�����Ӱ���DS��Ⱦ�õ�RT
	LPDIRECT3DSURFACE9 pOldRT;
	if(FAILED(d3ddevice->GetRenderTarget(0, &pOldRT)))
		return E_FAIL;	
	
	// ����ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = &m_PSLumLogFirst;

	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pFullSceneDepth)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);

	for(int i=0; i<4; i++)
	{
		// ÿ��ѭ����������RT��Depth����һ���Ǵ�256x256-->LogSum DownTo1[0]
		m_ppTexDownTo1[i]->GetSurfaceLevel(0, &pSurf);
		if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
			return E_FAIL;
		SAFE_RELEASE(pSurf);
		
		// �������Image��ͼ����ͼ��С
		D3DSURFACE_DESC DescTex;
		if(i==0)
		{
			// ����ǵ�һ����Ⱦ����ô��ԴӦ����256x256���������Դ����DownTo1����һ��
			if(FAILED(d3ddevice->SetTexture(0, m_p256x256SceneRT)))
				return E_FAIL;
			pPS = &m_PSLumLogFirst;
		}
		else
		{
			if(FAILED(d3ddevice->SetTexture(0, m_ppTexDownTo1[i-1])))
				return E_FAIL;
			if(i==3)
				pPS = &m_PSLumLogLast;
			else
				pPS = &m_PSDownScale4x4;
		}

		// Rendering
		if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
			return E_FAIL;
		
		if(FAILED(pVS->SetTransform(NULL)))
			return E_FAIL;
		if(FAILED(pPS->SetPixelShader()))
			return E_FAIL;
		
		// ������������ƫ�Ƶ�VS��PS��c100��c30
		m_ppTexDownTo1[i]->GetLevelDesc(0, &DescTex);
		D3DXVECTOR4 TexCoordBias = D3DXVECTOR4(1/(float)DescTex.Width, 1/(float)DescTex.Height, 0.5f/(float)DescTex.Width, 0.5f/(float)DescTex.Height);
		pVS->SetConstant(100, &TexCoordBias, 1);
		pPS->SetConstant(30, &TexCoordBias, 1);
		
		// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
		d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		
		// ����
		if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
			return E_FAIL;
		
		// ��Ⱦ��ϣ��ָ�
		pPS->RestorePixelShader();
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	}


	// �ָ�RT
	if(FAILED(d3ddevice->SetRenderTarget(0, pOldRT)))
		return E_FAIL;
	SAFE_RELEASE(pOldRT);

	return S_OK;
}




HRESULT KPostProcess::MeasureLuminanceShaderPS20()
{
	if(!m_bHDR)
		return D3DERR_INVALIDCALL;
	
	if(m_iCreateAttrib!=4 || !m_p256x256SceneRT || !m_p256x256TempRT || !m_pFullSceneDepth || !m_ppTexDownTo1[0] || !m_ppTexDownTo1[1] || !m_ppTexDownTo1[2] || !m_ppTexDownTo1[3])
		return E_FAIL;
	
	if(m_bMeasureLuminance)
		return S_OK;
	
	// �õ�ԭRT����ʱ������Ⱦ��ָ��������Ͳ�����Ӱ���DS��Ⱦ�õ�RT
	LPDIRECT3DSURFACE9 pOldRT;
	if(FAILED(d3ddevice->GetRenderTarget(0, &pOldRT)))
		return E_FAIL;	
	
	// ����ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = &m_PSLumLogFirstWithoutDS4x4;
	
	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pFullSceneDepth)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);

	for(int i=-1; i<4; i++)
	{
		// ÿ��ѭ����������RT��Depth����һ���Ǵ�256x256Scene-->Log 256x256Temp
		if(i==-1)
		{
			m_p256x256TempRT->GetSurfaceLevel(0, &pSurf);
			if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
				return E_FAIL;
			SAFE_RELEASE(pSurf);
		}
		else
		{
			m_ppTexDownTo1[i]->GetSurfaceLevel(0, &pSurf);
			if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
				return E_FAIL;
			SAFE_RELEASE(pSurf);
		}
		
		// �������Image��ͼ����ͼ��С
		D3DSURFACE_DESC DescTex;
		if(i==-1)
		{
			// ����ǵ�һ����Ⱦ����ô��ԴӦ����256x256Scene����ʱlog��256x256Temp��
			if(FAILED(d3ddevice->SetTexture(0, m_p256x256SceneRT)))
				return E_FAIL;
			pPS = &m_PSLumLogFirstWithoutDS4x4;
		}
		else if(i==0)
		{
			// ����ǵڶ�����Ⱦ����ô��ԴӦ����256x256Temp���������Դ����DownTo1����һ��
			if(FAILED(d3ddevice->SetTexture(0, m_p256x256TempRT)))
				return E_FAIL;
			pPS = &m_PSDownScale4x4;
		}
		else
		{
			if(FAILED(d3ddevice->SetTexture(0, m_ppTexDownTo1[i-1])))
				return E_FAIL;
			if(i==3)
				pPS = &m_PSLumLogLast;
			else
				pPS = &m_PSDownScale4x4;
		}
		
		// Rendering
		if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
			return E_FAIL;
		
		if(FAILED(pVS->SetTransform(NULL)))
			return E_FAIL;
		if(FAILED(pPS->SetPixelShader()))
			return E_FAIL;
		
		// ������������ƫ�Ƶ�VS��PS��c100��c30�����ڵ�һ����Ⱦ����ͬsize��ֻ����һ�εģ������ò�������ֵ
		m_ppTexDownTo1[i]->GetLevelDesc(0, &DescTex);
		D3DXVECTOR4 TexCoordBias = D3DXVECTOR4(1/(float)DescTex.Width, 1/(float)DescTex.Height, 0.5f/(float)DescTex.Width, 0.5f/(float)DescTex.Height);
		pVS->SetConstant(100, &TexCoordBias, 1);
		pPS->SetConstant(30, &TexCoordBias, 1);
		
		// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
		d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		
		// ����
		if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
			return E_FAIL;
		
		// ��Ⱦ��ϣ��ָ�
		pPS->RestorePixelShader();
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	}
	
	
	// �ָ�RT
	if(FAILED(d3ddevice->SetRenderTarget(0, pOldRT)))
		return E_FAIL;
	SAFE_RELEASE(pOldRT);
	
	
	// �����ڲ�����������ʹ��m_pTex/RTָ�룬���Բ��ý���
	m_bMeasureLuminance = true;
	return S_OK;
}





HRESULT KPostProcess::BrightPass(float fKeyValue, float fClampValue, float fOffset)
{
	if(m_iCreateAttrib!= 4 || !m_pScaleSceneRT)
		return E_FAIL;
	if(m_bHDR && (!m_bMeasureLuminance || !m_pLumAdaptCurrentFrameRT) )
		return E_FAIL;
	
	// ����RT��ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = m_bHDR ? &m_PSBrightPass : &m_PSBrightPassLDR;

	m_pTemp1RT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);
	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;
	
	
	// Post Processing, Now Start...
	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;
	
	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;
	
	// �������Image��ͼ����ͼ��С
	if(FAILED(d3ddevice->SetTexture(0, m_pScaleSceneRT)))
		return E_FAIL;
	if(m_bHDR)
		if(FAILED(d3ddevice->SetTexture(1, m_pLumAdaptCurrentFrameRT)))
			return E_FAIL;
	
	// ������������ƫ�Ƶ�VS��PS��c100��c30
	D3DSURFACE_DESC Desc;
	m_pScaleSceneRT->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	// ����BrightPass�Ĳ�����27��29��LDRֻ��27��28��
	pPS->SetConstant(27, &D3DXVECTOR4(fKeyValue, fKeyValue, fKeyValue, 1), 1);
	pPS->SetConstant(28, &D3DXVECTOR4(fClampValue, fClampValue, fClampValue, 0), 1);
	pPS->SetConstant(29, &D3DXVECTOR4(fOffset, fOffset, fOffset, 0), 1);
	
	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
	// �ֶ�����m_pTex/RT��ָ��
	m_pTexture = m_pTemp1RT;
	m_pRT = m_pTemp2RT;
	return S_OK;
}

HRESULT KPostProcess::ToneMapping(float fKeyValue)
{
	if(m_bCalcToneMap)
		return S_OK;
	if(m_iCreateAttrib!= 4 || !m_bMeasureLuminance || !m_pLumAdaptCurrentFrameRT || !m_pScaleSceneRT)
		return E_FAIL;
	
	// ����RT��ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = &m_PSToneMapping;

	m_pToneMapRT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);
	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;
	
	
	// Post Processing, Now Start...
	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;
	
	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;
	
	// �������Image��ͼ����ͼ��С
	if(FAILED(d3ddevice->SetTexture(0, m_pSourceSceneRT)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(1, m_pLumAdaptCurrentFrameRT)))
		return E_FAIL;
	
	// ������������ƫ�Ƶ�VS��PS��c100��c30
	D3DSURFACE_DESC Desc;
	m_pSourceSceneRT->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
		// ����KeyValue
	pPS->SetConstant(31, &D3DXVECTOR4(fKeyValue, fKeyValue, fKeyValue, 1), 1);

	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
	
	// ���ܰ�ָ���ΪToneMap�ģ�������ָ��ͬ��С�������ˣ�ֻҪ���ù����������EndProcess���Զ�����ToneMap����SourceScene
	m_bCalcToneMap = true;
	return S_OK;
}


HRESULT KPostProcess::CalcAdapt()
{
	if(m_bCalcAdapt)
		return S_OK;
	if(m_iCreateAttrib!=4 || !m_bHDR || !m_ppTexDownTo1 || !m_ppTexDownTo1[3] || !m_pLumAdaptPrevFrameRT || !m_pLumAdaptCurrentFrameRT)
		return E_FAIL;

	static bool s_bFirstRun = false;
	LPDIRECT3DSURFACE9 pSrcSurf = NULL, pDstSurf = NULL;

	// ����ǵ�һ�����У�����Ӧ��ֻ��ʼ����������Ϊ��һ֡��HDRȫ������
	if(!s_bFirstRun)
	{
		// ������������m_pLumAdaptCurrentFrameRT�ϣ����ڷ�HDR����ʹ�ã������HDR����ô�����ڼ�����Ӧ��ʱ���Զ�����m_pLumAdaptCurrentFrameRTһ��
		if(FAILED(m_ppTexDownTo1[3]->GetSurfaceLevel(0, &pSrcSurf)))
			return E_FAIL;
		
		// ͬ��С�����ƣ�ѡPOINT���ˣ����Ƶ�Cur��Prev
		if(FAILED(m_pLumAdaptPrevFrameRT->GetSurfaceLevel(0, &pDstSurf)))
			return E_FAIL;
		if(FAILED(d3ddevice->StretchRect(pSrcSurf, NULL, pDstSurf, NULL, D3DTEXF_POINT)))
			return E_FAIL;
		SAFE_RELEASE(pDstSurf);

		if(FAILED(m_pLumAdaptCurrentFrameRT->GetSurfaceLevel(0, &pDstSurf)))
			return E_FAIL;
		if(FAILED(d3ddevice->StretchRect(pSrcSurf, NULL, pDstSurf, NULL, D3DTEXF_POINT)))
			return E_FAIL;
		SAFE_RELEASE(pDstSurf);

		SAFE_RELEASE(pSrcSurf);
		s_bFirstRun = true;
	}

	else
	{
		// Prev = Cur��������Prev���㵽��Cur�ϡ��Ƚ���Prev��Cur���ټ����µ�Cur
		LPDIRECT3DTEXTURE9 pTexTemp = NULL;
		pTexTemp = m_pLumAdaptPrevFrameRT;
		m_pLumAdaptPrevFrameRT = m_pLumAdaptCurrentFrameRT;
		m_pLumAdaptCurrentFrameRT = pTexTemp;

		// ����ʱ�������Ӧ
		if(FAILED(CalcAdaptShader(CameraChange.GetFrameTime(), CameraChange.GetIdleTime())))
			return E_FAIL;
	}

	m_bCalcAdapt = true;
	return S_OK;
}








/**************************�����ڲ����ܽӿ�********************************/
HRESULT KPostProcess::DownScale2x2()
{
	if(m_iCreateAttrib<2 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;

	// ����RT��ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = &m_PSDownScale2x2;

	m_pRT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);

	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;
	
	
	// Post Processing, Now Start...
	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;
	
	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;
	
	// �������Image��ͼ����ͼ��С
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;

	// ������������ƫ�Ƶ�VS��PS��c100��c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	
	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
		
	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	

	// ��ס�����ǲ�ͬSize�ģ����ҳ�ʼ����ʱ����ã����Բ��ܽ���m_pTex/RT��ָ��
	return S_OK;
}

HRESULT KPostProcess::DownScale4x4()
{
	if(m_iCreateAttrib<2 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;
	
	// ����RT��ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = &m_PSDownScale4x4;
	
	V_RETURN(SetTexturedRenderTarget(0, m_pRT, m_pDepth));
	
	// Post Processing, Now Start...
	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;
	
	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;
	
	// �������Image��ͼ����ͼ��С
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;
	
	// ������������ƫ�Ƶ�VS��PS��c100��c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	
	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
		
	// ��ס�����ǲ�ͬSize�ģ����ҳ�ʼ����ʱ����ã����Բ��ܽ���m_pTex/RT��ָ��
	return S_OK;
}



HRESULT KPostProcess::BilinearResize(bool bUseOldRT /* = false */)
{
	if(m_iCreateAttrib<2 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;
	if(bUseOldRT && !m_pOldRT)
		return D3DERR_INVALIDCALL;
	
	// ����RT��ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSBilinearResize;
	PIXELSHADER *pPS = &m_PSBilinearResize;
	
	if(bUseOldRT)
	{
		if(FAILED(d3ddevice->SetRenderTarget(0, m_pOldRT)))
			return E_FAIL;
	}
	else
	{
		m_pRT->GetSurfaceLevel(0, &pSurf);
		if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
			return E_FAIL;
		SAFE_RELEASE(pSurf);
	}
	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;
	
	
	// �ı�ר�ö��㻺�壬�����������������������VSBilinear
	D3DSURFACE_DESC Desc;
	if(FAILED(m_pTexture->GetLevelDesc(0, &Desc)))
		return E_FAIL;
	MYIMAGEVERTEXTYPE MyImageVertex[4] = 
	{
		{-1,-1,0,	0,(float)Desc.Height},
		{-1,1,0,	0,0},
		{1,-1,0,	(float)Desc.Width,(float)Desc.Height},
		{1,1,0,		(float)Desc.Width,0}
	};
	
	VOID* pVertexStream;
	if(m_pBilinearVertexBuffer->Lock(0, 4*sizeof(MYIMAGEVERTEXTYPE), &pVertexStream, 0))
		return E_FAIL;
	memcpy(pVertexStream, MyImageVertex, 4*sizeof(MYIMAGEVERTEXTYPE));
	m_pBilinearVertexBuffer->Unlock();
	
	// PostProcessing, Begin...
	if(FAILED(d3ddevice->SetStreamSource(0, m_pBilinearVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;
	
	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;
	
	// �������Image��ͼ����ͼ��С
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;
	
	// ������������ƫ�Ƶ�PS��c30
	if(FAILED(m_pTexture->GetLevelDesc(0, &Desc)))
		return E_FAIL;
	D3DXVECTOR4 TexCoordBias( 1/((float)Desc.Width), 1/((float)Desc.Height), (float)Desc.Width, (float)Desc.Height );
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
	
	// ��ס�����ǲ�ͬSize�ģ�����Bloom��ʱ����ã����Բ��ܽ���m_pTex/RT��ָ��
	return S_OK;
}


HRESULT KPostProcess::CopyResize(bool bUseOldRT /* = false */)
{
	if(m_iCreateAttrib<2 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;

	// ����RT��ָ��
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = &m_PSCopyResize;

	if(bUseOldRT)
	{
		if(FAILED(d3ddevice->SetRenderTarget(0, m_pOldRT)))
			return E_FAIL;
	}
	else
	{
		m_pRT->GetSurfaceLevel(0, &pSurf);
		if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
			return E_FAIL;
		SAFE_RELEASE(pSurf);
	}
	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;
	
	
	// Post Processing, Now Start...
	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;
	
	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;
	
	// �������Image��ͼ����ͼ��С
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;
	
	// ������������ƫ�Ƶ�VS��PS��c100��c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	
	// ���ò�ͬFilter����ĳ����Ĵ�������Ⱦ״̬�������״̬��������ͼ
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// ����
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// ��Ⱦ��ϣ��ָ�
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	
	
	// ��ס�����ǲ�ͬSize�ģ����ҳ�ʼ����ʱ����ã����Բ��ܽ���m_pTex/RT��ָ��
	return S_OK;
}












HRESULT KPostProcess::CopyTexture(LPDIRECT3DTEXTURE9 pTexSrc, LPDIRECT3DTEXTURE9 pTexDst)
{
	if(!pTexSrc || !pTexDst)
		return E_FAIL;

	LPDIRECT3DSURFACE9 pSurfSrc = NULL, pSurfDst = NULL;

	if(FAILED(pTexSrc->GetSurfaceLevel(0, &pSurfSrc)))
		return E_FAIL;
	
	// ͬ��С�����ƣ�ѡPOINT����
	if(FAILED(pTexDst->GetSurfaceLevel(0, &pSurfDst)))
		return E_FAIL;
	
	if(FAILED(d3ddevice->StretchRect(pSurfSrc, NULL, pSurfDst, NULL, D3DTEXF_POINT)))
		return E_FAIL;

	SAFE_RELEASE(pSurfDst);
	SAFE_RELEASE(pSurfSrc);

	return S_OK;
}

float KPostProcess::GetGaussianWeight(float fX, float fY, float fRadius)
{
	float fDenominator = sqrtf(2 * D3DX_PI) * fRadius;
	return 1 / ( expf( (fX*fX + fY*fY) / (fRadius*fRadius) ) * fDenominator);
}


// 1D Guassian -8��8��H��V��ʹ���ĸ�shader������
void KPostProcess::SetGaussian1D(float fRadius)
{
	//(GetGaussianWeight(1, 3), GetGaussianWeight(2, 3), GetGaussianWeight(3, 3), GetGaussianWeight(4, 3));
	//(GetGaussianWeight(5, 3), GetGaussianWeight(6, 3), GetGaussianWeight(7, 3), GetGaussianWeight(8, 3));
	//(GetGaussianWeight(0, 3), 0, 0.5f, 1);
	D3DXVECTOR4 GaussWeight[3];
	
	//(GetGaussianFix(8, 3), GetGaussianFix(8, 3), GetGaussianFix(8, 3), 1);
	D3DXVECTOR4 GaussFix;

	float fSum = 0;
	float *pVec4 = (float *)GaussWeight;
	for(int i=1; i<=8; i++, pVec4++)
	{
		*pVec4 = GetGaussianWeight((float)i, 0, fRadius);
		fSum += (2 * *pVec4);
	}
	// �����˼���0��ֵ
	float fZero = GetGaussianWeight(0, 0, fRadius);
	fSum += fZero;
	
	GaussWeight[2] = D3DXVECTOR4(fZero, fZero, fZero, fZero);
	fSum = 1 / fSum * 1.3f;
	GaussFix = D3DXVECTOR4(fSum, fSum, fSum, 1);

	m_PSGaussianH.SetConstant(10, &GaussWeight, 3);
	m_PSGaussianH.SetConstant(31, &GaussFix, 1);
}


// Gaussian 5x5����������һ��û��
void KPostProcess::SetGaussian5x5(float fRadius)
{
	D3DXVECTOR4 GaussWeight[4];
	D3DXVECTOR4 GaussFix;
	
	float fSum = 0;
	float *pVec4 = (float *)GaussWeight;

	// ����13������
	*pVec4 = GetGaussianWeight(0, -2, fRadius);
	fSum += (2 * *pVec4);
	pVec4++;
	*pVec4 = GetGaussianWeight(-1, -1, fRadius);
	fSum += (2 * *pVec4);
	pVec4++;
	*pVec4 = GetGaussianWeight(0, -1, fRadius);
	fSum += (2 * *pVec4);
	pVec4++;
	*pVec4 = GetGaussianWeight(1, -1, fRadius);
	fSum += (2 * *pVec4);
	pVec4++;
	*pVec4 = GetGaussianWeight(-2, 0, fRadius);
	fSum += *pVec4;
	pVec4++;
	*pVec4 = GetGaussianWeight(-1, 0, fRadius);
	fSum += *pVec4;
	pVec4++;
	*pVec4 = GetGaussianWeight(1, 0, fRadius);
	fSum += *pVec4;
	pVec4++;
	*pVec4 = GetGaussianWeight(2, 0, fRadius);
	fSum += *pVec4;
	
	// ����(0,0)
	float fZero = GetGaussianWeight(0, 0, fRadius);
	fSum += fZero;

	// ������VEC4�͵�һ����һ���ģ���Ϊ��X��ͬ������ֵ��ͬ������£�����Y�ĸ�˹����ֵ��ȫһ�£�Ϊ�����Ч������ֱ�Ӹ�ֵ
	GaussWeight[2] = GaussWeight[0];
	GaussWeight[3] = D3DXVECTOR4(fZero, fZero, fZero, fZero);

	// ��������ֵ
	fSum = 1 / fSum * 1.3f;
	GaussFix = D3DXVECTOR4(fSum, fSum, fSum, 1);
	

	m_PSGaussian5x5.SetConstant(20, &GaussWeight, 4);
	m_PSGaussian5x5.SetConstant(31, &GaussFix, 1);
}