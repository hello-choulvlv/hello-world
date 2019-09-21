#pragma once

//这里是各种摄像机控制的定义，
typedef enum CAMERACHANGEWAY
{
	CAM_NOCHANGE	=	0,		// 未移动
	CAM_GOFORWARD	=	1,		//前进后退
	CAM_GOBACKWARD	=	2,
	CAM_GOLEFT		=	4,		//左右平移
    CAM_GORIGHT		=	8,
	CAM_GOUP		=	16,		//上下平移
	CAM_GODOWN		=	32,
	CAM_TURNLEFT	=	64,		//左右转动视角
	CAM_TURNRIGHT	=	128,
	CAM_TURNUP		=	256,	//上下转动视角
	CAM_TURNDOWN	=	512,
	CAM_SLICELEFT	=	1024,	//左右颠簸
	CAM_SLICERIGHT	=	2048,
}CameraChangeWay;


class CAMERACHANGE
{
public:
	D3DXVECTOR3 Eye;   //摄像机所在位置
	D3DXVECTOR3 Look;     //视线方向点（是坐标而非向量，它的值就是VecLook+Eye）
	float m_fProjXYCoef;		// XY轴的比例系数，PROJ变换用

	CAMERACHANGE();
	// 初始化摄像机，EYE表示摄像机位置坐标，LOOK表示视线点坐标，move表每秒移动(前进/后退)的距离，rise表每秒平移(上下/左右)的距离，angle表每秒转向的角度（为了方便，输入弧度也会自动匹配）
	// 可多次初始化，就是强制改变摄像机位置
	HRESULT InitCamera(float Eyex, float Eyey, float Eyez, float Lookx, float Looky, float Lookz, float Headx, float Heady, float Headz, float move, float rise, float angle);
	//changeway表示镜头改变的方式，见上面的DEFINE，可以多种方式一起进行，如前移加左转，用|连接
	// MoveCoef表示移动速度，用于鼠标控制，根据鼠标移动的距离来控制基准速度（Init时给定的速度）的倍数
	void ChangeCamera(unsigned short int changeway, float fMoveCoef = 1.0f);
	// 将Slice和TurnUp/Down的角度固定下来，即是说让Head与Y重合，让Look和Right_X在XZ面上
	void ResetCamera();
	
	//根据当前矩阵myproj和myview进行观察变换和剪裁变换，方便使用的函数
	void ViewTransform();
	void ProjTransform(float xycoef = 0);     //参数是XY轴的比例系数，默认为屏幕分辨率纵横轴之比

	// 得到View Frustum9个顶点在世界空间中的坐标，只能用于透视投影的Frustum，而且传入的指针必须分配好9个VECTOR，从0到8分别是Eye、Near的左上右上左下右下、Far的左上右上左下右下
	HRESULT GetEyeFrustum(D3DXVECTOR3 *PtFrustum, float fNear, float fFar);

	void SetFrameTime();	// 设置本帧时间，每帧都要进行，内部函数，只在UserControl中使用

	float GetFrameTime();	// 得到上一帧渲染前到本帧渲染前的时间间隔（单位秒），包括Present，便于物体移动、亮度适应或其他和时间相关的参数



	float GetIdleTime();	// 得到已经Idle了多久（单位秒），如果当前是移动状态，返回0.0
	
	bool IsIdle() { return m_MoveAttrib==0 ? true : false; }			// 查询是否处于静止状态
	bool IsMoving() { return m_MoveAttrib==1? true : false; }			// 查询是否处于移动状态
	bool IsIdleToMoving() { return m_MoveAttrib==2 ? true : false; }	// 查询是否刚从静止到移动
	bool IsMovingToIdle() { return m_MoveAttrib==3 ? true : false; }	// 查询是否刚从移动到静止

private:
	D3DXVECTOR3 m_VecLook;   //视线方向向量
	D3DXVECTOR3 m_VecHead;   //头顶方向向量
	D3DXVECTOR3 m_VecRight_X;	// X轴向量，这两个向量和Look组成一个正交空间
	
	bool m_bInit;		// 是否已初始化
	bool m_bIdle;	// true表示Idle状态，false表示移动状态
	BYTE m_MoveAttrib;	// 0表示Idle，1表示Moving，2表示刚从Idle到Moving，3表示刚从Moving到Idle
	DWORD dwLastIdleTime;	// 上次IDLE的时间，用于计算截至到现在已经Idle了多久（用于HDR）
	DWORD dwCurrentTime, dwLastFrameTime;	// 当前时间和上一帧的时间（这两个是真实时间，而非两帧之间的时间差）
	DWORD dwTempLastFrameTime;	// 临时使用，用于GetFrameTime
	
	float m_fMoveSpeed;		// 前进时每秒移动距离（世界坐标系中）
	float m_fRiseSpeed;		// 上升下降时每秒移动距离（世界坐标系中）
	float m_fAngleSpeed;	// 转向时每秒转动的角度(弧度)
	
	bool m_bYMove;	// 前后和左右平移时是否考虑Y轴，如考虑则根据视线和Y轴夹角调整移动的高度，如不考虑则只是在一个和XZ平面平行的平面内移动，初始为关闭（不考虑）
};








// 这个类只是简化操作罢了，并不牵扯到Shader
class KTrapezoidalShadowMap
{
public:
	KTrapezoidalShadowMap();
	~KTrapezoidalShadowMap();
	
	// 初始化转换贴图，创建阴影贴图
	HRESULT Init(UINT iWidth = 512, UINT iHeight = 0);
	void Release();
	
	// 设置第几个阴影贴图为RT，准备进行渲染，
	HRESULT RenderShadowTexture(UINT iShadowMapNo = 0);
	// 渲染好阴影贴图之后一定要恢复旧的RT和深度缓冲
	HRESULT RestoreRenderTarget();
	// 设置渲染好的阴影贴图，准备渲染物体阴影，注意考虑到转换纹理，所以它要占两层贴图
	HRESULT SetShadowTexture(DWORD dwStage);

	// 设置光源属性，方向、位置，同时计算LVP的VIEW和PROJ矩阵
	// fMinRange, fMaxRange是光源Z轴范围大小（即LVP远近剪裁面），fX/YRangeCoef是光源X和Y轴的范围大小系数，初始状态时根据光源类型不同分别为PI/2,PI/2（透视）或2.0,2.0（正交）
	HRESULT SetLight(D3DXVECTOR3 PtLightPos, D3DXVECTOR3 VecLightDirection, D3DXVECTOR3 VecHeadDirection, float fMinRange = 1.0f, float fMaxRange = 500.0f, D3DLIGHTTYPE LightType = D3DLIGHT_DIRECTIONAL, float fXRangeCoef = 1.0f, float fYRangeCoef = 1.0f);

	// 设置虚拟EF的远近剪裁面，用于解决近剪裁面过小，远处阴影质量下降的问题，默认是2～200
	HRESULT SetFrustumRange(float fNear, float fFar);
	
	// 得到WORLD->LVP空间VIEW*PROJ矩阵和WORLD->T空间转换矩阵（组合好的完整矩阵，直接可以使用），T空间矩阵必须传矩阵数组的指针进来，因为考虑到会有视线和光照平行的问题，可能要用到4张不同的ShadowMap
	HRESULT GetMatrix(D3DXMATRIX *pMatLVPProj, D3DXMATRIX pMatTSpace[4]);
	
	
	
	// 是否强制USM（即T矩阵强制为Identity），如果符合两个Problem条件，就强制为USM
	bool m_bForceUSM;
	// 如果LVP空间中，EYE跑到Far Plane里面去了，就置该标记，解决方法是4个ShadowMaps，但这里为了方便直接将Far Plane作为矩形传入
	bool m_bFourShadowMaps;
	

	
	
//下面这些都是内部函数，测试完后就移到private中去
	// 得到从LVP空间到Trapezoidal空间的转换矩阵，仅LVP—>T
	HRESULT GetTSpaceMatrix(D3DXMATRIX *pMatTSpace);
	
	// 将World空间的Eye Frustum投影到LVP空间去
	void ViewFrustumToLVPSpace(D3DXVECTOR3 *PtFrustum);
	// 通过LVP投影后的Eye Frustum计算出梯形
	HRESULT FrustumToTrapezoid(D3DXVECTOR3 *pPtFrustum, D3DXVECTOR3 *pPtTrapezoid);
	// 将梯形从LVP空间变换到T-SPACE，返回最终的转换矩阵
	D3DXMATRIX TransformToTrapezoidSpace(D3DXVECTOR3 PtTopL, D3DXVECTOR3 PtTopR, D3DXVECTOR3 PtBottomL, D3DXVECTOR3 PtBottomR, D3DXVECTOR3 PtIntersect);
	
	// 光源属性及矩阵
	D3DXVECTOR3 m_VecLightDirection, m_VecHeadDirection, m_VecRightDirection;
	D3DXVECTOR3 m_PtLightPosition;
	D3DXMATRIX m_MatLVPProj, m_MatLVPView;
private:
	// 建立转换贴图是否成功，还有是否渲染过阴影贴图，1表示未渲染过阴影贴图，2表示都成功
	UINT m_iCreateAttrib;
	
	// 光源类型，暂时只支持SPOT和DIRECTIONAL，这两种类型决定了LVP的投影矩阵类型
	D3DLIGHTTYPE m_LightType;
	
	// 该TSM对应虚拟EF远近剪裁面的深度
	float m_fVirtualEFNearPlane, m_fVirtualEFFarPlane;

	// 新旧深度缓冲和后台缓冲，保存状态和临时使用
	LPDIRECT3DSURFACE9 m_pOldBackBuffer, m_pOldDepthBuffer, m_pDepthBuffer;
	
	// 阴影贴图
	LPDIRECT3DTEXTURE9 m_pTexShadowMap[4];
};