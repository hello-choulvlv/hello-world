#include "Myd3d.h"
#include "Texture.h"
void SetMyScene(UINT);
void RenderMyScene(UINT);


HRESULT D3DXLoadTextureFromTexture(LPDIRECT3DTEXTURE9 pTexDst, LPDIRECT3DTEXTURE9 pTexSrc)
{
	if(!pTexDst ||!pTexSrc)
		return D3DERR_INVALIDCALL;
	
	LPDIRECT3DSURFACE9 pSurfSrc = NULL, pSurfDst = NULL;
	
	V_RETURN(pTexSrc->GetSurfaceLevel(0, &pSurfSrc));
	V_RETURN(pTexDst->GetSurfaceLevel(0, &pSurfDst));
	V_RETURN(D3DXLoadSurfaceFromSurface(pSurfDst, NULL, NULL, pSurfSrc, NULL, NULL, D3DX_FILTER_NONE, 0));
	
	SAFE_RELEASE(pSurfSrc);
	SAFE_RELEASE(pSurfDst);
	return S_OK;
}




// 从六个图片文件名创建一张CubeMap，文件名数组依次为“X正、负；Y正、负；Z正、负轴的贴图”
// POOL_MANAGED，仅作为俭省天空盒子的存储容量而做
HRESULT CreateCubeMapFromSixFiles(char **ppszFileName, LPDIRECT3DCUBETEXTURE9 *ppCubeTex, UINT iLength, UINT iMip/* = 0*/, D3DFORMAT Format/* = D3DFMT_A8R8G8B8*/)
{
	if(!ppszFileName || !ppCubeTex || !iLength)
		return D3DERR_INVALIDCALL;

	LPDIRECT3DTEXTURE9 pTex[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	LPDIRECT3DSURFACE9 pSurfSrc = NULL, pSurfDst = NULL;
	UINT i = 0, j = 0;

	if(iMip == 0)
		iMip = 8;

	for(i = 0; i < 6; i++)
	{
		if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, ppszFileName[i], iLength, iLength, 0, D3DUSAGE_0, Format, D3DPOOL_SYSTEMMEM, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, NULL, NULL, &pTex[i])))
			goto Failed;
	}

	// 创建CubeMap并写入
	if(FAILED(d3ddevice->CreateCubeTexture(iLength, iMip, D3DUSAGE_0, Format, D3DPOOL_MANAGED, ppCubeTex, NULL)))
		goto Failed;

	for(i = 0; i < 6; i++)
	{
		for(j = 0; j < iMip; j++)
		{
			if(FAILED((*ppCubeTex)->GetCubeMapSurface((D3DCUBEMAP_FACES)i, j, &pSurfDst)))
				goto Failed;
			if(FAILED(pTex[i]->GetSurfaceLevel(j, &pSurfSrc)))
				goto Failed;

			if(FAILED(D3DXLoadSurfaceFromSurface(pSurfDst, NULL, NULL, pSurfSrc, NULL, NULL, D3DX_FILTER_NONE, 0)))
				goto Failed;

			SAFE_RELEASE(pSurfSrc);
			SAFE_RELEASE(pSurfDst);
		}
	}


	// 结束
	for(i = 0; i < 6; i++)
		SAFE_RELEASE(pTex[i]);

	return S_OK;


Failed:
	for(i = 0; i < 6; i++)
		SAFE_RELEASE(pTex[i]);
	SAFE_RELEASE((*ppCubeTex));
	SAFE_RELEASE(pSurfSrc);
	SAFE_RELEASE(pSurfDst);

	return E_FAIL;
}




// 将贴图中像素的值log出来，支持任意2D贴图
HRESULT TextureLogOut(char *pszFileName, LPDIRECT3DTEXTURE9 pSrcTex)
{
	if(!pszFileName || !pszFileName[0] || !pSrcTex)
		return D3DERR_INVALIDCALL;

	D3DLOCKED_RECT Rect;
	float *p = NULL;
	float16 *p16 = NULL;
	D3DXVECTOR4 fValue;
	float fRe = 0.0f, fIm = 0.0f;

	LPDIRECT3DTEXTURE9 pTex = NULL;
	D3DSURFACE_DESC Desc;
	LPDIRECT3DSURFACE9 pSurf = NULL;

	V_RETURN(pSrcTex->GetLevelDesc(0, &Desc));
	// 必须是不在显存中且格式为FP32全通道的才能读取，如果不符合要求，就新建一个
	if(Desc.Format != D3DFMT_A32B32G32R32F || Desc.Pool == D3DPOOL_DEFAULT)
	{
		V_RETURN(d3ddevice->CreateTexture(Desc.Width, Desc.Height, 1, 0, Desc.Format, D3DPOOL_SYSTEMMEM, &pTex, NULL));
		V_RETURN(D3DXLoadTextureFromTexture(pTex, pSrcTex));
		V_RETURN(pTex->GetSurfaceLevel(0, &pSurf));
	}
	else
		V_RETURN(pSrcTex->GetSurfaceLevel(0, &pSurf));



	FILE *fp = fopen(pszFileName, "w");
	if(!fp)
	{
		OutputDebugString("创建文件失败！可能是目录不存在、文件名有误、磁盘写保护或磁盘空间不足！\n");
		return E_FAIL;
	}



	V_RETURN(pSurf->LockRect(&Rect, NULL, 0));

	char szLog[100];
	for(UINT iY = 0; iY < Desc.Height; iY++)
	{
		sprintf(szLog, "Y=%d:", iY);
		fwrite(szLog, strlen(szLog), 1, fp);

		p = (float *)((BYTE *)Rect.pBits + iY * Rect.Pitch);
		p16 = (float16 *)((BYTE *)Rect.pBits + iY * Rect.Pitch);
		for(UINT iX = 0; iX < Desc.Width; iX++)
		{
			// 先读取数据
			fValue[0] = *p++;
			fValue[1] = *p++;
			fValue[2] = *p++;
			fValue[3] = *p++;

			// 根据符号恢复数值
			fValue[0] *= (fValue[2]-1);
			fValue[1] *= (fValue[3]-1);

			for(UINT i = 0; i < 4; i++)
			{
				sprintf(szLog, "  %.3f,", fValue[i]);
				fwrite(szLog, strlen(szLog), 1, fp);
			}

			// 换行排版
			if(iX < Desc.Width -1)
			{
				sprintf(szLog, "\n    ");
				fwrite(szLog, strlen(szLog), 1, fp);
			}
			else
			{
				sprintf(szLog, "\n");
				fwrite(szLog, strlen(szLog), 1, fp);
			}
		}
	}
	V_RETURN(pSurf->UnlockRect());
	fclose(fp);

	SAFE_RELEASE(pSurf);
	SAFE_RELEASE(pTex);
	return S_OK;
}




/*************************渲染到纹理*******************/
RENDERTOTEXTURE::RENDERTOTEXTURE()
{
	CreateAttrib=0;
	RenderTexture=NULL;
	DepthBuffer=NULL;
}

RENDERTOTEXTURE::~RENDERTOTEXTURE()
{
	Release();
}

void RENDERTOTEXTURE::Release()
{
	if(!CreateAttrib)
		return;
	SAFE_RELEASE(RenderTexture);
	SAFE_RELEASE(DepthBuffer);
	CreateAttrib=0;
}


HRESULT RENDERTOTEXTURE::InitTexture(UINT Size)
{
	if(CreateAttrib) return E_FAIL;

	//临时使用，只是为了得到和原来一样的信息，如分辨率，采样率，点格式等
	LPDIRECT3DSURFACE9 OldTempDepthBuffer;
	D3DSURFACE_DESC Desc;
	if(FAILED(d3ddevice->GetDepthStencilSurface(&OldTempDepthBuffer)))
		return E_FAIL;
	OldTempDepthBuffer->GetDesc(&Desc);
	SAFE_RELEASE(OldTempDepthBuffer);
	
	//开始初始化
	if(FAILED(d3ddevice->CreateDepthStencilSurface(Desc.Width, Desc.Height, Desc.Format, Desc.MultiSampleType, 0, FALSE, &DepthBuffer, NULL)))
		return E_FAIL;
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, TextureFormat)))
		return E_FAIL;
	if(FAILED(d3ddevice->CreateTexture(Size, Size, 1, D3DUSAGE_RENDERTARGET, TextureFormat, D3DPOOL_DEFAULT, &RenderTexture, NULL)))
		return E_FAIL;
	CreateAttrib=1;
	return S_OK;
}

HRESULT RENDERTOTEXTURE::SetTexture(DWORD Stage)
{
	if(CreateAttrib!=2||RenderTexture==NULL) return E_FAIL;
	return d3ddevice->SetTexture(Stage, RenderTexture);
}

HRESULT RENDERTOTEXTURE::RenderToTexture(float Eyex, float Eyey, float Eyez, float Lookx, float Looky, float Lookz, float Heady, float Arg, UINT *Mask)
{
	if(CreateAttrib==0||Heady==0) return E_FAIL;
	//保存当前设置
	d3ddevice->GetDepthStencilSurface(&OldDepthBuffer);
	d3ddevice->GetRenderTarget(0, &OldBackBuffer);

	// 该纹理变成渲染对象，取消Texture状态，避免Bound
	for(int i = 0; i < 8; i++)
		d3ddevice->SetTexture(i, NULL);
	
	//获得并设置渲染面指针，清除表面，并设置其他参数
	D3DXMATRIX surfaceview, surfaceproj;
	
	V_RETURN(SetTexturedRenderTarget(0, RenderTexture, DepthBuffer));

	d3ddevice->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,D3DCOLOR_XRGB(255,255,255),1.0f,8);
	D3DXMatrixLookAtLH(&surfaceview,&D3DXVECTOR3(Eyex,Eyey,Eyez), &D3DXVECTOR3(Lookx,Looky,Lookz),&D3DXVECTOR3(0,Heady,0));
	d3ddevice->SetTransform(D3DTS_VIEW,&surfaceview);
    D3DXMatrixPerspectiveFovLH( &surfaceproj, Arg, 1.0f, 1.0f, 100.0f );
    d3ddevice->SetTransform( D3DTS_PROJECTION, &surfaceproj );

	//开始渲染
	for(UINT j=0;j<MODULENUM;j++)
	{
	//如果遇到临时不需要渲染的物体，或要贴此渲染纹理的物体，在此处设置已便跳过渲染
	if(Mask[j]==0) continue;     
	SetMyScene(j);
	RenderMyScene(j);
	}

	
	//渲染完毕，恢复
	CameraChange.ViewTransform();
	CameraChange.ProjTransform(CameraChange.m_fProjXYCoef);
	d3ddevice->SetRenderTarget(0, OldBackBuffer);
	d3ddevice->SetDepthStencilSurface(OldDepthBuffer);

	SAFE_RELEASE(OldBackBuffer);
	SAFE_RELEASE(OldDepthBuffer);
	CreateAttrib=2;
	return S_OK;
}








/*************************立方环境纹理*******************/
CUBEMAP::CUBEMAP()
{
	CreateAttrib=0;
	CubeMapStage=0;
	CubeMap=NULL;  DepthBuffer=NULL;
}

CUBEMAP::~CUBEMAP()
{
	Release();
}

void CUBEMAP::Release()
{
	if(!CreateAttrib)
		return;
	SAFE_RELEASE(DepthBuffer);
	DepthBuffer = NULL;
	SAFE_RELEASE(CubeMap);
	CubeMap = NULL;
	CreateAttrib=0;
	CubeMapStage=0;
}

HRESULT CUBEMAP::InitCubeMap(UINT Size)
{
	if(CreateAttrib) return E_FAIL;
	if((d3dcaps.TextureCaps&D3DPTEXTURECAPS_CUBEMAP)==0)
		return E_FAIL;
	//临时使用，只是为了得到和原来一样的信息，如分辨率，采样率，点格式等
	LPDIRECT3DSURFACE9 OldTempDepthBuffer;
	D3DSURFACE_DESC Desc;
	if(CreateAttrib) return E_FAIL;
	if(FAILED(d3ddevice->GetDepthStencilSurface(&OldTempDepthBuffer)))
		return E_FAIL;
	OldTempDepthBuffer->GetDesc(&Desc);
	SAFE_RELEASE(OldTempDepthBuffer);

	//开始初始化
	if(FAILED(d3ddevice->CreateDepthStencilSurface(Desc.Width, Desc.Height, Desc.Format, Desc.MultiSampleType, 0, FALSE, &DepthBuffer, NULL)))
		return E_FAIL;
	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, TextureFormat)))
		return E_FAIL;
	if(FAILED(d3ddevice->CreateCubeTexture(Size, 0, D3DUSAGE_RENDERTARGET, TextureFormat, D3DPOOL_DEFAULT, &CubeMap, NULL)))
		return E_FAIL;
	CreateAttrib=1;
	return S_OK;
}

HRESULT CUBEMAP::InitCubeMap(LPSTR Filename)
{
	if(CreateAttrib) return E_FAIL;
	if(FAILED(D3DXCreateCubeTextureFromFile(d3ddevice, Filename, &CubeMap)))
		return E_FAIL;
	CreateAttrib=2;
	return S_OK;
}

HRESULT CUBEMAP::SetTexture(DWORD Stage)
{
	if(CreateAttrib==0||CubeMap==NULL) return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(Stage, CubeMap)))
		return E_FAIL;
	CubeMapStage=Stage;
	
#ifdef USE_CUBEMAPENVIRONMENT
	// 这个用于固定管线的CubeMap环境映射，但使用VS渲染环境映射的时候会引起D3D DEBUG ERROR
	d3ddevice->SetTextureStageState(CubeMapStage,D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
	d3ddevice->SetTextureStageState(CubeMapStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
#else
	// 使用VS渲染时，应该设置为如下内容
	for(int i = Stage; i>=0; i--)
	{
		d3ddevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		d3ddevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);
	}
#endif
	
	return S_OK;
}

void CUBEMAP::RestoreSettings()
{
	d3ddevice->SetTextureStageState(CubeMapStage,D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	d3ddevice->SetTextureStageState(CubeMapStage,D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);
}

HRESULT CUBEMAP::RenderToSurface(float Eyex, float Eyey, float Eyez, UINT *Mask)
{
	if(CreateAttrib!=1) return E_FAIL;
	//先保存
	d3ddevice->GetDepthStencilSurface(&OldDepthBuffer);
	d3ddevice->GetRenderTarget(0, &OldBackBuffer);
	//开始渲染
	D3DXMATRIX cubeview, cubeproj;
	D3DXVECTOR3 lookat[6]={D3DXVECTOR3(1 + Eyex,0,0),D3DXVECTOR3(-1 + Eyex,0,0),D3DXVECTOR3(0,1 + Eyey,0),D3DXVECTOR3(0,-1 + Eyey,0),D3DXVECTOR3(0,0,1 + Eyez),D3DXVECTOR3(0,0,-1 + Eyez)};
	D3DXVECTOR3 head[6]={D3DXVECTOR3(0,1,0),D3DXVECTOR3(0,1,0),D3DXVECTOR3(0,0,-1),D3DXVECTOR3(0,0,1),D3DXVECTOR3(0,1,0),D3DXVECTOR3(0,1,0)};
	
	// 该立方纹理变成渲染对象，取消Texture状态，避免Bound
	for(int i = 0; i < 8; i++)
		d3ddevice->SetTexture(i, NULL);
	
	for(int i = 0; i < 6; i++)
	{
		V_RETURN(SetTexturedRenderTarget(0, i, CubeMap, DepthBuffer));
		d3ddevice->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,D3DCOLOR_XRGB(0,0,0),1.0f,8);
		D3DXMatrixLookAtLH(&cubeview,&D3DXVECTOR3(Eyex,Eyey,Eyez), &lookat[i],&head[i]);
		d3ddevice->SetTransform(D3DTS_VIEW,&cubeview);
	    D3DXMatrixPerspectiveFovLH( &cubeproj, D3DX_PI/2, 1.0f, 0.5f, 100.0f );
		d3ddevice->SetTransform( D3DTS_PROJECTION, &cubeproj );
		
		for(UINT j=0;j<MODULENUM;j++)
		{
		//如果遇到临时不需要渲染的物体，或要贴环境映射的物体，在此处设置已便跳过渲染
		if(Mask[j]==0) continue;     
		SetMyScene(j);
		RenderMyScene(j);
		}

	}
	//渲染完毕，恢复
	CameraChange.ViewTransform();
	CameraChange.ProjTransform(CameraChange.m_fProjXYCoef);
	d3ddevice->SetRenderTarget(0, OldBackBuffer);
	d3ddevice->SetDepthStencilSurface(OldDepthBuffer);
	SAFE_RELEASE(OldBackBuffer);
	SAFE_RELEASE(OldDepthBuffer);
	return S_OK;
}












/*************************环境映射凹凸纹理*******************/

BUMPMAP::BUMPMAP()
{
	BumpStage=8; //初始设置当前层数为非法值
	BumpMap=NULL;
	CreateAttrib=0;
}

BUMPMAP::~BUMPMAP()
{
	Release();
}

void BUMPMAP::Release()
{
	SAFE_RELEASE(BumpMap);
	BumpMap = NULL;
	BumpStage=8;
	CreateAttrib=0;
}

HRESULT BUMPMAP::GetSupport()
{
	if((d3dcaps.TextureOpCaps&(D3DTEXOPCAPS_BUMPENVMAP|D3DTEXOPCAPS_BUMPENVMAPLUMINANCE))==0) return E_FAIL;
	else return S_OK;
}

HRESULT BUMPMAP::InitFromFile(LPSTR Filename, D3DFORMAT Format)
{
	//检查是否支持凹凸环境映射，一定要先进行！因为很多显卡都不支持的
	if((d3dcaps.TextureOpCaps&(D3DTEXOPCAPS_BUMPENVMAP|D3DTEXOPCAPS_BUMPENVMAPLUMINANCE))==0)
		{CreateAttrib=-1;return E_FAIL;}

	if(CreateAttrib!=0) return E_FAIL;
	LPDIRECT3DTEXTURE9 SourceMap;   //文件图纹理
	D3DSURFACE_DESC srcdesc;  //文件信息
	D3DLOCKED_RECT srcrect, dstrect;  //锁定的数据信息
	BYTE *psrc;  //文件图每行的头指针
	BYTE *pdst,*pdstbk;  //信息图每行的头指针
    int sizeperpixel; //凹凸图每像素所占的大小

	//判断格式，只支持这三种格式
	if(Format==D3DFMT_X8L8V8U8) sizeperpixel=4;
	else if(Format==D3DFMT_L6V5U5||Format==D3DFMT_V8U8) sizeperpixel=2;
	else return E_FAIL;
	
	//初始化两张贴图，并得到指针和信息，文件贴图是A8R8G8B8的格式，方便下面采样
	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, Filename, 0, 0, 1, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &SourceMap)))
		return E_FAIL;

	SourceMap->GetLevelDesc(0, &srcdesc);
	SourceMap->LockRect(0, &srcrect, NULL, 0);
	psrc=(BYTE *)srcrect.pBits;

	if(FAILED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_0, D3DRTYPE_TEXTURE, Format)))
		return E_FAIL;

	if(FAILED(d3ddevice->CreateTexture(srcdesc.Width, srcdesc.Height, 1, D3DUSAGE_0, Format, D3DPOOL_MANAGED, &BumpMap, NULL)))
		return E_FAIL;
	BumpMap->LockRect(0, &dstrect, NULL, 0);
	pdstbk=(BYTE *)dstrect.pBits;


	//开始进行转换
	BYTE *pup, *pdown, *pleft, *pright, *pcur; //几个采样指针分别对临近像素进行计算
	DWORD left,right,cur;
	BYTE du, dv;  //坐标信息
	BYTE L;  //亮度

	for(UINT y=0; y<srcdesc.Height; y++)
	{
		//定位行头
		pdst=pdstbk;
		pcur=psrc;
		pup=pcur-srcrect.Pitch;
		pdown=pcur+srcrect.Pitch;
		//限制Y不能到第一行之前和最后一行之后
		if(y==0) pup=pcur;
		if(y==srcdesc.Height-1) pdown=pcur;

		for(UINT x=0; x<srcdesc.Width; x++)
		{
			pleft=pcur-4;
			pright=pcur+4;
			left=*pleft;
			right=*pright;
			cur=*pcur;
			//判断处于波谷
			if((left>cur)&&(right>cur))
			{
				cur=left-cur;
				if(cur<(right-cur))
					cur=right-cur;
			}
			//采样，转换数据，因为RGB都相等，所以随便取一个值就可以了，不过注意跳过A
			dv=*(pup+1)-*(pdown+1);
			du=*(pleft+1)-*(pright+1);
			L=(cur>1) ? 63 : 127;
			//根据格式写入数据
			switch(Format)
			{
			case D3DFMT_V8U8:
				*pdst++=du;
				*pdst++=dv;
				break;
			case D3DFMT_L6V5U5:
				*pdst++=( (du&31) | (dv<<5) );
				*pdst++=( ((dv<<3)>>6) | L<<2);
				break;
			case D3DFMT_X8L8V8U8:
				*pdst++=du;
				*pdst++=dv;
				*pdst++=(BYTE)L;
				*pdst++=(BYTE)0;
				break;
			}// end Switch*/
		pup+=4; pcur+=4; pdown+=4;
		}//end for x
		pdstbk+=dstrect.Pitch;  psrc+=srcrect.Pitch;
	}//end for y

	//初始化结束释放文件图
	SourceMap->UnlockRect(0);
	SAFE_RELEASE(SourceMap);
	BumpMap->UnlockRect(0);
	CreateAttrib=1;
	return S_OK;
}

void BUMPMAP::Save()
{
	//保存TSS设置(坐标索引和色彩混合)
	if(BumpStage)
	{
		d3ddevice->GetTextureStageState(BumpStage-1, D3DTSS_COLOROP, &ColorOP_1);
		d3ddevice->GetTextureStageState(BumpStage-1, D3DTSS_COLORARG1, &ColorArg1_1);
		d3ddevice->GetTextureStageState(BumpStage-1, D3DTSS_COLORARG2, &ColorArg2_1);
		d3ddevice->GetTextureStageState(BumpStage-1, D3DTSS_TEXCOORDINDEX, &TexIndex_1);
	}
	d3ddevice->GetTextureStageState(BumpStage, D3DTSS_COLOROP, &ColorOP_2);
	d3ddevice->GetTextureStageState(BumpStage, D3DTSS_COLORARG1, &ColorArg1_2);
	d3ddevice->GetTextureStageState(BumpStage, D3DTSS_COLORARG2, &ColorArg2_2);
	d3ddevice->GetTextureStageState(BumpStage-1, D3DTSS_TEXCOORDINDEX, &TexIndex_2);
	d3ddevice->GetTextureStageState(BumpStage+1, D3DTSS_COLOROP, &ColorOP_3);
	d3ddevice->GetTextureStageState(BumpStage+1, D3DTSS_COLORARG1, &ColorArg1_3);
	d3ddevice->GetTextureStageState(BumpStage+1, D3DTSS_COLORARG2, &ColorArg2_3);
	d3ddevice->GetTextureStageState(BumpStage-1, D3DTSS_TEXCOORDINDEX, &TexIndex_3);
}

HRESULT BUMPMAP::SetTexture(DWORD Stage, float M00, float M01, float M10, float M11, float O, float S)
{
	if(CreateAttrib!=1||Stage<0||Stage>6||BumpMap==NULL) return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(Stage, BumpMap)))
		return E_FAIL;
	BumpStage=Stage;

	Save();
	//设置凹凸参数
	d3ddevice->SetTextureStageState(Stage, D3DTSS_BUMPENVMAT00, ftodw(M00));
	d3ddevice->SetTextureStageState(Stage, D3DTSS_BUMPENVMAT01, ftodw(M01));
	d3ddevice->SetTextureStageState(Stage, D3DTSS_BUMPENVMAT10, ftodw(M10));
	d3ddevice->SetTextureStageState(Stage, D3DTSS_BUMPENVMAT11, ftodw(M11));
	d3ddevice->SetTextureStageState(Stage, D3DTSS_BUMPENVLOFFSET, ftodw(O));
	d3ddevice->SetTextureStageState(Stage, D3DTSS_BUMPENVLSCALE, ftodw(S));
	
	//设置纹理层混合
	if(Stage)       //如果有底层纹理，就设置
	{
		d3ddevice->SetTextureStageState(Stage-1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		d3ddevice->SetTextureStageState(Stage-1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		d3ddevice->SetTextureStageState(Stage-1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	}
	d3ddevice->SetTextureStageState(Stage, D3DTSS_COLOROP, D3DTOP_BUMPENVMAPLUMINANCE);
	d3ddevice->SetTextureStageState(Stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3ddevice->SetTextureStageState(Stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
	d3ddevice->SetTextureStageState(Stage+1, D3DTSS_COLOROP, D3DTOP_ADD);
	d3ddevice->SetTextureStageState(Stage+1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3ddevice->SetTextureStageState(Stage+1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	return S_OK;
}

HRESULT BUMPMAP::SetTexture(DWORD Stage, float M00, float M01, float M10, float M11)
{
	if(CreateAttrib!=1||Stage<0||Stage>6||BumpMap==NULL) return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(Stage, BumpMap)))
		return E_FAIL;
	
	BumpStage=Stage;

	Save();
	//设置凹凸参数
	d3ddevice->SetTextureStageState(Stage, D3DTSS_BUMPENVMAT00, ftodw(M00));
	d3ddevice->SetTextureStageState(Stage, D3DTSS_BUMPENVMAT01, ftodw(M01));
	d3ddevice->SetTextureStageState(Stage, D3DTSS_BUMPENVMAT10, ftodw(M10));
	d3ddevice->SetTextureStageState(Stage, D3DTSS_BUMPENVMAT11, ftodw(M11));
	
	//设置纹理层混合
	if(Stage)       //如果有底层纹理，就设置
	{
		d3ddevice->SetTextureStageState(Stage-1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		d3ddevice->SetTextureStageState(Stage-1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		d3ddevice->SetTextureStageState(Stage-1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	}
	d3ddevice->SetTextureStageState(Stage, D3DTSS_COLOROP, D3DTOP_BUMPENVMAP);
	d3ddevice->SetTextureStageState(Stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3ddevice->SetTextureStageState(Stage, D3DTSS_COLORARG2, D3DTA_CURRENT);
	d3ddevice->SetTextureStageState(Stage+1, D3DTSS_COLOROP, D3DTOP_ADD);
	d3ddevice->SetTextureStageState(Stage+1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3ddevice->SetTextureStageState(Stage+1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	
	return S_OK;
}


void BUMPMAP::RestoreSettings()
{
	if(CreateAttrib!=1) return;
	if(BumpStage)
	{
		d3ddevice->SetTextureStageState(BumpStage-1, D3DTSS_COLOROP, ColorOP_1);
		d3ddevice->SetTextureStageState(BumpStage-1, D3DTSS_COLORARG1, ColorArg1_1);
		d3ddevice->SetTextureStageState(BumpStage-1, D3DTSS_COLORARG2, ColorArg2_1);
		d3ddevice->SetTextureStageState(BumpStage-1, D3DTSS_TEXCOORDINDEX, TexIndex_1);
	}
	d3ddevice->SetTextureStageState(BumpStage, D3DTSS_COLOROP, ColorOP_2);
	d3ddevice->SetTextureStageState(BumpStage, D3DTSS_COLORARG1, ColorArg1_2);
	d3ddevice->SetTextureStageState(BumpStage, D3DTSS_COLORARG2, ColorArg2_2);
	d3ddevice->SetTextureStageState(BumpStage-1, D3DTSS_TEXCOORDINDEX, TexIndex_2);
	d3ddevice->SetTextureStageState(BumpStage+1, D3DTSS_COLOROP, ColorOP_3);
	d3ddevice->SetTextureStageState(BumpStage+1, D3DTSS_COLORARG1, ColorArg1_3);
	d3ddevice->SetTextureStageState(BumpStage+1, D3DTSS_COLORARG2, ColorArg2_3);
	d3ddevice->SetTextureStageState(BumpStage-1, D3DTSS_TEXCOORDINDEX, TexIndex_3);
}







/*************************法向量纹理*******************/
NORMALMAP::NORMALMAP()
{
	NormalStage=8; //初始设置当前层数为非法值
	NormalMap=NULL;
	CreateNormalAttrib=0;
	CreatePowerAttrib=0;
	CreateAnisotropyAttrib=0;
	CreateAnisotropyDirAttrib=0;
}

NORMALMAP::~NORMALMAP()
{
	Release();
}

void NORMALMAP::Release()
{
	SAFE_RELEASE(NormalMap);
	SAFE_RELEASE(PowerMap);
	SAFE_RELEASE(AnisotropyMap);
	SAFE_RELEASE(AnisotropyDirMap);
	NormalStage=8;
	CreateNormalAttrib=0;
	CreatePowerAttrib=0;
	CreateAnisotropyAttrib=0;
	CreateAnisotropyDirAttrib=0;
}


HRESULT NORMALMAP::SetNormalMap(DWORD Stage)
{
	if(CreateNormalAttrib==0||NormalMap==NULL) return E_FAIL;
	return d3ddevice->SetTexture(Stage, NormalMap);
}

HRESULT NORMALMAP::SetPowerMap(DWORD Stage)
{
	if(CreatePowerAttrib==0||PowerMap==NULL) return E_FAIL;
	return d3ddevice->SetTexture(Stage, PowerMap);
}

HRESULT NORMALMAP::SetAnisotropyMap(DWORD Stage)
{
	if(CreateAnisotropyAttrib==0||AnisotropyMap==NULL) return E_FAIL;
	return d3ddevice->SetTexture(Stage, AnisotropyMap);
}

HRESULT NORMALMAP::SetAnisotropyDirMap(DWORD Stage)
{
	if(CreateAnisotropyDirAttrib==0||AnisotropyDirMap==NULL) return E_FAIL;
	return d3ddevice->SetTexture(Stage, AnisotropyDirMap);
}


HRESULT NORMALMAP::SetTexture(DWORD Stage, float Lx, float Ly, float Lz)
{
	if(CreateNormalAttrib==0||NormalMap==NULL) return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(Stage, NormalMap)))
		return E_FAIL;
	NormalStage=Stage;
	
	//保存TSS色彩混合设置
	d3ddevice->GetTextureStageState(NormalStage, D3DTSS_COLOROP, &ColorOP);
	d3ddevice->GetTextureStageState(NormalStage, D3DTSS_COLORARG1, &ColorArg1);
	d3ddevice->GetTextureStageState(NormalStage, D3DTSS_COLORARG2, &ColorArg2);
	//设置光线方向
	d3ddevice->SetRenderState(D3DRS_TEXTUREFACTOR, VectoRGBSigned(Lx, Ly, Lz, 1));
	//设置纹理层混合
	d3ddevice->SetTextureStageState(Stage, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3);
	d3ddevice->SetTextureStageState(Stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3ddevice->SetTextureStageState(Stage, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	
	return S_OK;
}


void NORMALMAP::RestoreSettings()
{
	if(CreateNormalAttrib==0) return;
	d3ddevice->SetTextureStageState(NormalStage, D3DTSS_COLOROP, ColorOP);
	d3ddevice->SetTextureStageState(NormalStage, D3DTSS_COLORARG1, ColorArg1);
	d3ddevice->SetTextureStageState(NormalStage, D3DTSS_COLORARG2, ColorArg2);
}



HRESULT NORMALMAP::InitFromFile(LPSTR Filename, double Scale)
{
	if(CreateNormalAttrib) return E_FAIL;
	
	LPDIRECT3DTEXTURE9 source;
	D3DSURFACE_DESC srcdesc;  //文件信息
	D3DLOCKED_RECT srcrect, dstrect;  //锁定的数据信息
	BYTE *psrc;  //文件图每行的头指针
	BYTE *pdst,*pdstbk;  //信息图每行的头指针
	D3DXVECTOR3 N;   //法向量

	if(FAILED(D3DXCreateTextureFromFileEx(d3ddevice, Filename, 0, 0, 1, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &source)))
		return E_FAIL;
	source->GetLevelDesc(0, &srcdesc);
	source->LockRect(0, &srcrect, NULL, 0);
	psrc=(BYTE *)srcrect.pBits;


	if(FAILED(D3DXCreateTexture(d3ddevice, srcdesc.Width, srcdesc.Height, 0, 0, D3DFMT_A8R8G8B8,D3DPOOL_MANAGED, &NormalMap)))
		return E_FAIL;
	NormalMap->LockRect(0, &dstrect, NULL, 0);
	pdstbk=(BYTE *)dstrect.pBits;


	//开始进行转换
	BYTE *pup, *pdown, *pright, *pleft, *pupleft, *pupright, *pdownleft, *pdownright, *pcur; //几个采样指针分别对临近像素进行计算
	BYTE down,up,left,right,upleft,upright,downleft,downright;
	double t1, t2, mod;
	DWORD *cur;

	for(UINT y=0; y<srcdesc.Height; y++)
	{
		//定位行头
		pdst=pdstbk;
		pcur=psrc;
		pup=pcur-srcrect.Pitch;
		pdown=pcur+srcrect.Pitch;
		
		//限制Y不能到第一行之前和最后一行之后
		if(y==0) pup=pcur;
		if(y==srcdesc.Height-1) pdown=pcur;

		for(UINT x=0; x<srcdesc.Width; x++)
		{
			pleft=pcur-4;
			pright=pcur+4;
			pupleft=pup-4;
			pupright=pup+4;
			pdownleft=pdown-4;
			pdownright=pdown+4;
			if(x==0) {pleft=pcur;pupleft=pup;pdownleft=pdown;}
			if(x==srcdesc.Width-1) {pright=pcur;pupright=pup;pdownright=pdown;}

			up=*(pup+1);
			down=*(pdown+1);
			left=*(pleft+1);
			right=*(pright+1);
			upleft=*(pupleft+1);
			upright=*(pupright+1);
			downleft=*(pdownleft+1);
			downright=*(pdownright+1);

			t1=(double)upleft+(double)left+(double)downleft-(double)upright-(double)right-(double)downright;
			t2=(double)upleft+(double)up+(double)upright-(double)downleft-(double)down-(double)downright;
			mod=1/(double)255; mod*=0.1;
			N=D3DXVECTOR3((float)(t1*Scale*mod), (float)(t2*Scale*mod), 1);

			cur=(DWORD *)pdst;
			*cur=VectoRGBSigned(N.x, N.y, N.z, 1);

		pup+=4; pcur+=4; pdown+=4; pdst+=4;
		}//end for x
		pdstbk+=dstrect.Pitch;  psrc+=srcrect.Pitch;
	}//end for y

	//初始化结束释放文件图
	source->UnlockRect(0);
	SAFE_RELEASE(source);
	NormalMap->UnlockRect(0);
	CreateNormalAttrib=1;
	return S_OK;
}



HRESULT NORMALMAP::InitFromFileDX(LPSTR Filename, float Scale)
{
	if(CreateNormalAttrib) return E_FAIL;

	LPDIRECT3DTEXTURE9 source;
	D3DSURFACE_DESC desc;
	if(FAILED(D3DXCreateTextureFromFile(d3ddevice, Filename, &source)))
		return E_FAIL;
	source->GetLevelDesc(0, &desc);

	if(FAILED(D3DXCreateTexture(d3ddevice, desc.Width, desc.Height, 0, 0, D3DFMT_A8R8G8B8,D3DPOOL_MANAGED, &NormalMap)))
		return E_FAIL;
	if(FAILED(D3DXComputeNormalMap(NormalMap, source, NULL, 0, D3DX_CHANNEL_RED, Scale)))
		return E_FAIL;

	SAFE_RELEASE(source);
	CreateNormalAttrib=1;
	return S_OK;
}


HRESULT NORMALMAP::InitPowerMap(UINT Size, double Power)
{
	if(CreatePowerAttrib) return E_FAIL;
	D3DLOCKED_RECT dstrect;  //锁定的数据信息
	DWORD *pdst,*pdstbk;  //信息图每行的头指针

	if(FAILED(d3ddevice->CreateTexture(Size, Size, 1, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &PowerMap, NULL)))
		return E_FAIL;
	PowerMap->LockRect(0, &dstrect, NULL, 0);
	pdstbk=(DWORD *)dstrect.pBits;


	//开始进行转换
	float z;

	for(UINT y=0; y<Size; y++)
	{
		//定位行头
		pdst=pdstbk;
		
		for(UINT x=0; x<Size; x++)
		{
			z=(float)pow((double)y/(double)Size, Power);
			*pdst++=ColortoRGBUnsigned(z,z,z,1);
		}//end for x*/
		pdstbk+=dstrect.Pitch/4;
	}//end for y

	//初始化结束解除锁定
	PowerMap->UnlockRect(0);
	CreatePowerAttrib=1;
	return S_OK;
}


HRESULT NORMALMAP::InitAnisotropyMap(UINT Size)
{
	if(CreateAnisotropyAttrib) return E_FAIL;
	D3DLOCKED_RECT dstrect;  //锁定的数据信息
	DWORD *pdst,*pdstbk;  //信息图每行的头指针

	if(FAILED(d3ddevice->CreateTexture(Size, Size, 1, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &AnisotropyMap, NULL)))
		return E_FAIL;
	AnisotropyMap->LockRect(0, &dstrect, NULL, 0);
	pdstbk=(DWORD *)dstrect.pBits;


	//开始进行转换
	float z,a, xx, yy;

	for(UINT y=0; y<Size; y++)
	{
		//定位行头
		pdst=pdstbk;

		for(UINT x=0; x<Size; x++)
		{
			xx=(float)x/Size;
			yy=(float)y/Size;
			if(y>Size/2) yy-=1;
			if(x>Size/2) xx-=1;
			yy*=2;  xx*=2;
			
			//DIFFUSE
			a=sqrtf(1-xx*xx);
			a/=5;

			//SPECULAR
			z=sqrtf(1-xx*xx) * sqrtf(1-yy*yy) - xx*yy;
			z=powf(z, 64);
			
			*pdst++=ColortoRGBUnsigned((float)a,(float)a,(float)a,(float)z);

		}//end for x*/
		pdstbk+=dstrect.Pitch/4;
	}//end for y

	//初始化结束解除锁定
	AnisotropyMap->UnlockRect(0);
	CreateAnisotropyAttrib=1;
	return S_OK;
}


HRESULT NORMALMAP::InitAnisotropyDirMap(UINT Size, double Power)
{
	if(CreateAnisotropyDirAttrib) return E_FAIL;
	D3DLOCKED_RECT dstrect;  //锁定的数据信息
	DWORD *pdst,*pdstbk;  //信息图每行的头指针

	if(FAILED(d3ddevice->CreateTexture(Size, Size, 1, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &AnisotropyDirMap, NULL)))
		return E_FAIL;
	AnisotropyDirMap->LockRect(0, &dstrect, NULL, 0);
	pdstbk=(DWORD *)dstrect.pBits;


	//开始进行转换
	double xx, yy, r2;
	D3DXVECTOR3 vec;

	for(UINT y=0; y<Size; y++)
	{
		//定位行头
		pdst=pdstbk;

		for(UINT x=0; x<Size; x++)
		{
			//先转换为一个圆心的XY轴
			xx = (double)((int)x-(int)Size/2)*2 / (double)Size;
			yy = (double)((int)y-(int)Size/2)*-2 / (double)Size;

			//半径的平方
			r2 = yy*yy + xx*xx;

			//得到每个点切线的方向向量，无长度
			vec.x = (float)((yy>0)?yy:-yy); vec.y = (float)((xx>0)?xx:-xx);
			if(xx>0 && yy>=0) vec.y *= -1;  //第一象限垂线在第二象限，y为负，包括X轴
			else if(xx>=0 && yy<0) {vec.x *= -1; vec.y *= -1;}  //第二象限的垂线在第三象限，都为负，包括Y轴
			else if(xx<0 && yy<0) vec.x *= -1;  //第三象限垂线在第四象限，x为负
			//第四象限xy都为正
			
			//统一向量的长度，因为切线本身就是同心大大小小的圆，所以除以半径后自然就带有长度（从内圈到外圈的光照强度）信息了，然后再改变全局光照强度，即乘以常数
			//其实这一步也可以直接用vec的normalize，然后乘以强度系数来代替
			vec.x = vec.x / sqrtf((float)r2) * (float)Power;
			vec.y = vec.y / sqrtf((float)r2) * (float)Power;
			
			//原点
			if(r2 == 0) vec.x = vec.y = 0;
			vec.z=0;
			*pdst++=ColortoRGBSigned(vec.x, vec.y, vec.z, 1);

		}//end for x*/
		pdstbk+=dstrect.Pitch/4;
	}//end for y

	//初始化结束解除锁定
	AnisotropyDirMap->UnlockRect(0);
	//if(FAILED(D3DXSaveTextureToFile("Anisodir.bmp", D3DXIFF_BMP, AnisotropyDirMap, NULL))) return E_FAIL;
	CreateAnisotropyDirAttrib=1;
	return S_OK;
}


HRESULT NORMALMAP::InitFromUser(UINT Size)
{
	if(CreateNormalAttrib) return E_FAIL;
	D3DLOCKED_RECT dstrect;  //锁定的数据信息
	DWORD *pdst,*pdstbk;  //信息图每行的头指针

	if(FAILED(d3ddevice->CreateTexture(Size, Size, 1, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &NormalMap, NULL)))
		return E_FAIL;
	NormalMap->LockRect(0, &dstrect, NULL, 0);
	pdstbk=(DWORD *)dstrect.pBits;


	//开始进行转换
	float z;

	for(UINT y=0; y<Size; y++)
	{
		//定位行头
		pdst=pdstbk;

		for(UINT x=0; x<Size; x++)
		{
			z=1-(float)x*x/Size/Size-(float)y*y/Size/Size;
			if(z>0)	*pdst++=VectoRGBSigned((float)x/Size,(float)y/Size,z,1);
			else *pdst++=0x80808080;
			
		}//end for x*/
		pdstbk+=dstrect.Pitch/4;
	}//end for y

	//初始化结束解除锁定
	NormalMap->UnlockRect(0);
	CreateNormalAttrib=1;
	return S_OK;
}







/*************************立方法线纹理*******************/
NORMALCUBEMAP::NORMALCUBEMAP()
{
	CreateAttrib=0;
	CubeMap=NULL;
	CreateAttribAtten=0;
	AttenMap=NULL;
}

NORMALCUBEMAP::~NORMALCUBEMAP()
{
	Release();
}

void NORMALCUBEMAP::Release()
{
	SAFE_RELEASE(CubeMap);
	CubeMap = NULL;
	SAFE_RELEASE(AttenMap);
	AttenMap = NULL;
	CreateAttrib=0;
	CreateAttribAtten=0;
}


HRESULT NORMALCUBEMAP::SetCubeTexture(DWORD Stage)
{
	if(CreateAttrib==0||CubeMap==NULL) return E_FAIL;
	return d3ddevice->SetTexture(Stage, CubeMap);
}

HRESULT NORMALCUBEMAP::SetAttenTexture(DWORD Stage)
{
	if(CreateAttribAtten==0||AttenMap==NULL) return E_FAIL;
	if(FAILED(d3ddevice->SetTexture(Stage, AttenMap)))
		return E_FAIL;
	d3ddevice->SetSamplerState(Stage, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	d3ddevice->SetSamplerState(Stage, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	return S_OK;
}


HRESULT NORMALCUBEMAP::InitCubeMap(UINT Size)
{
	if(CreateAttrib) return E_FAIL;

	//开始初始化
	if((d3dcaps.TextureCaps&D3DPTEXTURECAPS_CUBEMAP)==0)
		return E_FAIL;
	if(FAILED(d3ddevice->CreateCubeTexture(Size, 1, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &CubeMap, NULL)))
		return E_FAIL;
	
	LPDIRECT3DSURFACE9 surface;
	D3DLOCKED_RECT rect;
	DWORD *p,*head;
	D3DXVECTOR3 vec;

	for(UINT i=0;i<6;i++)
	{
		CubeMap->GetCubeMapSurface((D3DCUBEMAP_FACES)i, 0, &surface);
		surface->LockRect(&rect, NULL, 0);
		head=(DWORD *)rect.pBits;
		vec.x=0; vec.y=0; vec.z=0;
		
		if(i==0) vec.x=(float)Size/2;
		if(i==1) vec.x=-1*(float)Size/2+1;
		if(i==2) vec.y=(float)Size/2;
		if(i==3) vec.y=-1*(float)Size/2+1;
		if(i==4) vec.z=(float)Size/2;
		if(i==5) vec.z=-1*(float)Size/2+1;

			for(UINT y=0;y<Size;y++)
			{
				p=head;
				for(UINT x=0;x<Size;x++)	
				{
					if(i==0)
					{
						vec.z=(float)Size/2-(float)x;
						vec.y=(float)Size/2-(float)y;
					}
					if(i==1)
					{
						vec.z=-1*(float)Size/2+(float)x+1;
						vec.y=(float)Size/2-(float)y;
					}
					if(i==2)
					{
						vec.x=-1*(float)Size/2+(float)x+1;
						vec.z=-1*(float)Size/2+(float)y+1;
					}
					if(i==3)
					{
						vec.x=-1*(float)Size/2+(float)x+1;
						vec.z=(float)Size/2-(float)y;
					}
					if(i==4)
					{
						vec.x=-1*(float)Size/2+(float)x+1;
						vec.y=(float)Size/2-(float)y;
					}
					if(i==5)
					{
						vec.x=(float)Size/2-(float)x;
						vec.y=(float)Size/2-(float)y;
					}
					*p++=VectoRGBSigned(vec.x,vec.y,vec.z,1);
				}//end x
				head+=rect.Pitch/4;
			}//end y
		surface->UnlockRect();
	}//end for

	CreateAttrib=1;
	return S_OK;
}


HRESULT NORMALCUBEMAP::InitAttenMap(UINT Size)
{
	if(CreateAttribAtten) return E_FAIL;

	//开始初始化
	if(FAILED(d3ddevice->CreateTexture(Size, Size, 1, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &AttenMap, NULL)))
		return E_FAIL;
	
	LPDIRECT3DSURFACE9 surface;
	D3DLOCKED_RECT rect;
	DWORD *p,*head;
	float distance, atten, tx, ty;

		AttenMap->GetSurfaceLevel(0, &surface);
		surface->LockRect(&rect, NULL, 0);
		head=(DWORD *)rect.pBits;
		
			for(UINT y=0;y<Size;y++)
			{
				p=head;
				ty=(float)y/(Size-1)-0.5f;

				for(UINT x=0;x<Size;x++)	
				{
					tx=(float)x/(Size-1)-0.5f;
					distance=tx*tx+ty*ty;
					if(distance>=0.25) atten=0;
					else atten=expf(-25*distance);
					*p++=VectoRGBUnsigned(atten, atten, atten, 1);
				}//end x
				head+=rect.Pitch/4;
			}//end y
		surface->UnlockRect();

	CreateAttribAtten=1;
	return S_OK;
}












/************************Shadow Map****************************/
KShadowMap::KShadowMap()
{
	m_iCreateAttrib = 0;
	m_pTexConvertor = NULL;
	m_pTexShadowMap = NULL;
	m_pOldBackBuffer = NULL;
	m_pOldDepthBuffer = NULL;
	m_pDepthBuffer = NULL;
}

KShadowMap::~KShadowMap()
{
	Release();
}

void KShadowMap::Release()
{
	m_iCreateAttrib = 0;
	SAFE_RELEASE(m_pTexConvertor);
	SAFE_RELEASE(m_pTexShadowMap);
	SAFE_RELEASE(m_pOldBackBuffer);
	SAFE_RELEASE(m_pOldDepthBuffer);
	SAFE_RELEASE(m_pDepthBuffer);
}


HRESULT KShadowMap::RenderShadowTexture(DWORD dwStage)
{
	if(!m_pTexConvertor || !m_pTexShadowMap || !m_iCreateAttrib)
		return E_FAIL;

	SAFE_RELEASE(m_pOldBackBuffer);
	SAFE_RELEASE(m_pOldDepthBuffer);

	if(FAILED(d3ddevice->GetRenderTarget(0, &m_pOldBackBuffer)))
		return E_FAIL;
	if(FAILED(d3ddevice->GetDepthStencilSurface(&m_pOldDepthBuffer)))
		return E_FAIL;

	// 直接设置新的，初始化的时候，新旧两套都已经准备好了
	V_RETURN(SetTexturedRenderTarget(0, m_pTexShadowMap, m_pDepthBuffer));

	d3ddevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0);

	d3ddevice->SetTexture(dwStage, m_pTexConvertor);

	m_iCreateAttrib = 2;
	
	return S_OK;
}

HRESULT KShadowMap::RestoreRenderTarget()
{
	if(!m_pOldBackBuffer || !m_pOldDepthBuffer || m_iCreateAttrib<2)
		return E_FAIL;
	
	// 恢复旧的缓冲
	if(FAILED(d3ddevice->SetRenderTarget(0, m_pOldBackBuffer)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pOldDepthBuffer)))
		return E_FAIL;

	SAFE_RELEASE(m_pOldBackBuffer);
	SAFE_RELEASE(m_pOldDepthBuffer);

	return S_OK;
}

HRESULT KShadowMap::SetShadowTexture(DWORD dwStage)
{
	if(dwStage > 6)
		return D3DERR_INVALIDCALL;
	if(!m_pTexConvertor || !m_pTexShadowMap || m_iCreateAttrib < 2)
		return E_FAIL;
	
	d3ddevice->SetTexture(dwStage, m_pTexShadowMap);
	d3ddevice->SetTexture(dwStage+1, m_pTexConvertor);
	
	return S_OK;
}


HRESULT KShadowMap::Init(UINT iSize /* = 512 */)
{
	if(m_iCreateAttrib)
		return S_OK;
	
	D3DSURFACE_DESC Desc;
	// 初始化阴影贴图
	D3DFORMAT Format = D3DFMT_A8R8G8B8;
	
#ifdef USE_FP16
	Format = D3DFMT_G16R16F;
#elif defined USE_FP32
	Format = D3DFMT_R32F;
#else
	Format = D3DFMT_A8R8G8B8;
#endif
	
	if(FAILED(D3DXCreateTexture(d3ddevice, iSize, iSize, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pTexShadowMap)))
		return E_FAIL;
	

	// 初始化新的深度缓冲（临时使用），同时得到旧的深度缓冲
	if(FAILED(d3ddevice->GetDepthStencilSurface(&m_pOldDepthBuffer)))
		return E_FAIL;
	if(FAILED(m_pOldDepthBuffer->GetDesc(&Desc)))
		return E_FAIL;
	SAFE_RELEASE(m_pOldDepthBuffer);

	if(FAILED(d3ddevice->CreateDepthStencilSurface(iSize, iSize, Desc.Format, Desc.MultiSampleType, Desc.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL)))
		return E_FAIL;
	

	// 初始化转换图，2048*1
	if(FAILED(D3DXCreateTexture(d3ddevice, 2048, 1, 1, D3DUSAGE_0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTexConvertor)))
		return E_FAIL;
	
	LPDIRECT3DSURFACE9 pSurf = NULL;
	D3DLOCKED_RECT Rect;
	
	if(FAILED(m_pTexConvertor->GetLevelDesc(0, &Desc)))
		return E_FAIL;
	if(FAILED(m_pTexConvertor->GetSurfaceLevel(0, &pSurf)))
		return E_FAIL;
	if(FAILED(pSurf->LockRect(&Rect, NULL, 0)))
		return E_FAIL;

	BYTE *p = (BYTE *)Rect.pBits;
	for(int x=0; x<2048; x++)
	{
		*p++ = 0;
		*p++ = (x % 0xff00) >> 3;
		*p++ = x % 0xff;
		*p++ = 0;
	}

	pSurf->UnlockRect();
	SAFE_RELEASE(pSurf);

	m_iCreateAttrib = 1;
	return S_OK;
}
























/************************Omni Shadow Map****************************/
KOmniShadowMap::KOmniShadowMap()
{
	m_iCreateAttrib = 0;
	m_pTexFaceIndex = NULL;
	m_pTexViewMatrix = NULL;
	m_pTexViewMatrixFix = NULL;
	m_pTexShadowMap = NULL;
	m_pOldBackBuffer = NULL;
	m_pOldDepthBuffer = NULL;
	m_pDepthBuffer = NULL;
}

KOmniShadowMap::~KOmniShadowMap()
{
	Release();
}

void KOmniShadowMap::Release()
{
	m_iCreateAttrib = 0;
	SAFE_RELEASE(m_pTexFaceIndex);
	SAFE_RELEASE(m_pTexViewMatrix);
	SAFE_RELEASE(m_pTexViewMatrixFix);
	SAFE_RELEASE(m_pTexShadowMap);
	SAFE_RELEASE(m_pOldBackBuffer);
	SAFE_RELEASE(m_pOldDepthBuffer);
	SAFE_RELEASE(m_pDepthBuffer);
}



HRESULT KOmniShadowMap::RenderShadowTexture(UINT iFaceNo, D3DXVECTOR3 PtPointLight)
{
	if(!m_pTexFaceIndex || !m_pTexViewMatrix || !m_pTexViewMatrixFix || !m_pTexShadowMap || !m_iCreateAttrib)
		return E_FAIL;

	// 创建View矩阵
	D3DXVECTOR3 PtLookAt[6]={D3DXVECTOR3(1,0,0),D3DXVECTOR3(-1,0,0),D3DXVECTOR3(0,1,0),D3DXVECTOR3(0,-1,0),D3DXVECTOR3(0,0,1),D3DXVECTOR3(0,0,-1)};
	PtLookAt[iFaceNo] += PtPointLight;
	D3DXVECTOR3 VecHead[6]={D3DXVECTOR3(0,1,0),D3DXVECTOR3(0,1,0),D3DXVECTOR3(0,0,-1),D3DXVECTOR3(0,0,1),D3DXVECTOR3(0,1,0),D3DXVECTOR3(0,1,0)};

	D3DXMATRIX Mat;
	D3DXMatrixLookAtLH(&m_MatView[iFaceNo], &PtPointLight, &PtLookAt[iFaceNo], &VecHead[iFaceNo]);
	// 得到View * Proj矩阵以便写入
	D3DXMATRIX MatProj;
	D3DXMatrixPerspectiveFovLH(&MatProj, D3DX_PI/2, 1.0f, 1.0f, 100.0f);
	D3DXMatrixTranspose(&Mat, &(m_MatView[iFaceNo] * MatProj));


	// 修改View Matrix贴图的数据，4*6，每行代表一个矩阵，6行分别表示6个面
	D3DLOCKED_RECT Rect;
	D3DLOCKED_RECT RectFix;
	LPDIRECT3DSURFACE9 pSurf = NULL;
	LPDIRECT3DSURFACE9 pSurfFix = NULL;

	if(FAILED(m_pTexViewMatrix->GetSurfaceLevel(0, &pSurf)))
		return E_FAIL;
	if(FAILED(pSurf->LockRect(&Rect, NULL, 0)))
		return E_FAIL;

	if(FAILED(m_pTexViewMatrixFix->GetSurfaceLevel(0, &pSurfFix)))
		return E_FAIL;
	if(FAILED(pSurfFix->LockRect(&RectFix, NULL, 0)))
		return E_FAIL;

#ifdef USE_FP16
	float16 *p = (float16 *)((BYTE *)Rect.pBits + Rect.Pitch * iFaceNo);
	float16 *pFix = (float16 *)((BYTE *)RectFix.pBits + RectFix.Pitch * iFaceNo);
#elif defined USE_FP32
	float *p = (float *)((BYTE *)Rect.pBits + Rect.Pitch * iFaceNo);
	float *pFix = (float *)((BYTE *)RectFix.pBits + RectFix.Pitch * iFaceNo);
#endif


	float fValue = 0.0f, fValueFix = 0.0f;

	for(int iY=0; iY < 4; iY++)
	{
		for(int iX=0; iX < 4; iX++)
		{
			// 因为贴图无法存储负数，所以这里遇到负数就用绝对值 - 2 * 绝对值的方式来修正，如果是正数，就让Fix = 0，这样就不会影响原数据
			if(Mat[iY * 4 + iX] < 0.0f)
			{
				fValue = absf(Mat[iY * 4 + iX]);
				fValueFix = absf(Mat[iY * 4 + iX]) * 2.0f;
			}
			else
			{
				fValue = Mat[iY * 4 + iX];
				fValueFix = 0.0f;
			}

#ifdef USE_FP16
			*p++ = floatToFP16(fValue);
			*pFix++ = floatToFP16(fValueFix);
#elif defined USE_FP32
			*p++ = fValue;
			*pFix++ = fValueFix;
#endif
		}
	}


	pSurf->UnlockRect();
	SAFE_RELEASE(pSurf);

	pSurfFix->UnlockRect();
	SAFE_RELEASE(pSurfFix);


	// 直接设置新的，初始化的时候，新旧两套都已经准备好了
	V_RETURN(SetTexturedRenderTarget(0, iFaceNo, m_pTexShadowMap, m_pDepthBuffer));

	d3ddevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0);


	m_iCreateAttrib = 2;

	return S_OK;
}

HRESULT KOmniShadowMap::SaveRenderTarget()
{
	if(!m_iCreateAttrib)
		return E_FAIL;

	// 保存原有缓冲
	SAFE_RELEASE(m_pOldBackBuffer);
	SAFE_RELEASE(m_pOldDepthBuffer);

	if(FAILED(d3ddevice->GetRenderTarget(0, &m_pOldBackBuffer)))
		return E_FAIL;
	if(FAILED(d3ddevice->GetDepthStencilSurface(&m_pOldDepthBuffer)))
		return E_FAIL;

	return S_OK;
}



HRESULT KOmniShadowMap::RestoreRenderTarget()
{
	if(!m_pOldBackBuffer || !m_pOldDepthBuffer || m_iCreateAttrib < 2)
		return E_FAIL;

	// 恢复旧的缓冲
	if(FAILED(d3ddevice->SetRenderTarget(0, m_pOldBackBuffer)))
		return E_FAIL;
	if(FAILED(d3ddevice->SetDepthStencilSurface(m_pOldDepthBuffer)))
		return E_FAIL;

	SAFE_RELEASE(m_pOldBackBuffer);
	SAFE_RELEASE(m_pOldDepthBuffer);

	return S_OK;
}

HRESULT KOmniShadowMap::SetShadowTexture(DWORD dwStage)
{
	if(dwStage > 4)
		return D3DERR_INVALIDCALL;
	if(!m_pTexFaceIndex || !m_pTexViewMatrix || !m_pTexViewMatrixFix || !m_pTexShadowMap || m_iCreateAttrib < 2)
		return E_FAIL;

	d3ddevice->SetTexture(dwStage, m_pTexShadowMap);
	d3ddevice->SetTexture(dwStage + 1, m_pTexFaceIndex);
	d3ddevice->SetTexture(dwStage + 2, m_pTexViewMatrix);
	d3ddevice->SetTexture(dwStage + 3, m_pTexViewMatrixFix);

	return S_OK;
}


HRESULT KOmniShadowMap::Init(UINT iSize /* = 512 */)
{
	if(m_iCreateAttrib)
		return S_OK;

	D3DSURFACE_DESC Desc;
	// 初始化阴影贴图
	D3DFORMAT Format = D3DFMT_A8R8G8B8;

	if(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_CUBETEXTURE, D3DFMT_R32F))
	{
		OutputDebugString("硬件不支持R32F格式的贴图！\n");
		return E_FAIL;
	}

#ifdef USE_FP16
	Format = D3DFMT_G16R16F;
#elif defined USE_FP32
	Format = D3DFMT_R32F;
#else
	mymessage("没有定义使用浮点贴图！");
	return E_FAIL;
#endif

	if(FAILED(D3DXCreateCubeTexture(d3ddevice, iSize, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pTexShadowMap)))
		return E_FAIL;


	// 初始化新的深度缓冲（临时使用），同时得到旧的深度缓冲
	if(FAILED(d3ddevice->GetDepthStencilSurface(&m_pOldDepthBuffer)))
		return E_FAIL;
	if(FAILED(m_pOldDepthBuffer->GetDesc(&Desc)))
		return E_FAIL;
	SAFE_RELEASE(m_pOldDepthBuffer);

	if(FAILED(d3ddevice->CreateDepthStencilSurface(iSize, iSize, Desc.Format, Desc.MultiSampleType, Desc.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL)))
		return E_FAIL;


	// 初始化矩阵
	for(UINT i = 0; i < 6; i++)
		D3DXMatrixIdentity(&m_MatView[i]);


	LPDIRECT3DSURFACE9 pSurf = NULL;
	D3DLOCKED_RECT Rect;
	UINT iFaceNo = 0;
	UINT iX = 0, iY = 0;

	
	float *p = NULL;

	// 初始化数据图存放面序号，2*2即可，这个贴图不晓得怎么回事，用FP16一转就出问题，1.0变成0.0，那就强制用FP32吧
	if(FAILED(D3DXCreateCubeTexture(d3ddevice, 2, 1, D3DUSAGE_0, D3DFMT_R32F, D3DPOOL_MANAGED, &m_pTexFaceIndex)))
		return E_FAIL;

	if(FAILED(m_pTexFaceIndex->GetLevelDesc(0, &Desc)))
		return E_FAIL;
	for(iFaceNo = 0; iFaceNo < 6; iFaceNo++)
	{
		if(FAILED(m_pTexFaceIndex->GetCubeMapSurface(( D3DCUBEMAP_FACES)iFaceNo, 0, &pSurf)))
			return E_FAIL;
		if(FAILED(pSurf->LockRect(&Rect, NULL, 0)))
			return E_FAIL;


		for(iY = 0; iY < 2; iY++)
		{
			p = (float *)((BYTE *)Rect.pBits + Rect.Pitch * iY);

			for(iX = 0; iX < 2; iX++)
			{
				float fFaceIndex = (float)iFaceNo;
				*p++ = fFaceIndex;
			}
		}

		pSurf->UnlockRect();
		SAFE_RELEASE(pSurf);
	}// end each face



	// 初始化数据图存放View Matrix，4*6，每行代表一个矩阵，6行分别表示6个面
#ifdef USE_FP16
	Format = D3DFMT_A16B16G16R16F;
#elif defined USE_FP32
	Format = D3DFMT_A32B32G32R32F;
#else
	mymessage("没有定义使用浮点贴图！");
	return E_FAIL;
#endif

	if(FAILED(D3DXCreateTexture(d3ddevice, 4, 6, 1, D3DUSAGE_0, Format, D3DPOOL_MANAGED, &m_pTexViewMatrix)))
		return E_FAIL;

	if(FAILED(D3DXCreateTexture(d3ddevice, 4, 6, 1, D3DUSAGE_0, Format, D3DPOOL_MANAGED, &m_pTexViewMatrixFix)))
		return E_FAIL;


	// 成功
	m_iCreateAttrib = 1;
	return S_OK;
}










KFPRenderTargetClear::KFPRenderTargetClear()
{
	m_iCreateAttrib = 0;
	m_pTexData = NULL;
	m_fR = m_fG = m_fB = m_fA = 1.0f;
}

KFPRenderTargetClear::~KFPRenderTargetClear()
{
	Release();
}

void KFPRenderTargetClear::Release()
{
	m_iCreateAttrib = 0;
	SAFE_RELEASE(m_pTexData);
}


HRESULT KFPRenderTargetClear::Init(LPDIRECT3DTEXTURE9 pSourceTex, float fR /* = 1.0f */, float fG /* = 1.0f */, float fB /* = 1.0f */, float fA /* = 1.0f */)
{
	if(m_iCreateAttrib || !pSourceTex)
		return D3DERR_INVALIDCALL;

	pSourceTex->GetLevelDesc(0, &m_Desc);

	if(m_Desc.Usage != D3DUSAGE_RENDERTARGET)
		return D3DERR_INVALIDCALL;
	// 检查格式范围在R16F到A32B32……之间
	if(m_Desc.Format > 116 || m_Desc.Format < 111)
		return D3DERR_INVALIDCALL;


	// 按照指定格式创建刷新贴图
	V_RETURN(D3DXCreateTexture(d3ddevice, m_Desc.Width, m_Desc.Height, 1, D3DUSAGE_0, m_Desc.Format, D3DPOOL_DEFAULT, &m_pTexData));
	m_iCreateAttrib = 1;

	// 刷新指定数据
	V_RETURN(RefreshFloatValue(fR, fG, fB, fA));

	return S_OK;
}


HRESULT KFPRenderTargetClear::Init(LPDIRECT3DCUBETEXTURE9 pSourceTex, float fR /* = 1.0f */, float fG /* = 1.0f */, float fB /* = 1.0f */, float fA /* = 1.0f */)
{
	if(m_iCreateAttrib || !pSourceTex)
		return D3DERR_INVALIDCALL;

	pSourceTex->GetLevelDesc(0, &m_Desc);

	if(m_Desc.Usage != D3DUSAGE_RENDERTARGET)
		return D3DERR_INVALIDCALL;
	// 检查格式范围在R16F到A32B32……之间
	if(m_Desc.Format > 116 || m_Desc.Format < 111)
		return D3DERR_INVALIDCALL;


	// 按照指定格式创建刷新贴图
	V_RETURN(D3DXCreateTexture(d3ddevice, m_Desc.Width, m_Desc.Height, 1, D3DUSAGE_0, m_Desc.Format, D3DPOOL_DEFAULT, &m_pTexData));
	m_iCreateAttrib = 1;

	// 刷新指定数据
	V_RETURN(RefreshFloatValue(fR, fG, fB, fA));

	return S_OK;
}







HRESULT KFPRenderTargetClear::Clear(LPDIRECT3DTEXTURE9 pSourceTex, float fR /* = 1.0f */, float fG /* = 1.0f */, float fB /* = 1.0f */, float fA /* = 1.0f */)
{
	if(!m_iCreateAttrib || !m_pTexData)
		return D3DERR_NOTAVAILABLE;

	// 校验是否和当前Data贴图的数据一致，如果不一致就Refresh数据贴图
	if(fR != m_fR || fG != m_fG || fB != m_fB || fA != m_fA)
		V_RETURN(RefreshFloatValue(fR, fG, fB, fA));

	// StrectRect
	D3DSURFACE_DESC Desc;
	pSourceTex->GetLevelDesc(0, &Desc);
	if(Desc.Format != m_Desc.Format || Desc.Usage != D3DUSAGE_RENDERTARGET || Desc.Width != m_Desc.Width || Desc.Height != m_Desc.Height)
		return D3DERR_NOTAVAILABLE;


	LPDIRECT3DSURFACE9 pSurfSrc = NULL, pSurfDst = NULL;
	pSourceTex->GetSurfaceLevel(0, &pSurfDst);
	m_pTexData->GetSurfaceLevel(0, &pSurfSrc);
	V_RETURN(d3ddevice->StretchRect(pSurfSrc, NULL, pSurfDst, NULL, D3DTEXF_NONE));


	SAFE_RELEASE(pSurfSrc);
	SAFE_RELEASE(pSurfDst);
	return S_OK;
}


HRESULT KFPRenderTargetClear::Clear(LPDIRECT3DCUBETEXTURE9 pSourceTex, UINT iFaceNo, float fR /* = 1.0f */, float fG /* = 1.0f */, float fB /* = 1.0f */, float fA /* = 1.0f */)
{
	if(!m_iCreateAttrib || !m_pTexData)
		return D3DERR_NOTAVAILABLE;

	// 校验是否和当前Data贴图的数据一致，如果不一致就Refresh数据贴图
	if(fR != m_fR || fG != m_fG || fB != m_fB || fA != m_fA)
		V_RETURN(RefreshFloatValue(fR, fG, fB, fA));

	// StrectRect
	D3DSURFACE_DESC Desc;
	pSourceTex->GetLevelDesc(0, &Desc);
	if(Desc.Format != m_Desc.Format || Desc.Usage != D3DUSAGE_RENDERTARGET || Desc.Width != m_Desc.Width || Desc.Height != m_Desc.Height)
		return D3DERR_NOTAVAILABLE;


	LPDIRECT3DSURFACE9 pSurfSrc = NULL, pSurfDst = NULL;
	pSourceTex->GetCubeMapSurface((_D3DCUBEMAP_FACES)iFaceNo, 0, &pSurfDst);
	m_pTexData->GetSurfaceLevel(0, &pSurfSrc);
	V_RETURN(d3ddevice->StretchRect(pSurfSrc, NULL, pSurfDst, NULL, D3DTEXF_NONE));


	SAFE_RELEASE(pSurfSrc);
	SAFE_RELEASE(pSurfDst);
	return S_OK;
}






















HRESULT KFPRenderTargetClear::RefreshFloatValue(float fR, float fG, float fB, float fA)
{
	if(!m_iCreateAttrib || !m_pTexData)
		return D3DERR_NOTAVAILABLE;

	// 先创建内存贴图
	LPDIRECT3DTEXTURE9 pTexRefresh = NULL;
	if(FAILED(D3DXCreateTexture(d3ddevice, m_Desc.Width, m_Desc.Height, 1, D3DUSAGE_0, m_Desc.Format, D3DPOOL_SYSTEMMEM, &pTexRefresh)))
		return E_FAIL;

	// 写入内存贴图
	D3DLOCKED_RECT Rect;
	LPDIRECT3DSURFACE9 pSurfSrc = NULL, pSurfDst = NULL;
	UINT iX = 0, iY = 0;

	if(FAILED(pTexRefresh->GetSurfaceLevel(0, &pSurfSrc)))
		return E_FAIL;
	if(FAILED(pSurfSrc->LockRect(&Rect, NULL, 0)))
		return E_FAIL;

	float16 *pClear16 = NULL;
	float *pClear32 = NULL;

	for(iY=0; iY < m_Desc.Height; iY++)
	{
		pClear16 = (float16 *)((BYTE *)Rect.pBits + Rect.Pitch * iY);
		pClear32 = (float *)((BYTE *)Rect.pBits + Rect.Pitch * iY);

		for(iX=0; iX < m_Desc.Width; iX++)
		{
			switch(m_Desc.Format)
			{
			case D3DFMT_R16F:
				*pClear16++ = floatToFP16(fR);
				break;
			case D3DFMT_G16R16F:
				*pClear16++ = floatToFP16(fR);
				*pClear16++ = floatToFP16(fG);
				break;
			case D3DFMT_A16B16G16R16F:
				*pClear16++ = floatToFP16(fR);
				*pClear16++ = floatToFP16(fG);
				*pClear16++ = floatToFP16(fB);
				*pClear16++ = floatToFP16(fA);
				break;
			case D3DFMT_R32F:
				*pClear32++ = fR;
				break;
			case D3DFMT_G32R32F:
				*pClear32++ = fR;
				*pClear32++ = fG;
				break;
			case D3DFMT_A32B32G32R32F:
				*pClear32++ = fR;
				*pClear32++ = fG;
				*pClear32++ = fB;
				*pClear32++ = fA;
				break;
			}// end for each pixel
		}// end for x
	}// end for y

	pSurfSrc->UnlockRect();


	// 内存图刷到显存图备用
	if(FAILED(m_pTexData->GetSurfaceLevel(0, &pSurfDst)))
		return E_FAIL;

	if(FAILED(d3ddevice->UpdateSurface(pSurfSrc, NULL, pSurfDst, NULL)))
		return E_FAIL;

	SAFE_RELEASE(pSurfSrc);
	SAFE_RELEASE(pSurfDst);
	SAFE_RELEASE(pTexRefresh);

	// 更新内部数据
	m_fR = fR;
	m_fG = fG;
	m_fB = fB;
	m_fA = fA;

	return S_OK;
}




















/****************************Texture Copy*************************/
/////////外部接口
KTextureCopy::KTextureCopy()
{
	m_iCreateAttrib = 0;
	m_iStride = 0;
	m_iWidth = m_iHeight = 0;

	m_pDepthBuffer = NULL;
	m_pVB = NULL;
	
	m_pVSDrawQuad = NULL;
	m_pDeclaration = NULL;
	m_pPSCopyPoint = NULL;
	m_pPSCopyBilinear = NULL;
}

void KTextureCopy::Release()
{
	m_iCreateAttrib = 0;
	m_iStride = 0;
    m_iWidth = m_iHeight = 0;

	SAFE_RELEASE(m_pDepthBuffer);

	SAFE_RELEASE(m_pVB);

	SAFE_RELEASE(m_pVSDrawQuad);
	SAFE_RELEASE(m_pDeclaration);
	SAFE_RELEASE(m_pPSCopyPoint);
	SAFE_RELEASE(m_pPSCopyBilinear);
}


HRESULT KTextureCopy::TextureCopy(LPDIRECT3DTEXTURE9 pTexDst, LPDIRECT3DTEXTURE9 pTexSrc, BOOL bShaderFilter /* = TRUE */)
{
	if(!m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;
	if(!pTexDst || !pTexSrc)
		return D3DERR_INVALIDCALL;

	BOOL bBilinear = FALSE;

	// 检查贴图格式和分辨率
	D3DSURFACE_DESC DescSrc, DescDst;
	V_RETURN(pTexDst->GetLevelDesc(0, &DescDst));
	V_RETURN(pTexSrc->GetLevelDesc(0, &DescSrc));

		// 源图不能是内存纹理，目的图必须是RT！
	if(DescSrc.Pool == D3DPOOL_SYSTEMMEM || DescDst.Usage != D3DUSAGE_RENDERTARGET)
		return D3DERR_WRONGTEXTUREFORMAT;

	if(DescSrc.Width != DescDst.Width || DescSrc.Height != DescDst.Height)
		bBilinear = TRUE;

	// 得到原深度缓冲、RT和Shader
	LPDIRECT3DSURFACE9 pOldDepth = NULL, pOldRT = NULL;
	LPDIRECT3DPIXELSHADER9 pOldPS = NULL;
	LPDIRECT3DVERTEXSHADER9 pOldVS = NULL;

	V_RETURN(d3ddevice->GetDepthStencilSurface(&pOldDepth));
	V_RETURN(d3ddevice->GetRenderTarget(0, &pOldRT));
	V_RETURN(d3ddevice->GetVertexShader(&pOldVS));
	V_RETURN(d3ddevice->GetPixelShader(&pOldPS));


	// 深度缓冲小了或还未创建，必须重建
	if(DescDst.Width > m_iWidth || DescDst.Height > m_iHeight || !m_pDepthBuffer)
	{
		SAFE_RELEASE(m_pDepthBuffer);
		m_iWidth = DescDst.Width;
		m_iHeight = DescDst.Height;

		D3DSURFACE_DESC DescDepth;
		if(FAILED(pOldDepth->GetDesc(&DescDepth)))
			return E_FAIL;

		// 好像不用DepthBuffer也无所谓？关了Z缓冲就成了
		//V_RETURN(d3ddevice->CreateDepthStencilSurface(m_iWidth, m_iHeight, DescDepth.Format, DescDepth.MultiSampleType, DescDepth.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL));

	}
	

	// 设置
	V_RETURN(d3ddevice->SetDepthStencilSurface(m_pDepthBuffer));
	V_RETURN(SetTexturedRenderTarget(0, pTexDst, NULL));
	V_RETURN(d3ddevice->SetTexture(0, pTexSrc));

	if(bBilinear && bShaderFilter)
	{
		V_RETURN(d3ddevice->SetPixelShader(m_pPSCopyBilinear));
	}
	else
		V_RETURN(d3ddevice->SetPixelShader(m_pPSCopyPoint));

	V_RETURN(d3ddevice->SetVertexShader(m_pVSDrawQuad));
	V_RETURN(d3ddevice->SetVertexDeclaration(m_pDeclaration));
	V_RETURN(d3ddevice->SetFVF(0));


	// 常量寄存器
	d3ddevice->SetVertexShaderConstantF(0, (float *)&D3DXVECTOR4((float)DescSrc.Width, (float)DescSrc.Height, 0, 0), 1);

	float fOneTapWidth = 1.0f / (float)DescSrc.Width;
	float fOneTapHeight = 1.0f / (float)DescSrc.Height;
	d3ddevice->SetPixelShaderConstantF(0, (float *)&D3DXVECTOR4(0.5f * fOneTapWidth, 0.5f * fOneTapHeight, 0, 0), 1);
	d3ddevice->SetPixelShaderConstantF(1, (float *)&D3DXVECTOR4(fOneTapWidth, fOneTapHeight, 0, 0), 1);
	d3ddevice->SetPixelShaderConstantF(2, (float *)&D3DXVECTOR4(0.5f, 1.0f, 2.0f, 0.0f), 1);


	// 设置渲染参数（关掉Z）
	DWORD dwZ = 0;
	V_RETURN(d3ddevice->GetRenderState(D3DRS_ZENABLE, &dwZ));
	V_RETURN(d3ddevice->SetRenderState(D3DRS_ZENABLE, TRUE));

	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
	V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
	if(bBilinear)
	{
		V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
		V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
	}
	else
	{
		V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
		V_RETURN(d3ddevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
	}

	// 渲染
	V_RETURN(d3ddevice->SetStreamSource(0, m_pVB, 0, m_iStride));
	V_RETURN(d3ddevice->BeginScene());
	V_RETURN(d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
	V_RETURN(d3ddevice->EndScene());




	// 恢复，结束
	V_RETURN(d3ddevice->SetRenderState(D3DRS_ZENABLE, dwZ));

	V_RETURN(d3ddevice->SetVertexShader(pOldVS));
	V_RETURN(d3ddevice->SetPixelShader(pOldPS));
	V_RETURN(d3ddevice->SetDepthStencilSurface(pOldDepth));
	V_RETURN(d3ddevice->SetRenderTarget(0, pOldRT));

	SAFE_RELEASE(pOldRT);
	SAFE_RELEASE(pOldDepth);
	SAFE_RELEASE(pOldVS);
	SAFE_RELEASE(pOldPS);

	return S_OK;
}







HRESULT KTextureCopy::Init()
{
	if(m_iCreateAttrib)
		return D3DERR_NOTAVAILABLE;

	// 初始化Quad
	V_RETURN(d3ddevice->CreateVertexBuffer(4*sizeof(QUADVERTEXTYPE), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVB, NULL));

	QUADVERTEXTYPE QuadVertex[4] = 
	{
		{-1.0f, -1.0f, 0,	0,1,0,0},
		{-1.0f, 1.0f, 0,	0,0,0,0},
		{1.0f, -1.0f, 0,	1,1,0,0},
		{1.0f, 1.0f, 0,		1,0,0,0}
	};

	m_iStride = sizeof(QUADVERTEXTYPE);

	VOID* pVertexStream;
	V_RETURN(m_pVB->Lock(0, 4 * m_iStride, &pVertexStream, 0));
	memcpy(pVertexStream, QuadVertex, 4 * m_iStride);
	m_pVB->Unlock();


	// 初始化Vertex Shader
	D3DVERTEXELEMENT9 Dclr[] = {
		{0,0,D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0,12,D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	V_RETURN(d3ddevice->CreateVertexDeclaration(Dclr, &m_pDeclaration));

	// c0存放需要Bilinear Filtering的来源纹理的分辨率
	char szVS[] = "vs.2.0\
		dcl_position v0\
		dcl_texcoord v1\
		mov oPos, v0\
		mov oT0, v1\
		mul oT1, v1, c0\
		";


	LPD3DXBUFFER ComplicationErrors = NULL, CompiledShader = NULL;
	V_RETURN(D3DXAssembleShader(szVS, strlen(szVS), NULL, NULL, D3DXSHADER_DEBUG, &CompiledShader, &ComplicationErrors));
	V_RETURN(d3ddevice->CreateVertexShader((DWORD*)CompiledShader->GetBufferPointer(), &m_pVSDrawQuad));
	SAFE_RELEASE(CompiledShader);
	SAFE_RELEASE(ComplicationErrors);


	// 创建PS
	// c0存放来源纹理的1/2 Tap
	// c1存放来源纹理的1Tap
	// c2是常量：0.5, 1, 2, 0
	char szInclude[] = "#define Temp1 r10\
					   #define Temp2 r11\
					   #define cHalfTap c0\
					   #define cOneTap c1\
					   #define HALF c2.x\
					   #define ONE c2.y\
					   ";

	char szPSPoint[] = "ps.2.0\
						dcl_2d s0\
						dcl t0\
						dcl t1\
						add r0, t0, c0\
						texld r1, r0, s0\
						mov oC0, r1\
				  ";


	(D3DXAssembleShader(szPSPoint, strlen(szPSPoint), NULL, NULL, D3DXSHADER_DEBUG, &CompiledShader, &ComplicationErrors));
	V_RETURN(d3ddevice->CreatePixelShader((DWORD*)CompiledShader->GetBufferPointer(), &m_pPSCopyPoint));
	SAFE_RELEASE(CompiledShader);
	SAFE_RELEASE(ComplicationErrors);




	char szPSBilinear[] = "	ps.2.0\
							dcl_2d s0\
							dcl t0\
							dcl t1\
							\
							frc r11, t1\
							sub r1, t1, r11\
							\
							sub r11.z, c2.y, r11.x\
							sub r11.w, c2.y, r11.y\
							\
							add r4, r1, c2.y\
							\
							mov r2.x, r4.x\
							mov r2.y, r1.y\
							mov r2.zw, c2.y\
							\
							mov r3.x, r1.x\
							mov r3.y, r4.y\
							mov r3.zw, c2.y\
							\
							mul r5.x, r11.z, r11.w\
							mul r5.y, r11.x, r11.w\
							mul r5.z, r11.z, r11.y\
							mul r5.w, r11.x, r11.y\
							\
							add r1, r1, c2.x\
							mul r1, r1, c1\
							texld r1, r1, s0\
							\
							add r2, r2, c2.x\
							mul r2, r2, c1\
							texld r2, r2, s0\
							\
							add r3, r3, c2.x\
							mul r3, r3, c1\
							texld r3, r3, s0\
							\
							add r4, r4, c2.x\
							mul r4, r4, c1\
							texld r4, r4, s0\
							\
							mul r11, r1, r5.x\
							mad r11, r2, r5.y, r11\
							mad r11, r3, r5.z, r11\
							mad r11, r4, r5.w, r11\
							\
							mov oC0, r11\
					   ";


	V_RETURN(D3DXAssembleShader(szPSBilinear, strlen(szPSBilinear), NULL, NULL, D3DXSHADER_DEBUG, &CompiledShader, &ComplicationErrors));
	V_RETURN(d3ddevice->CreatePixelShader((DWORD*)CompiledShader->GetBufferPointer(), &m_pPSCopyBilinear));
	SAFE_RELEASE(CompiledShader);
	SAFE_RELEASE(ComplicationErrors);


	m_iCreateAttrib = 1;
	return S_OK;
}