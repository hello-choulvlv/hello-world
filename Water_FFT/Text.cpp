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
	HRESULT hr = D3DXCreateFont( d3ddevice, FontSize, FontSize/2, 0, 0, FALSE, ANSI_CHARSET, OUT_TT_ONLY_PRECIS, 1, 0, FontName, &DFont ); //从已有字体创建接口
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
	if(FAILED(DFont->DrawText(NULL, Content, Length, &Rect, //文本的显示区域，使用窗口坐标
			            	  TextFormat, //显示格式：左对齐
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
	if(FAILED(DFont->DrawText(NULL, Content, Length, &Rect, //文本的显示区域，使用窗口坐标
			            	  TextFormat, //显示格式：左对齐
							  D3DCOLOR_ARGB(a,r,g,b))))
		att=E_FAIL;
	d3ddevice->EndScene();
	return att;
}



//3D字体部分
HRESULT MYTEXT::InitFont3D(UINT FontSize, LPSTR FontName, LPSTR Content, float deviation, float extrusion)
{
	Deviation=deviation; Extrusion=extrusion;
	HDC memdc;
	memdc=CreateCompatibleDC( NULL );
	//在这里定义字体的属性，比如斜体、黑体和下划线，具体看SDK
	HFONT hFont = CreateFont(FontSize, 0, 0, 0, 0, 0, 0, 0,	ANSI_CHARSET, 0, 0, 0, 0, FontName);
	HFONT OldFont = (HFONT)SelectObject(memdc, hFont);

	//创建三维文本的Mesh对象
	HRESULT hr = D3DXCreateText(d3ddevice, memdc, Content,
								Deviation, //定义了字体轮廓的圆滑程度，取值越小，字体越圆滑
								Extrusion, //文本在Z轴方向上的厚度
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
	Material.Ambient.r = DiffuseR;  //一般材质反射和漫反射光线颜色相同，如果不同的等调用完函数后再自己设置
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
	//释放纹理
	for(UINT i=0;i<TextureNum;i++) SAFE_RELEASE(Texture[i]);
	//最后释放顶点缓冲区
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
	
	//创建顶点缓冲区
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

	//得到剪裁后的边界数据，主要是剪裁PIC两个边界，屏幕输出的边界系统会自动剪裁
	if(width==0&&height==0) ext0sign=1;
	if(picx<0||picy<0||picx>=(int)desc.Width||picy>=(int)desc.Height) return E_FAIL;  //判别溢出
	if(ext0sign) {width=desc.Width; height=desc.Height;}
	if(picx+width>=(int)desc.Width) width=desc.Width-picx;
	if(picy+height>=(int)desc.Height) height=desc.Height-picy;
	if(width<=0||height<=0) return E_FAIL;

	//转换为顶点数据
	MyImageVertex[1].atu=(float)picx/desc.Width; MyImageVertex[1].atv=(float)picy/desc.Height;
	MyImageVertex[0].atu=(float)picx/desc.Width; MyImageVertex[0].atv=(float)(picy+height)/desc.Height;
	MyImageVertex[3].atu=(float)(picx+width)/desc.Width; MyImageVertex[3].atv=(float)picy/desc.Height;
	MyImageVertex[2].atu=(float)(picx+width)/desc.Width; MyImageVertex[2].atv=(float)(picy+height)/desc.Height;
	
	d3ddevice->GetTexture(1, &AlphaTexture);
	d3ddevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &AlphaOP);
	d3ddevice->GetTextureStageState(1, D3DTSS_ALPHAOP, &AlphaOP_1);
	d3ddevice->GetTextureStageState(1, D3DTSS_ALPHAARG1, &AlphaArg1_1);
	//设置纹理状态和渲染状态
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
	if(startx>=(int)d3ddm.Width||starty>=(int)d3ddm.Height||endx<startx||endy<starty||endx<0||endy<0) return E_FAIL;  //判别溢出

	//转换为顶点数据
	MyImageVertex[1].x=(float)startx; MyImageVertex[1].y=(float)starty;
	MyImageVertex[0].x=(float)startx; MyImageVertex[0].y=(float)endy;
	MyImageVertex[3].x=(float)endx; MyImageVertex[3].y=(float)starty;
	MyImageVertex[2].x=(float)endx; MyImageVertex[2].y=(float)endy;
	
	//填充顶点缓冲区
	VOID* VertexStream;
	if(VertexBuffer->Lock(0, 4*EachPointSize, &VertexStream, 0))
		return E_FAIL;
	memcpy(VertexStream, MyImageVertex, 4*EachPointSize);
	VertexBuffer->Unlock();

	//开始渲染
	if(FAILED(d3ddevice->SetStreamSource(0, VertexBuffer, 0, EachPointSize)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetFVF(VertexShader)))
		return E_FAIL;

  
	//保存TSS设置，COLOROP ALPHAOP ALPHABLENDING ALPHATEST
	Save();

	if(alphabmpenable==0) d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	else
	{
		d3ddevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		d3ddevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		d3ddevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	}
	//设置纹理状态和渲染状态
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

	//得到剪裁后的边界数据，主要是剪裁PIC两个边界，屏幕输出的边界系统会自动剪裁
	if(width==0&&height==0) ext0sign=1;
	if(startx>=(int)d3ddm.Width||starty>=(int)d3ddm.Height||endx<startx||endy<starty||endx<0||endy<0||picx<0||picy<0||picx>=(int)desc.Width||picy>=(int)desc.Height) return E_FAIL;  //判别溢出
	if(ext0sign) {width=desc.Width; height=desc.Height;}
	if(picx+width>=(int)desc.Width) width=desc.Width-picx;
	if(picy+height>=(int)desc.Height) height=desc.Height-picy;
	if(width<=0||height<=0) return E_FAIL;

	//转换为顶点数据
	MyImageVertex[1].x=(float)startx; MyImageVertex[1].y=(float)starty;
	MyImageVertex[1].tu=(float)picx/desc.Width; MyImageVertex[1].tv=(float)picy/desc.Height;
	MyImageVertex[0].x=(float)startx; MyImageVertex[0].y=(float)endy;
	MyImageVertex[0].tu=(float)picx/desc.Width; MyImageVertex[0].tv=(float)(picy+height)/desc.Height;
	MyImageVertex[3].x=(float)endx; MyImageVertex[3].y=(float)starty;
	MyImageVertex[3].tu=(float)(picx+width)/desc.Width; MyImageVertex[3].tv=(float)picy/desc.Height;
	MyImageVertex[2].x=(float)endx; MyImageVertex[2].y=(float)endy;
	MyImageVertex[2].tu=(float)(picx+width)/desc.Width; MyImageVertex[2].tv=(float)(picy+height)/desc.Height;
	
	//填充顶点缓冲区
	VOID* VertexStream;
	if(VertexBuffer->Lock(0, 4*EachPointSize, &VertexStream, 0))
		return E_FAIL;
	memcpy(VertexStream, MyImageVertex, 4*EachPointSize);
	VertexBuffer->Unlock();

	//开始渲染
	if(FAILED(d3ddevice->SetStreamSource(0, VertexBuffer, 0, EachPointSize)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetFVF(VertexShader)))
		return E_FAIL;

	//保存TSS设置，COLOROP ALPHAOP ALPHABLENDING ALPHATEST
	Save();

	if(FAILED(d3ddevice->SetTexture(0, Texture[picno])))
	return E_FAIL;

	//设置纹理状态和渲染状态
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

	//得到剪裁后的边界数据，主要是剪裁PIC两个边界，屏幕输出的边界系统会自动剪裁
	if(width==0&&height==0) ext0sign=1;
	if(startx>=(int)d3ddm.Width||starty>=(int)d3ddm.Height||endx<startx||endy<starty||endx<0||endy<0||picx<0||picy<0||picx>=(int)desc.Width||picy>=(int)desc.Height) return E_FAIL;  //判别溢出
	if(ext0sign) {width=desc.Width; height=desc.Height;}
	if(picx+width>=(int)desc.Width) width=desc.Width-picx;
	if(picy+height>=(int)desc.Height) height=desc.Height-picy;
	if(width<=0||height<=0) return E_FAIL;

	//转换为顶点数据
	MyImageVertex[1].x=(float)startx; MyImageVertex[1].y=(float)starty;
	MyImageVertex[1].tu=(float)picx/desc.Width; MyImageVertex[1].tv=(float)picy/desc.Height;
	MyImageVertex[0].x=(float)startx; MyImageVertex[0].y=(float)endy;
	MyImageVertex[0].tu=(float)picx/desc.Width; MyImageVertex[0].tv=(float)(picy+height)/desc.Height;
	MyImageVertex[3].x=(float)endx; MyImageVertex[3].y=(float)starty;
	MyImageVertex[3].tu=(float)(picx+width)/desc.Width; MyImageVertex[3].tv=(float)picy/desc.Height;
	MyImageVertex[2].x=(float)endx; MyImageVertex[2].y=(float)endy;
	MyImageVertex[2].tu=(float)(picx+width)/desc.Width; MyImageVertex[2].tv=(float)(picy+height)/desc.Height;
	
	//填充顶点缓冲区
	VOID* VertexStream;
	if(VertexBuffer->Lock(0, 4*EachPointSize, &VertexStream, 0))
		return E_FAIL;
	memcpy(VertexStream, MyImageVertex, 4*EachPointSize);
	VertexBuffer->Unlock();

	//开始渲染
	if(FAILED(d3ddevice->SetStreamSource(0, VertexBuffer, 0, EachPointSize)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetFVF(VertexShader)))
		return E_FAIL;

	//保存纹理指针和TSS设置，COLOROP ALPHAOP ALPHABLENDING ALPHATEST
	Save();

	if(FAILED(d3ddevice->SetTexture(0, Texture[picno])))
		return E_FAIL;
	
	//设置纹理状态和渲染状态
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















/*******************************系统接口*********************************/
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

	// 释放默认的RT和Depth
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
	// 先释放所有的RT和Depth，以便重新创建（因为每帧都要调用该函数）
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
	
	// 释放深度缓冲
	SAFE_RELEASE(m_pFullSceneDepth);
	
	// 重置几个指针
	m_pTexture = NULL;
	m_pRT = NULL;
	m_pDepth = NULL;

	// 重置标记
	m_bMeasureLuminance = m_bCalcAdapt = m_bCalcEdge = m_bCalcToneMap = false;
	m_bHDR = m_bMRT = false;
	m_bGlare = false;
}












/*******************************管理接口*********************************/
HRESULT KPostProcess::Init()
{
	if(m_iCreateAttrib)
		return E_FAIL;

	ZeroMemory(&m_DescRT, sizeof(D3DSURFACE_DESC));
	ZeroMemory(&m_DescDepth, sizeof(D3DSURFACE_DESC));

	// 初始化面片的顶点缓冲
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
	
	//填充顶点缓冲区
	VOID* pVertexStream;
	if(m_pVertexBuffer->Lock(0, 4*sizeof(MYIMAGEVERTEXTYPE), &pVertexStream, 0))
		return E_FAIL;
	memcpy(pVertexStream, MyImageVertex, 4*sizeof(MYIMAGEVERTEXTYPE));
	m_pVertexBuffer->Unlock();

	// 初始化Shader
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
		mymessage("显卡不支持Pixel Shader 2.0，无法正常进行Post Processing！");
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

	// Depth Of Field，如果不支持足够的ps2.a，就不能编译shader
	if(CheckPS2xSupport(21, FALSE))
	{
		if(FAILED(m_PSDOF.InitPixelShader("shader\\PostProcessing\\DOF\\DOFCombine.psh")))
			return E_FAIL;
	}

	// 结束
	m_iCreateAttrib = 1;
	return S_OK;
}

HRESULT KPostProcess::StartRender(bool bHDR /* = false */, bool bMRT /* = false */)
{
	if(!m_iCreateAttrib || !m_pVertexBuffer)
		return E_FAIL;

	// HDR和MRT必须用浮点纹理，必须预先定义，如果未定义FP16/32宏，就出错，如果都定义了，FP16优先
#ifndef USE_FP16
#ifndef USE_FP32
	if(bHDR || bMRT)
	{
		OutputDebugString("\n错误！要使用HDR或者MRT，必须先定义使用FP16或FP32！！\n");
		return D3DERR_INVALIDCALL;
	}
#endif
#endif


	int i = 0;
	LPDIRECT3DSURFACE9 pSurf = NULL;
	D3DSURFACE_DESC Desc, DescDepth;	// 用于和原来的格式作对比，如果分辨率或像素格式有任一个不符合，就重新释放重建
	
// 得到旧的后台缓冲和深度缓冲及属性
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

	// 如果每次都重新分配就太费时了
	// 如果原始RT的分辨率或像素格式改变了，就重新分配。用于多次调用该函数或嵌套调用
	// 如果HDR或MRT的设置也和上次不同了，照样重新分配，避免同一个KPost对象先后拦截不同需要的场景，而又没有分配对应的纹理
	if(m_bHDR != bHDR || m_bMRT != bMRT || Desc.Width!=m_DescRT.Width || Desc.Height!=m_DescRT.Height || Desc.Format!=m_DescRT.Format)
	{
		FreeTex();

		// 这个和深度缓冲都要放在FreeTex的下面
		m_bHDR = bHDR;
		m_bMRT = bMRT;
		
		// 如果是HDR，就用浮点纹理（这里使用FP16就够了），否则就要整数纹理
		// 不要被迷惑了，m_DescRT只是为了判断后台缓冲格式是否改变，还有就是用来作为原大、1/4大小的创建依据，但它的像素格式跟我们内部的HDR纹理格式并无直接关系
#ifdef USE_FP16
		D3DFORMAT Format = m_bHDR ? D3DFMT_A16B16G16R16F : D3DFMT_A8R8G8B8;
		// 注意这里，如果用单通道FP16，就用G16R16而不是R16，因为GF6/7都不支持R16F
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
		// 如果是MRT，强制用浮点纹理（为了让主RT跟MRT格式匹配）
		Format = m_bMRT ? D3DFMT_A16B16G16R16F : Format;


		// 初始化原图，一张原大，一张1/4，可以是HDR
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pSourceSceneRT)))
			return E_FAIL;
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pScaleSceneRT)))
			return E_FAIL;

		// 初始化Glare贴图
		for(i = 0; i < MAX_GLARE_LINE_NUM; i++)
		{
			if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_ppGlareRT[i])))
				return E_FAIL;
		}

		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pGlareTemp1RT)))
			return E_FAIL;
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pGlareTemp2RT)))
			return E_FAIL;

		// 初始化临时贴图，1/4大，必定为LDR（和HDR相关的都有专用贴图，比如亮度统计、适应和ToneMap），这两张总是存放映射到LDR后的图像
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTemp1RT)))
			return E_FAIL;
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width/4, m_DescRT.Height/4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTemp2RT)))
			return E_FAIL;
		
		// 初始化其他专用贴图
		// DOF两个必定是LDR的，所以强制用D3DFMT_A8R8G8B8
		// 深度转换图必须用浮点，所以要指定FP16和FP32，否则会改用整数纹理出现错误结果
		if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pDOFRT)))
			return E_FAIL;

		// 初始化MRT，法线图和深度图为了精确，都使用浮点纹理，转换后的边缘图用整数纹理即可
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

		// 初始化HDR专用的，亮度贴图和ToneMap
		if(m_bHDR)
		{
			if(FAILED(D3DXCreateTexture(d3ddevice, 256, 256, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_p256x256SceneRT)))
				return E_FAIL;
			if(FAILED(D3DXCreateTexture(d3ddevice, 256, 256, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_p256x256TempRT)))
				return E_FAIL;

			// 64x64->1x1，统计全局亮度用，强制用单通道纹理
			for(i=0; i<4; i++)
				if(FAILED(D3DXCreateTexture(d3ddevice, 64>>(i*2), 64>>(i*2), 1, D3DUSAGE_RENDERTARGET, SingleChannelTexFormat, D3DPOOL_DEFAULT, &m_ppTexDownTo1[i])))
					return E_FAIL;
			// ToneMap必定是LDR的，所以强制用D3DFMT_A8R8G8B8
			if(FAILED(D3DXCreateTexture(d3ddevice, m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pToneMapRT)))
				return E_FAIL;

			// 初始化几个1x1的贴图，亮度适应用，强制用单通道纹理
			if(FAILED(D3DXCreateTexture(d3ddevice, 1, 1, 1, D3DUSAGE_RENDERTARGET, SingleChannelTexFormat, D3DPOOL_DEFAULT, &m_pLumAdaptCurrentFrameRT)))
				return E_FAIL;
			if(FAILED(D3DXCreateTexture(d3ddevice, 1, 1, 1, D3DUSAGE_RENDERTARGET, SingleChannelTexFormat, D3DPOOL_DEFAULT, &m_pLumAdaptPrevFrameRT)))
				return E_FAIL;

		}//end HDR专用贴图
			
	}// end change for Desc


	// 设置深度缓冲，虽然要用比较来筛选，可是当RT的分辨率改变的时候，同时也会释放DEPTH缓冲，所以这里要检测指针是否有效，如果无效，即使DEPTH格式未改变，也要重新分配
	if(!m_pFullSceneDepth || DescDepth.Width!=m_DescDepth.Width || DescDepth.Height!=m_DescDepth.Height || DescDepth.Format!=m_DescDepth.Format)
	{
		// 初始化深度缓冲	
		if(FAILED(d3ddevice->CreateDepthStencilSurface(m_DescDepth.Width, m_DescDepth.Height, m_DescDepth.Format, m_DescDepth.MultiSampleType, m_DescDepth.MultiSampleQuality, FALSE, &m_pFullSceneDepth, NULL)))
			return E_FAIL;
	}

	
	// 设置SourceSceneRT作为RT，渲染完后场景就到它上面去了
	V_RETURN(SetTexturedRenderTarget(0, m_pSourceSceneRT, m_pFullSceneDepth));

	// 清除，拦截渲染图像，就清除这一次，因为渲染图像不一定覆盖全屏，但是后面的Image操作就不用清除了，全部是完全覆盖的
	if(FAILED(d3ddevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0)))
		return E_FAIL;
		
	m_iCreateAttrib = 2;

	return S_OK;
}



HRESULT KPostProcess::EndRender()
{
	if(m_iCreateAttrib!=2 || !m_pOldRT || !m_pOldDepth)
		return E_FAIL;
	
	// 场景渲染完毕，恢复默认RT，但是记住不能释放，他们还有用，要在最后EndProcess的时候作为绘制对象
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
	
	// 置内部原始数据和贴图
	// 将场景Scale或复制到对应的纹理上
	// 这时的原场景已经在SourceScene上了，将原场景DS4x4->SceneScale，这一步将是史无前例的费时，因为原场景非常的大，DS4X4又是采样数最多的，木有办法，waiting吧
	m_pTexture = m_pSourceSceneRT;
	m_pRT = m_pScaleSceneRT;
	m_pDepth = m_pFullSceneDepth;
	if(FAILED(DownScale4x4()))
		return E_FAIL;

	if(m_bHDR)
	{
		// 将原场景DS4x4后的直接Resize到256x256Scene，用于亮度统计，这样做最快，而且色彩损失也较少，当然在低分辨率下比如640*480（1/4只有160*120，Resize to 256x256不是很好）就不是最优方案了，但是性能永远是最好的
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
	// 设置MRT，法线图在第一个，深度图在第二个
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
	
	// 保存StateBlock
	SAFE_RELEASE(m_pStateBlock);
	if(FAILED(d3ddevice->CreateStateBlock(D3DSBT_PIXELSTATE, &m_pStateBlock)))
		return E_FAIL;
	
	d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	d3ddevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 1);
	d3ddevice->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, 1);
	
	// 如果是HDR，是不能直接使用ScaleScene的，无论Bloom还是ToneMap，都是直接映射到LDR后才弄到TEMP中的
	// 所以如果是HDR，这里就不用将ScaleScene弄到Temp1中了，多此一举
	if(!m_bHDR)
	{
		// 将Scale Copy->Temp1
		m_pTexture = m_pScaleSceneRT;
		m_pRT = m_pTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(Resize()))
			return E_FAIL;
	}
	
	// 置几个指针
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
	
	// 如果是HDR，是不能直接使用ScaleScene的，无论Bloom还是ToneMap，都是直接映射到LDR后才弄到TEMP中的
	// 所以如果是HDR，这里就不用将ScaleScene弄到Temp1中了，多此一举
	if(!m_bHDR)
	{
		// 将Scale Copy->Temp1
		m_pTexture = m_pScaleSceneRT;
		m_pRT = m_pTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(Resize()))
			return E_FAIL;
	}
	
	// 置几个指针
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

	// 创建原大的，以便EndProcess往上贴
	if(FAILED(d3ddevice->CreateTexture(m_DescRT.Width, m_DescRT.Height, 1, D3DUSAGE_RENDERTARGET, Desc.Format, D3DPOOL_DEFAULT, ppTex, NULL)))
		return D3DERR_OUTOFVIDEOMEMORY;

	// 如果是原大到原大，就用纹理复制
	if(Desc.Width == m_DescRT.Width && Desc.Height == m_DescRT.Height)
	{
		if(FAILED(m_pTexture->GetSurfaceLevel(0, &pSrcSurf)))
			return E_FAIL;
		if(FAILED((*ppTex)->GetSurfaceLevel(0, &pDstSurf)))
			return E_FAIL;
		// 纹理复制罢了，就选POINT过滤
		if(FAILED(d3ddevice->StretchRect(pSrcSurf, NULL, pDstSurf, NULL, D3DTEXF_POINT)))
			return E_FAIL;
		SAFE_RELEASE(pSrcSurf);
		SAFE_RELEASE(pDstSurf);
	}
	// 如果大小不同，就用Copy/Bilinear Resize
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

	// 恢复StateBlock
	m_pStateBlock->Apply();
	
	// 恢复原RT，之所以放到StateBlock后面是为了改变RT的设置
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
	// 正确性检测
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

	// 如果未指定PixelShader，就把处理好的图像复制到原RT上去
	if(!pPS)
	{
		// 如果计算过HDR ToneMapping，就直接复制ToneMap，大小相同，用Copy即可
		if(m_bHDR && m_bCalcToneMap)
		{
			m_pTexture = m_pToneMapRT;
			if(FAILED(CopyResize(true)))
				return E_FAIL;
		}
		// 否则就用处理好的图像，处理好的图像就在m_pTexture中
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
	
	// 自定义shader来混合当前场景、自定义纹理和RT
	else
	{
		// 设置最终的RT，把场景作为纹理贴上去进行渲染处理，渲染到最终RT上
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
		
		// 如果计算过ToneMap，就把ToneMap作为原场景，否则就将SourceScene作为原场景，它们其实是一样的，都作为0层纹理
		LPDIRECT3DTEXTURE9 pTexture;
		if(m_bHDR && m_bCalcToneMap && m_pToneMapRT)
			pTexture = m_pToneMapRT;
		else
			pTexture = m_pSourceSceneRT;

		if(FAILED(d3ddevice->SetTexture(0, pTexture)))
			return E_FAIL;
		
		// 设置纹理坐标偏移到VS和PS的c100和c30
		pTexture->GetLevelDesc(0, &Desc);
		D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
		pVS->SetConstant(100, &TexCoordBias, 1);
		
		// 如果自定义纹理有效，就设置到第0层，并设置参数到VS c101和 PS c31
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
		
		// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
		d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		// 反正第一层不是SourceScene就是ToneMap，都是原大，就用POINT过滤
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		
		// 绘制
		if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
			return E_FAIL;
		
		// 渲染完毕，恢复
		pPS->RestorePixelShader();
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	}// end shader blend to RT


	// 恢复StateBlock
	m_pStateBlock->Apply();
	
	
	// 恢复原RT，之所以放到StateBlock后面是为了不让StateBlock覆盖掉这里改变RT的设置操作
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
















/*******************************高级外部功能接口***************************/

HRESULT KPostProcess::HDR2LDR(float fKeyValue)
{
	if(!m_bHDR)
		OutputDebugString("\n错误！要使用HDR2LDR，必须在StartRender中指定HDR为true！！\n");
	
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
		
	// 如果未指定参数（为0），根据HDR还是LDR做前两个参数的调整，进行BrightPass
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

	// Glare，这一步只会把Glare叠加到BrightPass上，并不会影响其他的，当然SetGlare后才会生效
	if(m_bGlare)
	{
		// 用BrightPass过的图像(m_pTexture）处理，把模糊过的（多个拖尾）结果暂存到Glare专用贴图集合中
		V_RETURN(Glare());
		// 将所有Glare专用贴图合成到当前的BrightPass贴图（m_pTexture）中
		V_RETURN(AddGlare());
	}
	// 这里为了提高效率，在增大Glare采样间隔的同时还进行了Bloom，如果只想得到尖锐的Glare，将上面AddGlare的渲染目的设为m_pRT，并去掉下面的Gaussian即可



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

	// 如果是DEPTH，就多增强几次，因为Depth一般都比较细
	if(ProcRTType == POSTRTTYPE_DEPTH)
	{
		if(FAILED(EdgeEnhanced(true, bEdgeColorWhite)))
			return E_FAIL;
		if(FAILED(EdgeEnhanced(true, bEdgeColorWhite)))
			return E_FAIL;
		if(FAILED(EdgeEnhanced(true, bEdgeColorWhite)))
			return E_FAIL;
	}
	
	// 边缘图是在EdgeTemp中，要DS4x4到Temp，然后模糊出来效果才好（边缘图真是烦人，一会儿小了不行，一会儿大了不行）
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
	// 默认情况下用最简单的参数，即NearPlane为0，FarPlane = 2 * FocalPlane
	if(fFarPlane == 0.0f)
		fFarPlane = fFocalPlane * 2.0f;
	if(fNearPlane > fFocalPlane || fFocalPlane > fFarPlane || fNearPlane > fFarPlane || fNearPlane < 0 || fFocalPlane < 0 || fFarPlane < 0)
		return D3DERR_INVALIDCALL;

	if(CheckPS2xSupport(21, FALSE))
	{
		mymessage("显卡不支持Pixel Shader 2.0a，无法渲染Depth Of Field！")
		return E_FAIL;
	}


	// 从SourceScene的alpha通道恢复Camera Depth和Blur Coef，这是固定的从SourceScene到DOFDepthInfoRT，不用设置m_pTex/RT
	//if(FAILED(DOFConvertDepthInfoShader(fFocalPlane, fNearPlane, fFarPlane)))
	//	return E_FAIL;


	// 在HDR中，因为DOF是最终显示的图像，所以不能用ScaleScene，要用ToneMap
	if(m_bHDR)
	{
		// 没计算HDR，强制进行Tone Mapping
		if(!m_bCalcToneMap)
		{
			if(FAILED(HDR2LDR()))
				return E_FAIL;
		}

		// 将m_pToneMap DownScale4x4到TempRT上，以便Gaussian
		m_pTexture = m_pToneMapRT;
		m_pRT = m_pTemp1RT;
		if(FAILED(DownScale4x4()))
			return E_FAIL;
	}
	else
	{
		// 把ScaleScene Copy到TempRT上，以便Gaussian
		m_pTexture = m_pScaleSceneRT;
		m_pRT = m_pTemp1RT;
		if(FAILED(CopyResize()))
			return E_FAIL;
	}

	// 到了这里，HDR和非HDR场景都已经将1/4小图转换到TempRT上了，下来都是公用步骤
	// 得到Gaussian的模糊小图在m_pTexture中
	m_pTexture = m_pTemp1RT;
	m_pRT = m_pTemp2RT;
	if(FAILED(Gaussian1D(true)))
		return E_FAIL;
	if(FAILED(Gaussian1D(false)))
		return E_FAIL;

	// 如果是HDR，就用ToneMap，反之用SourceScene，将原图和模糊后的小图传入进行DOF处理
	m_pRT = m_pDOFRT;
	if(FAILED(DOFShader(m_bHDR ? m_pToneMapRT : m_pSourceSceneRT, m_pTexture, fFocalPlane, fNearPlane, fFarPlane, fBlurPower)))
		return E_FAIL;

	m_pTexture = m_pDOFRT;

	// Test
	//D3DXSaveTextureToFile("c:\\dof.tga", D3DXIFF_TGA, m_pDOFRT, NULL);
	return S_OK;
}






/*******************************组合内部功能接口*********************************/

HRESULT KPostProcess::Resize()
{
	if(m_iCreateAttrib<2 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;

	// 先得到m_pTexture的属性，用来判断是Bilinear还是Copy
	D3DSURFACE_DESC DescTex, DescRT;
	if(FAILED(m_pTexture->GetLevelDesc(0, &DescTex)))
		return E_FAIL;
	if(FAILED(m_pRT->GetLevelDesc(0, &DescRT)))
		return E_FAIL;

	// 如果大小完全相同，那就直接Copy（不牵扯纹理过滤）
	if(DescRT.Width == DescTex.Width && DescRT.Height == DescTex.Height)
		return CopyResize();
	// 大小不同就要考虑纹理过滤了，如果为浮点纹理，就用Shader过滤
	else if(DescTex.Format == D3DFMT_A16B16G16R16F || DescTex.Format == D3DFMT_G16R16F || DescTex.Format == D3DFMT_R16F || DescTex.Format == D3DFMT_A32B32G32R32F || DescTex.Format == D3DFMT_G32R32F || DescTex.Format == D3DFMT_R32F)
		return BilinearResize();
	// 整数纹理的话就用硬件过滤
	else
		return CopyResize();
}




HRESULT KPostProcess::EdgeDetection(POSTPROCESSING_SOURCETYPE ProcRTType /* = POSTRTTYPE_NORMAL */, float fThreshold /* = 0.0f */, bool bEdgeColorWhite /* = true */)
{
	// 因为可能在非MRT下（Sobel），MRT纹理不存在，所以纹理指针不做有效性检测，让DetectionShader去检测好了
	if(m_iCreateAttrib!=4)
		return E_FAIL;
	if(m_bCalcEdge)
		return S_OK;

	// 目的就是转存到EdgeTemp1

	// 用Sobel过滤，直接从Scale->Temp1，因为它是基于亮度的，如果是HDR，还要clamp到1
	if(ProcRTType == POSTRTTYPE_SCENE)
	{
		m_pTexture = m_pScaleSceneRT;
		m_pRT = m_pEdgeTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(EdgeDetectionShader(ProcRTType, fThreshold, bEdgeColorWhite)))
			return E_FAIL;
	}

	// 用法线图来做边缘检测，先转换为原大边缘图NormalEdge，再DS4x4到Temp
	else if(ProcRTType == POSTRTTYPE_NORMAL)
	{
		if(!m_bMRT || m_iMRTAttrib!=5 && m_iMRTAttrib!=7)
			return E_FAIL;

		// 默认是cos30度
		if(fThreshold == 0.0f)
			fThreshold = 0.866f;
		m_pTexture = m_pNormalSceneRT;
		m_pRT = m_pEdgeTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(EdgeDetectionShader(ProcRTType, fThreshold, bEdgeColorWhite)))
			return E_FAIL;
/*
		// 先检测，再DS4x4
		m_pTexture = m_pNormalEdgeRT;
		m_pRT = m_pTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(DownScale4x4()))
			return E_FAIL;
*/
	}

	// 用深度图来做，先DS4x4到DepthEdge(1/4 FP)，再检测
	else if(ProcRTType == POSTRTTYPE_DEPTH)
	{
		if(!m_bMRT || m_iMRTAttrib!=6 && m_iMRTAttrib!=7)
			return E_FAIL;

		// 默认的深度变化量是0.3
		if(fThreshold == 0.0f)
			fThreshold = 0.3f;
		
		m_pTexture = m_pDepthSceneRT;
/*		m_pRT = m_pDepthEdgeRT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(DownScale4x4()))
			return E_FAIL;

		// 先DS4x4，再进行深度检测
		m_pTexture = m_pDepthEdgeRT;
*/		m_pRT = m_pEdgeTemp1RT;
		m_pDepth = m_pFullSceneDepth;
		if(FAILED(EdgeDetectionShader(ProcRTType, fThreshold, bEdgeColorWhite)))
			return E_FAIL;
	}
	else
		return D3DERR_INVALIDCALL;

	// 强制重置指针
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
	
	
	// 根据显卡pixel shader版本判断使用哪个函数
	char p[10] = "";
	// 不支持，返回
	if(!D3DXGetPixelShaderProfile(d3ddevice))
		return E_FAIL;
	
	strcpy(p, D3DXGetPixelShaderProfile(d3ddevice));
	
	// 对于只支持ps2.0的显卡，调用ps2.0专用函数，ps2a ps2b不必理会，执行本函数即可
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


/*这一段用于不进行人眼自动适应时的Tone Mapping，在HDR2LDR中关闭CalcAdapt的同时，也要把这段代码打开
	LPDIRECT3DSURFACE9 pSurfDst = NULL, pSurfSrc = NULL;

	// 将m_ppTexDownTo1[3]复制到CurAdaptRT，在不进行亮度适应的情况下也可以映射到LDR了
	if(FAILED(m_ppTexDownTo1[3]->GetSurfaceLevel(0, &pSurfSrc)))
		return E_FAIL;
	if(FAILED(m_pLumAdaptCurrentFrameRT->GetSurfaceLevel(0, &pSurfDst)))
		return E_FAIL;
	if(FAILED(d3ddevice->StretchRect(pSurfSrc, NULL, pSurfDst, NULL, D3DTEXF_POINT)))
		return E_FAIL;
	SAFE_RELEASE(pSurfSrc);
	SAFE_RELEASE(pSurfDst);	
*/

	// 这是内部操作，不会使用m_pTex/RT指针，所以不用交换
	m_bMeasureLuminance = true;
	return S_OK;
}






/*******************************基本内部功能接口*********************************/
HRESULT KPostProcess::Glare()
{
	if(m_iCreateAttrib != 4 || !m_pGlareTemp1RT || !m_pGlareTemp2RT || !m_pTexture || !m_pDepth)
		return E_FAIL;

	// 设置RT和指针
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSMultiTextureBlend;
	PIXELSHADER *pPS = &m_PSGlare;
	LPDIRECT3DTEXTURE9 pRT = NULL, pTex = NULL;
	D3DSURFACE_DESC Desc;


	// 设置公用参数
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;

	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;

	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;


	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);


	// 多个拖尾方向，分别渲染到GlareRT数组的不同元素中
	for(UINT iTrailNo = 0; iTrailNo < m_GlareData.m_iLineNum; iTrailNo++)
	{
		// 通过拖尾长度计算出模糊次数和采样点间距，其中采样点数为固定值8（这样就不需要在Shader中做动态循环）
		UINT iSampleNum = 8;
			// 尽量增大采样间距而减少模糊次数，但间距不可以过大，否则会采样失真
		UINT iBlurNum = 1;
		UINT iSampleInterval = 8;	// 直接给最大值，如果拖尾长度大于它，再增加模糊次数
		if(m_GlareData.m_pLineData[iTrailNo].iLength > iSampleInterval)
			iBlurNum = m_GlareData.m_pLineData[iTrailNo].iLength / iSampleInterval;
		
		// 处理旋转，得到模糊方向
		D3DXVECTOR2 VecTrialDir;
		D3DXVec2TransformCoord(&VecTrialDir, &m_GlareData.m_pLineData[iTrailNo].VecDir, &m_GlareData.m_MatRotate);
		// 注意是按反方向模糊，而且注意这里的Dir是2D面上的xy坐标系		
		D3DXVec2Normalize(&VecTrialDir, &-VecTrialDir);


		// 计算单个拖尾，分多次渲染，中间用Temp1/2做切换，最后一次渲染到GlareRT中
		for(UINT iBlurNo = 0; iBlurNo < iBlurNum; iBlurNo++)
		{
			if(iBlurNo == 0)
			{
				pTex = m_pTexture;	// 每个拖尾的第一次渲染，来源都是已经渲染好的BrightPass图
				pRT = m_pGlareTemp1RT;
			}
			else
			{
				// 轮换RT和来源贴图
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

			// 最后一次渲染到GlareRT中
			if(iBlurNo == iBlurNum - 1)
				pRT = m_ppGlareRT[iTrailNo];


			// 设置RT
			pRT->GetSurfaceLevel(0, &pSurf);
			if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
				return E_FAIL;
			SAFE_RELEASE(pSurf);

			// 设置来源贴图
			if(FAILED(d3ddevice->SetTexture(0, pTex)))
				return E_FAIL;

			// 设置常量寄存器：沿着模糊方向的采样坐标偏移、1/2tap偏移
			pTex->GetLevelDesc(0, &Desc);
			D3DXVECTOR4 TexCoordBias = D3DXVECTOR4(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
			pVS->SetConstant(100, &TexCoordBias, 1);

			for(UINT i = 0; i < iSampleNum; i++)
			{
				D3DXVECTOR4 PtOffset;
				PtOffset.x = 1.0f / (float)Desc.Width * iSampleInterval * i * VecTrialDir.x;
				PtOffset.y = 1.0f / (float)Desc.Height * iSampleInterval * i * VecTrialDir.y;
				// 根据衰减度得到混合系数
				//PtOffset.w = 1.0f - (m_GlareData.m_pLineData[iTrailNo].fAttenuation - 1.0f) * (float)i / (float)iSampleNum;							//第一个元素为1，最后一个元素为fAttenuation，中间的元素插值
				PtOffset.w = m_GlareData.m_pLineData[iTrailNo].fAttenuation;
				pPS->SetConstant(10 + i, PtOffset, 1);
			}

			// 设置拖尾颜色
			D3DXVECTOR4 VecColor;
			D3DCOLOR SrcColor = m_GlareData.m_pLineData[iTrailNo].Color;
			VecColor.w = 0.0f;		// 叠加中不能影响Alpha值
			VecColor.x = ((SrcColor >> 16) & 0xff) / 255.0f;	// r
			VecColor.y = ((SrcColor >> 8) & 0xff) / 255.0f;	// g
			VecColor.z = (SrcColor & 0xff) / 255.0f;			// b
			pPS->SetConstant(30, &VecColor, 1);


			// 绘制
			if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
				return E_FAIL;
		}
	}



	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	return S_OK;
}





HRESULT KPostProcess::AddGlare()
{
	if(m_iCreateAttrib!=4 || !m_pTexture || !m_pDepth)
		return E_FAIL;

	// 设置RT和指针
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSMultiTextureBlend;
	PIXELSHADER *pPS = &m_PSAddGlare;
	LPDIRECT3DTEXTURE9 pRT = m_pTexture;	// 叠加到当前计算好的BrightPass图中
	D3DSURFACE_DESC Desc;


	// 设置公用参数
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pDepth)))
		return E_FAIL;

	if(FAILED(d3ddevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(MYIMAGEVERTEXTYPE))))
		return E_FAIL;

	if(FAILED(pVS->SetTransform(NULL)))
		return E_FAIL;
	if(FAILED(pPS->SetPixelShader()))
		return E_FAIL;


	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	// 拖尾数量
	UINT iTrailNum = m_GlareData.m_iLineNum;

	// 设置来源贴图及参数
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
		// 设置纹理坐标偏移
		m_ppGlareRT[i]->GetLevelDesc(0, &Desc);
		D3DXVECTOR4 TexCoordBias = D3DXVECTOR4(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
		pVS->SetConstant(100+i, &TexCoordBias, 1);

		// 设置权重，跳过不存在的拖尾层（为0），叠加时就不会产生影响了
		float fCoef = i < iTrailNum ? 1.0f : 0.0f;
		pPS->SetConstant(i, &D3DXVECTOR4(fCoef, fCoef, fCoef, fCoef), 1);
	}


	// 设置RT
	pRT->GetSurfaceLevel(0, &pSurf);
	if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);


	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;

	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	return S_OK;
}













HRESULT KPostProcess::DOFShader(LPDIRECT3DTEXTURE9 pTexSource, LPDIRECT3DTEXTURE9 pTexBlurred, float fFocalPlane, float fNearPlane, float fFarPlane, float fBlurPower)
{
	if(m_iCreateAttrib!=4 || !pTexBlurred || !pTexSource || !m_pRT || !m_pDepth)
		return E_FAIL;

	// 设置RT和指针
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
	
	// 待处理的Image贴图和贴图大小，0、1和2层分别为Depth、原图和模糊图
	if(FAILED(d3ddevice->SetTexture(0, m_pSourceSceneRT)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(1, pTexSource)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(2, pTexBlurred)))
		return E_FAIL;
	

	// 设置Circle Of Confusion系数到常量寄存器c0～c11
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

	// 增强模糊效果
	for(int i=0; i<12; i++)
		fCOFCoef[i] *= fBlurPower;
	pPS->SetConstant(0, fCOFCoef, 12);

	// 设置纹理坐标偏移到常量寄存器
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

	// 设置3个Plane
	pPS->SetConstant(12, &D3DXVECTOR4(fFocalPlane, fNearPlane, fFarPlane, 1), 1);
	
	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
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
	
	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	
	return S_OK;
}



HRESULT KPostProcess::CalcAdaptShader(float fFrameTime, float fIdleTime)
{
	// 不检测了，只有CalcAdapt会调用，检测都在它里面进行了

	// 设置RT和指针
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
	
	// 待处理的Image贴图和贴图大小，0和1层分别为Prev和Global-Lum
	if(FAILED(d3ddevice->SetTexture(0, m_pLumAdaptPrevFrameRT)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(1, m_ppTexDownTo1[3])))
		return E_FAIL;

	// 都是1x1的，直接在ps中定义常量纹理坐标即可，不用设置纹理坐标偏移到常量寄存器了
	pPS->SetConstant(31, &D3DXVECTOR4(fFrameTime, fFrameTime, fFrameTime, fFrameTime), 1);
	// PS的c30设置为当前Idle时间（秒），用于在刚静止的N秒内不进行适应计算（N在shader中定义），如果正在移动或刚静止（为0），那么置为最大，免得移动的时候不进行适应运算
	if(fIdleTime == 0)
		fIdleTime = 32768;
	pPS->SetConstant(30, &D3DXVECTOR4(fIdleTime, fIdleTime, fIdleTime, fIdleTime), 1);


	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
	
	// 内部使用，而且不牵扯m_pTex/RT
	return S_OK;
}





HRESULT KPostProcess::EdgeDetectionShader(POSTPROCESSING_SOURCETYPE ProcRTType, float fThreshold, bool bEdgeColorWhite)
{
	if(m_bCalcEdge)
		return S_OK;
	if(m_iCreateAttrib!=4 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;

	// 根据类型选择Shader
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


	// 设置RT
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

	// 待处理的Image贴图和贴图大小
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;

	// 设置纹理坐标偏移到VS和PS的c100和c30
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

	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0xffffffff);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;

	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);


	// 这是边缘检测，不要交换指针
	return S_OK;
}




HRESULT KPostProcess::EdgeEnhanced(bool bIncr /* = true */, bool bEdgeColorWhite /* = true */)
{
	if(m_iCreateAttrib!=4 || !m_bCalcEdge || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;
	
	// 根据类型选择Shader
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = NULL;
	if(bIncr)
		pPS = bEdgeColorWhite ? &m_PSEdgeIncrWhite : &m_PSEdgeIncrBlack;
	else
		pPS = bEdgeColorWhite ? &m_PSEdgeDecrWhite : &m_PSEdgeDecrBlack;
		
	
	// 设置RT，这时边缘图已经到了pTex，直接由pTex处理到pRT即可
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
	
	// 待处理的Image贴图和贴图大小
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;
	
	// 设置纹理坐标偏移到VS和PS的c100和c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
 	
	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	// BorderColor总是非边缘色
	DWORD dwBorderColor = bEdgeColorWhite ? 0 : 0xffffffff;
	d3ddevice->SetSamplerState(0, D3DSAMP_BORDERCOLOR, dwBorderColor);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	
	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	

	// 相同size的，交换指针
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

	// 设置RT
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

	// 待处理的Image贴图和贴图大小
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;

	// 设置纹理坐标偏移到VS和PS的c100和c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);

	SetGaussian1D(fRadius);


	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;

	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);


	// 这是同Size的，交换m_pTex/RT的指针，depth不用交换（因为同size嘛）
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

	// 设置RT
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

	// 待处理的Image贴图和贴图大小
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;

	// 设置纹理坐标偏移到VS和PS的c100和c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);

	SetGaussian5x5(fRadius);


	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;

	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	// 这是同Size的，交换m_pTex/RT的指针，depth不用交换（因为同size嘛）
	LPDIRECT3DTEXTURE9 pTemp;
	pTemp = m_pTexture;
	m_pTexture = m_pRT;
	m_pRT = pTemp;
	return S_OK;
}
















/*******************************高级内部功能接口*********************************/

HRESULT KPostProcess::MeasureLuminanceShader()
{
	if(!m_bHDR)
		return D3DERR_INVALIDCALL;

	if(m_iCreateAttrib!=4 || !m_p256x256SceneRT || !m_pFullSceneDepth || !m_ppTexDownTo1[0] || !m_ppTexDownTo1[1] || !m_ppTexDownTo1[2] || !m_ppTexDownTo1[3])
		return E_FAIL;
	
	if(m_bMeasureLuminance)
		return S_OK;


	// 得到原RT（临时），渲染完恢复，这样就不至于影响刚DS渲染好的RT
	LPDIRECT3DSURFACE9 pOldRT;
	if(FAILED(d3ddevice->GetRenderTarget(0, &pOldRT)))
		return E_FAIL;	
	
	// 设置指针
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = &m_PSLumLogFirst;

	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pFullSceneDepth)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);

	for(int i=0; i<4; i++)
	{
		// 每个循环，先设置RT和Depth，第一个是从256x256-->LogSum DownTo1[0]
		m_ppTexDownTo1[i]->GetSurfaceLevel(0, &pSurf);
		if(FAILED(d3ddevice->SetRenderTarget(0, pSurf)))
			return E_FAIL;
		SAFE_RELEASE(pSurf);
		
		// 待处理的Image贴图和贴图大小
		D3DSURFACE_DESC DescTex;
		if(i==0)
		{
			// 如果是第一次渲染，那么来源应该是256x256，后面的来源都是DownTo1的上一个
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
		
		// 设置纹理坐标偏移到VS和PS的c100和c30
		m_ppTexDownTo1[i]->GetLevelDesc(0, &DescTex);
		D3DXVECTOR4 TexCoordBias = D3DXVECTOR4(1/(float)DescTex.Width, 1/(float)DescTex.Height, 0.5f/(float)DescTex.Width, 0.5f/(float)DescTex.Height);
		pVS->SetConstant(100, &TexCoordBias, 1);
		pPS->SetConstant(30, &TexCoordBias, 1);
		
		// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
		d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		
		// 绘制
		if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
			return E_FAIL;
		
		// 渲染完毕，恢复
		pPS->RestorePixelShader();
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	}


	// 恢复RT
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
	
	// 得到原RT（临时），渲染完恢复，这样就不至于影响刚DS渲染好的RT
	LPDIRECT3DSURFACE9 pOldRT;
	if(FAILED(d3ddevice->GetRenderTarget(0, &pOldRT)))
		return E_FAIL;	
	
	// 设置指针
	LPDIRECT3DSURFACE9 pSurf = NULL;
	VERTEXSHADER *pVS = &m_VSDrawQuad;
	PIXELSHADER *pPS = &m_PSLumLogFirstWithoutDS4x4;
	
	
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pFullSceneDepth)))
		return E_FAIL;
	SAFE_RELEASE(pSurf);

	for(int i=-1; i<4; i++)
	{
		// 每个循环，先设置RT和Depth，第一个是从256x256Scene-->Log 256x256Temp
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
		
		// 待处理的Image贴图和贴图大小
		D3DSURFACE_DESC DescTex;
		if(i==-1)
		{
			// 如果是第一次渲染，那么来源应该是256x256Scene，临时log到256x256Temp上
			if(FAILED(d3ddevice->SetTexture(0, m_p256x256SceneRT)))
				return E_FAIL;
			pPS = &m_PSLumLogFirstWithoutDS4x4;
		}
		else if(i==0)
		{
			// 如果是第二次渲染，那么来源应该是256x256Temp，后面的来源都是DownTo1的上一个
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
		
		// 设置纹理坐标偏移到VS和PS的c100和c30，对于第一次渲染，是同size且只采样一次的，所以用不到该数值
		m_ppTexDownTo1[i]->GetLevelDesc(0, &DescTex);
		D3DXVECTOR4 TexCoordBias = D3DXVECTOR4(1/(float)DescTex.Width, 1/(float)DescTex.Height, 0.5f/(float)DescTex.Width, 0.5f/(float)DescTex.Height);
		pVS->SetConstant(100, &TexCoordBias, 1);
		pPS->SetConstant(30, &TexCoordBias, 1);
		
		// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
		d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		
		// 绘制
		if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
			return E_FAIL;
		
		// 渲染完毕，恢复
		pPS->RestorePixelShader();
		d3ddevice->SetVertexShader(NULL);
		d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	}
	
	
	// 恢复RT
	if(FAILED(d3ddevice->SetRenderTarget(0, pOldRT)))
		return E_FAIL;
	SAFE_RELEASE(pOldRT);
	
	
	// 这是内部操作，不会使用m_pTex/RT指针，所以不用交换
	m_bMeasureLuminance = true;
	return S_OK;
}





HRESULT KPostProcess::BrightPass(float fKeyValue, float fClampValue, float fOffset)
{
	if(m_iCreateAttrib!= 4 || !m_pScaleSceneRT)
		return E_FAIL;
	if(m_bHDR && (!m_bMeasureLuminance || !m_pLumAdaptCurrentFrameRT) )
		return E_FAIL;
	
	// 设置RT和指针
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
	
	// 待处理的Image贴图和贴图大小
	if(FAILED(d3ddevice->SetTexture(0, m_pScaleSceneRT)))
		return E_FAIL;
	if(m_bHDR)
		if(FAILED(d3ddevice->SetTexture(1, m_pLumAdaptCurrentFrameRT)))
			return E_FAIL;
	
	// 设置纹理坐标偏移到VS和PS的c100和c30
	D3DSURFACE_DESC Desc;
	m_pScaleSceneRT->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	// 设置BrightPass的参数，27～29（LDR只有27和28）
	pPS->SetConstant(27, &D3DXVECTOR4(fKeyValue, fKeyValue, fKeyValue, 1), 1);
	pPS->SetConstant(28, &D3DXVECTOR4(fClampValue, fClampValue, fClampValue, 0), 1);
	pPS->SetConstant(29, &D3DXVECTOR4(fOffset, fOffset, fOffset, 0), 1);
	
	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
	// 手动重置m_pTex/RT的指针
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
	
	// 设置RT和指针
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
	
	// 待处理的Image贴图和贴图大小
	if(FAILED(d3ddevice->SetTexture(0, m_pSourceSceneRT)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(1, m_pLumAdaptCurrentFrameRT)))
		return E_FAIL;
	
	// 设置纹理坐标偏移到VS和PS的c100和c30
	D3DSURFACE_DESC Desc;
	m_pSourceSceneRT->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
		// 设置KeyValue
	pPS->SetConstant(31, &D3DXVECTOR4(fKeyValue, fKeyValue, fKeyValue, 1), 1);

	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
	
	// 不能把指针变为ToneMap的，这样就指向不同大小的纹理了，只要调用过这个函数，EndProcess会自动设置ToneMap而非SourceScene
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

	// 如果是第一次运行，不适应，只初始化，即复制为第一帧的HDR全局亮度
	if(!s_bFirstRun)
	{
		// 拷贝计算结果到m_pLumAdaptCurrentFrameRT上，便于非HDR场景使用，如果是HDR，那么还会在计算适应的时候自动覆盖m_pLumAdaptCurrentFrameRT一次
		if(FAILED(m_ppTexDownTo1[3]->GetSurfaceLevel(0, &pSrcSurf)))
			return E_FAIL;
		
		// 同大小纹理复制，选POINT过滤，复制到Cur和Prev
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
		// Prev = Cur，再利用Prev计算到新Cur上。先交换Prev和Cur，再计算新的Cur
		LPDIRECT3DTEXTURE9 pTexTemp = NULL;
		pTexTemp = m_pLumAdaptPrevFrameRT;
		m_pLumAdaptPrevFrameRT = m_pLumAdaptCurrentFrameRT;
		m_pLumAdaptCurrentFrameRT = pTexTemp;

		// 根据时间计算适应
		if(FAILED(CalcAdaptShader(CameraChange.GetFrameTime(), CameraChange.GetIdleTime())))
			return E_FAIL;
	}

	m_bCalcAdapt = true;
	return S_OK;
}








/**************************基础内部功能接口********************************/
HRESULT KPostProcess::DownScale2x2()
{
	if(m_iCreateAttrib<2 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;

	// 设置RT和指针
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
	
	// 待处理的Image贴图和贴图大小
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;

	// 设置纹理坐标偏移到VS和PS的c100和c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	
	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
		
	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	

	// 记住，这是不同Size的，而且初始化的时候会用，所以不能交换m_pTex/RT的指针
	return S_OK;
}

HRESULT KPostProcess::DownScale4x4()
{
	if(m_iCreateAttrib<2 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;
	
	// 设置RT和指针
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
	
	// 待处理的Image贴图和贴图大小
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;
	
	// 设置纹理坐标偏移到VS和PS的c100和c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	
	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
		
	// 记住，这是不同Size的，而且初始化的时候会用，所以不能交换m_pTex/RT的指针
	return S_OK;
}



HRESULT KPostProcess::BilinearResize(bool bUseOldRT /* = false */)
{
	if(m_iCreateAttrib<2 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;
	if(bUseOldRT && !m_pOldRT)
		return D3DERR_INVALIDCALL;
	
	// 设置RT和指针
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
	
	
	// 改变专用顶点缓冲，让纹理坐标等于整数，用于VSBilinear
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
	
	// 待处理的Image贴图和贴图大小
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;
	
	// 设置纹理坐标偏移到PS的c30
	if(FAILED(m_pTexture->GetLevelDesc(0, &Desc)))
		return E_FAIL;
	D3DXVECTOR4 TexCoordBias( 1/((float)Desc.Width), 1/((float)Desc.Height), (float)Desc.Width, (float)Desc.Height );
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
	
	// 记住，这是不同Size的，而且Bloom的时候会用，所以不能交换m_pTex/RT的指针
	return S_OK;
}


HRESULT KPostProcess::CopyResize(bool bUseOldRT /* = false */)
{
	if(m_iCreateAttrib<2 || !m_pTexture || !m_pRT || !m_pDepth)
		return E_FAIL;

	// 设置RT和指针
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
	
	// 待处理的Image贴图和贴图大小
	if(FAILED(d3ddevice->SetTexture(0, m_pTexture)))
		return E_FAIL;
	
	// 设置纹理坐标偏移到VS和PS的c100和c30
	D3DSURFACE_DESC Desc;
	m_pTexture->GetLevelDesc(0, &Desc);
	D3DXVECTOR4 TexCoordBias(1/(float)Desc.Width, 1/(float)Desc.Height, 0.5f/(float)Desc.Width, 0.5f/(float)Desc.Height);
	pVS->SetConstant(100, &TexCoordBias, 1);
	pPS->SetConstant(30, &TexCoordBias, 1);
	
	
	// 设置不同Filter所需的常数寄存器、渲染状态、纹理层状态和其他贴图
	d3ddevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	// 绘制
	if(FAILED(pVS->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2)))
		return E_FAIL;
	
	// 渲染完毕，恢复
	pPS->RestorePixelShader();
	d3ddevice->SetVertexShader(NULL);
	d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	
	
	// 记住，这是不同Size的，而且初始化的时候会用，所以不能交换m_pTex/RT的指针
	return S_OK;
}












HRESULT KPostProcess::CopyTexture(LPDIRECT3DTEXTURE9 pTexSrc, LPDIRECT3DTEXTURE9 pTexDst)
{
	if(!pTexSrc || !pTexDst)
		return E_FAIL;

	LPDIRECT3DSURFACE9 pSurfSrc = NULL, pSurfDst = NULL;

	if(FAILED(pTexSrc->GetSurfaceLevel(0, &pSurfSrc)))
		return E_FAIL;
	
	// 同大小纹理复制，选POINT过滤
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


// 1D Guassian -8～8，H或V由使用哪个shader来决定
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
	// 别忘了加上0的值
	float fZero = GetGaussianWeight(0, 0, fRadius);
	fSum += fZero;
	
	GaussWeight[2] = D3DXVECTOR4(fZero, fZero, fZero, fZero);
	fSum = 1 / fSum * 1.3f;
	GaussFix = D3DXVECTOR4(fSum, fSum, fSum, 1);

	m_PSGaussianH.SetConstant(10, &GaussWeight, 3);
	m_PSGaussianH.SetConstant(31, &GaussFix, 1);
}


// Gaussian 5x5，其他参数一概没有
void KPostProcess::SetGaussian5x5(float fRadius)
{
	D3DXVECTOR4 GaussWeight[4];
	D3DXVECTOR4 GaussFix;
	
	float fSum = 0;
	float *pVec4 = (float *)GaussWeight;

	// 计算13个分量
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
	
	// 计算(0,0)
	float fZero = GetGaussianWeight(0, 0, fRadius);
	fSum += fZero;

	// 第三个VEC4和第一个是一样的，因为在X相同、绝对值相同的情况下，正负Y的高斯函数值完全一致，为了提高效率这里直接赋值
	GaussWeight[2] = GaussWeight[0];
	GaussWeight[3] = D3DXVECTOR4(fZero, fZero, fZero, fZero);

	// 计算修正值
	fSum = 1 / fSum * 1.3f;
	GaussFix = D3DXVECTOR4(fSum, fSum, fSum, 1);
	

	m_PSGaussian5x5.SetConstant(20, &GaussWeight, 4);
	m_PSGaussian5x5.SetConstant(31, &GaussFix, 1);
}