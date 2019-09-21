#include "myd3d.h"

CAMERACHANGE::CAMERACHANGE()
{
	m_bInit = false;
	m_bYMove = false;
	Look = D3DXVECTOR3(0, 0, 1);
	Eye = D3DXVECTOR3(0, 0, 0);
	m_VecLook = D3DXVECTOR3(0, 0, 1);
	m_VecHead = D3DXVECTOR3(0, 1, 0);
	m_fProjXYCoef = 1.2f;
	m_fAngleSpeed = 0.0f;
	m_bIdle = false;
	m_MoveAttrib = 0;
}

void CAMERACHANGE::ViewTransform()
{
	D3DXMatrixLookAtLH(&myview, &Eye, &Look, &m_VecHead);
	d3ddevice->SetTransform(D3DTS_VIEW, &myview);
}

void CAMERACHANGE::ProjTransform(float xycoef)
{
	if(xycoef == 0)
		m_fProjXYCoef = (float)d3dpp.BackBufferWidth / (float)d3dpp.BackBufferHeight;
	else
		m_fProjXYCoef = xycoef;

	D3DXMatrixPerspectiveFovLH(&myproj, D3DX_PI/2, m_fProjXYCoef, 1.0f, 150.0f);
	d3ddevice->SetTransform(D3DTS_PROJECTION, &myproj);
}

HRESULT CAMERACHANGE::GetEyeFrustum(D3DXVECTOR3 *PtFrustum, float fNear, float fFar)
{
	if(!PtFrustum || !m_bInit || m_fProjXYCoef==0)
		return E_FAIL;

	float fTemp = 0;
	float fFovY = D3DX_PI/2;	// Y方向的夹角这里固定为D3DX_PI/2，不过注意若上面的ProjTransform函数中改变了，FovY的初始值也要改变，否则会出错

	// 第一个点是摄像机坐标
	PtFrustum[0] = Eye;

	// 在摄像机空间来根据View Frustum计算9个顶点，这样就会以0,0,0为中心（摄像机位置），Z正轴为视线方向，便于计算，最后就会把它按照视线向量和摄像机位置旋转、平移
	float fNearY = 0, fNearX = 0, fFarY = 0, fFarX = 0;
	fTemp = tanf(fFovY / 2);
	fNearY = fTemp * fNear;
	fFarY = fTemp * fFar;

	// X方向的fov就是tan(fovY) * Coef，不是tan(fovY * Coef)
	fTemp = tanf(fFovY / 2) * m_fProjXYCoef;
	fNearX = fTemp * fNear;
	fFarX = fTemp * fFar;
	
	// 接下来四个点是Near平面的四个顶点，左上右上左下右下
	D3DXVECTOR3 PtFrustums[9];
	PtFrustums[1] = D3DXVECTOR3(-fNearX, fNearY,  fNear);
	PtFrustums[2] = D3DXVECTOR3(fNearX,  fNearY,  fNear);
	PtFrustums[3] = D3DXVECTOR3(-fNearX, -fNearY, fNear);
	PtFrustums[4] = D3DXVECTOR3(fNearX,  -fNearY, fNear);

	// 最后四个点是Far平面的四个顶点，左上右上左下右下
	PtFrustums[5] = D3DXVECTOR3(-fFarX, fFarY,  fFar);
	PtFrustums[6] = D3DXVECTOR3(fFarX,  fFarY,  fFar);
	PtFrustums[7] = D3DXVECTOR3(-fFarX, -fFarY, fFar);
	PtFrustums[8] = D3DXVECTOR3(fFarX,  -fFarY, fFar);

	// 旋转和平移，由摄像机空间转换到世界空间
	D3DXMATRIX Mat, MatTemp;
	D3DXMatrixIdentity(&Mat);
	D3DXMatrixIdentity(&MatTemp);
	float fAngle = 0;

	//Test
	D3DXMatrixInverse(&Mat, NULL, &myview);
	/*
		// 根据在世界空间的视线向量与摄像机空间视线向量的夹角，来构造旋转矩阵，用来将摄像机空间Z正轴的视线向量旋转到世界空间正确的位置
		D3DXVECTOR3 VecView;
		D3DXVec3Normalize(&VecView, &m_VecLook);

		D3DXVECTOR3 VecViewCross;
		D3DXVec3Cross(&VecViewCross, &D3DXVECTOR3(0, 0, 1), &VecView);
		fAngle = D3DXVec3Dot(&VecView, &D3DXVECTOR3(0, 0, 1));
		if(fAngle <= -0.9999999f)
			fAngle = D3DX_PI;
		else
		fAngle = acosf(fAngle);
		//旋转函数是逆时针，这一步不需要了 fAngle = D3DX_PI*2 - fAngle;
		D3DXMatrixRotationAxis(&MatTemp, &VecViewCross, fAngle);

		Mat *= MatTemp;
		

		// 平移
		D3DXMatrixTranslation(&MatTemp, Eye.x, Eye.y, Eye.z);
		Mat *= MatTemp;

  		*VecLookAt = VecView;
*/
	// 根据矩阵转换后8个点
	D3DXVec3TransformCoordArray(PtFrustum+1, sizeof(D3DXVECTOR3), PtFrustums+1, sizeof(D3DXVECTOR3), &Mat, 8);

	return S_OK;
}

HRESULT CAMERACHANGE::InitCamera(float Eyex, float Eyey, float Eyez, float Lookx, float Looky, float Lookz, float Headx, float Heady, float Headz, float move, float rise, float angle)
{
	if(D3DXVec3Length(&D3DXVECTOR3(Lookx-Eyex, Looky-Eyey, Lookz-Eyez)) == 0)
		return D3DERR_INVALIDCALL;

	// 先检查Angle，如果是角度，就转成弧度
	if(angle > D3DX_PI*2)
		angle = angtoarg(angle);

/*
	float len3d=sqrtf((Looky-Eyey)*(Looky-Eyey)+(Lookz-Eyez)*(Lookz-Eyez)+(Lookx-Eyex)*(Lookx-Eyex));
	if(len3d==0) return;

	float len=sqrtf((Lookz-Eyez)*(Lookz-Eyez)+(Lookx-Eyex)*(Lookx-Eyex));
	if(len==0) {Angle_H=0; 
				if((Looky-Eyey)<0) {Angle_V=D3DX_PI/2; m_VecHead.y=-1;}
				else Angle_V=D3DX_PI*3/2;
				}
	//先计算Angle_H，只考虑XZ平面
	Angle_H=asinf((Lookx-Eyex)/len);
	//再计算Angle_V，首先计算视线在XZ平面上投影的长度，然后用Y/此长度即角度的正切
	Angle_V=atanf((Eyey-Looky)/len);
*/
	//初始化各项变量的值
	m_fMoveSpeed = move;
	m_fRiseSpeed = rise;
	m_fAngleSpeed = angle;

	// 初始化正交空间向量和VIEW矩阵
	Eye = D3DXVECTOR3(Eyex, Eyey, Eyez);
	D3DXVec3Normalize(&m_VecLook, &D3DXVECTOR3(Lookx-Eyex, Looky-Eyey, Lookz-Eyez));
	D3DXVec3Normalize(&m_VecHead, &D3DXVECTOR3(Headx, Heady, Headz));
	if(FAILED(GenerateViewSpace(&m_VecLook, &m_VecHead, &m_VecRight_X)))
		return D3DERR_INVALIDCALL;
	Look = Eye + m_VecLook;
	D3DXMatrixLookAtLH(&myview, &Eye, &Look, &m_VecHead);
	
	// 初始化时间
	dwCurrentTime = dwLastFrameTime = dwTempLastFrameTime = dwLastIdleTime = timeGetTime();
	
	m_bInit=TRUE;
	return S_OK;
}



void CAMERACHANGE::SetFrameTime()
{
	if(!m_bInit)
		return;
	dwTempLastFrameTime = dwLastFrameTime;
	dwCurrentTime = dwLastFrameTime = timeGetTime();
}

float CAMERACHANGE::GetIdleTime()
{
	if(!m_bIdle || !m_bInit)
		return 0.0f;
	return (float)(dwCurrentTime - dwLastIdleTime) / 1000;
}

float CAMERACHANGE::GetFrameTime()
{
	if(!m_bInit)
		return 0.0f;
	return (float)(dwCurrentTime-dwTempLastFrameTime) / 1000;
}






void CAMERACHANGE::ResetCamera()
{
	// 重置摄像机的目的是消除Slice和TurnUp/Down，也就是说要旋转三个轴使得HEAD与Y重合，Look和Right_X在XZ面上
	D3DXVECTOR3 VecProj, VecCross, VecTemp;
	D3DXMATRIX MatRotate;
	float fAngle = 0.0f;

	// 先计算Right_X在XZ面上的投影，旋转以消除Slice
	VecProj = D3DXVECTOR3(m_VecRight_X.x, 0, m_VecRight_X.z);
	D3DXVec3Normalize(&VecProj, &VecProj);
		// 计算角度
	fAngle = D3DXVec3Dot(&VecProj, &m_VecRight_X);
	fAngle = acosf(fAngle);
		// 如果夹角过小（很难看出Slice），就不改变当前设定，免得由于精度问题或肉眼无法分辨的小夹角导致微弱的晃动
	if(fAngle > 0.05f)
	{
		// 构造矩阵，绕Look轴旋转
		D3DXMatrixRotationAxis(&MatRotate, &m_VecLook, fAngle);
		// 变换剩下的两个轴
		VecTemp = m_VecRight_X;
		D3DXVec3TransformCoord(&m_VecRight_X, &VecTemp, &MatRotate);
		VecTemp = m_VecHead;
		D3DXVec3TransformCoord(&m_VecHead, &VecTemp, &MatRotate);
	}
	
	// 这里有个极技巧极偶然的东西，上面为什么不判断旋转轴？因为上面的语句必须先投影一次，所以无法判断旋转轴
	// Dot的左右方向是无法判断出来的，如果不用叉乘求旋转轴的话，在Head偏向某个方向时就会出问题，本来应该朝相反的方向转动，但它却向同方向转动，导致偏向更加严重
	// 但下面的语句又判断了Head，它不但会判断Turn的夹角，同时还会纠正Slice的偏差，而这时Right_X和Look轴已经和XZ面重合了，下面用了正确的旋转轴就可以算出正确的结果了



	// 已经转动到了XZ面上，再计算Head与Y的夹角，绕着Right_X转动以消除TurnUp/Down
		// 计算角度
	fAngle = D3DXVec3Dot(&D3DXVECTOR3(0, 1, 0), &m_VecHead);
	fAngle = acosf(fAngle);
		// 计算旋转轴，计算旋转轴是为了能搞定旋转的左右方向，它其实跟m_Look差不多，只是可能会为负轴，所以会自动根据两向量位置关系决定旋转的方向
	D3DXVec3Cross(&VecCross, &m_VecHead, &D3DXVECTOR3(0, 1, 0));
	D3DXVec3Normalize(&VecCross, &VecCross);
		// 如果夹角过小（很难看出TurnUp/Down），就不改变当前设定，免得由于精度问题或肉眼无法分辨的小夹角导致微弱的晃动
	if(fAngle > 0.05f)
	{
		// 构造矩阵
		D3DXMatrixRotationAxis(&MatRotate, &VecCross, fAngle);
		// 变换剩下的两个轴
		VecTemp = m_VecHead;
		D3DXVec3TransformCoord(&m_VecHead, &VecTemp, &MatRotate);
		VecTemp = m_VecLook;
		D3DXVec3TransformCoord(&m_VecLook, &VecTemp, &MatRotate);
	}


	// 计算LOOK点
	Look = Eye + m_VecLook;
}




// 从0到11位分别代表：前后移，左右移，上下移，左右转，上下转，左右摇动
// 三个轴向量初始就是规格化的，在这里只有旋转操作，所以不会影响它的长度
void CAMERACHANGE::ChangeCamera(unsigned short int changeway, float fMoveCoef)
{
	if(!m_bInit) return;   //没初始化就返回

	// 临时使用，根据是否超过最大旋转角度（Head只能转180度）来判断该旋转操作是否有效（不写入m_Vec）
	D3DXVECTOR3 VecHead = m_VecHead, VecLook = m_VecLook, VecRight_X = m_VecRight_X;

	
	// 如果未改变，那么就说明处于Idle状态
	if(changeway == CAM_NOCHANGE)
	{
		// 如果bIdle是false，说明刚到Idle状态，重置
		if(!m_bIdle)
		{
			m_bIdle = true;
			m_MoveAttrib = 3;	// 刚从Moving到Idle
			dwLastIdleTime = timeGetTime();
		}
		else
			m_MoveAttrib = 0;	// 一直Idle
	}
	else
	{
		if(m_bIdle)
			m_MoveAttrib = 2;	// 刚从Idle到Moving
		else
			m_MoveAttrib = 1;	// 一直Moving
		m_bIdle = false;
	}

	// 得到当前时间
	dwCurrentTime = timeGetTime();
	float fChangeRatio = (float)(dwCurrentTime-dwLastFrameTime) / 1000;
	// 将移动倍数加入（鼠标控制）
	fChangeRatio *= fMoveCoef;

	if(changeway&1)               //GOFORWARD
	{
		D3DXVECTOR3 VecChange;
		
		//这两行是确定Y的，就是完全沿着现在的视线走，不要它就只是在Y相同的一个平面上移动
		if(m_bYMove)
			VecChange = D3DXVECTOR3(m_VecLook.x, 0, m_VecLook.z);
		else
			VecChange = m_VecLook;
	    Eye += VecChange * fChangeRatio * m_fMoveSpeed;
	}

	else if(changeway>>1&1)         //GOBACKWARD
	{
		D3DXVECTOR3 VecChange;
		
		//这两行是确定Y的，就是完全沿着现在的视线走，不要它就只是在Y相同的一个平面上移动
		if(m_bYMove)
			VecChange = D3DXVECTOR3(m_VecLook.x, 0, m_VecLook.z);
		else
			VecChange = m_VecLook;
		Eye -= VecChange * fChangeRatio * m_fMoveSpeed;
	}

	
	else if(changeway>>2&1)      //左平移GOLEFT
	{
		D3DXVECTOR3 VecChange;
		
		//这两行是确定Y的，就是完全沿着现在的视线走，不要它就只是在Y相同的一个平面上移动
		if(m_bYMove)
			VecChange = D3DXVECTOR3(m_VecRight_X.x, 0, m_VecRight_X.z);
		else
			VecChange = m_VecRight_X;
		Eye -= VecChange * fChangeRatio * m_fMoveSpeed;
	}

	else if(changeway>>3&1)   //右平移GORIGHT
	{
		D3DXVECTOR3 VecChange;
		
		//这两行是确定Y的，就是完全沿着现在的视线走，不要它就只是在Y相同的一个平面上移动
		if(m_bYMove)
			VecChange = D3DXVECTOR3(m_VecRight_X.x, 0, m_VecRight_X.z);
		else
			VecChange = m_VecRight_X;
		Eye += VecChange * fChangeRatio * m_fMoveSpeed;
	}

	else if(changeway>>4&1)      //向上升GOUP
	{
		Eye.y += fChangeRatio * m_fRiseSpeed;
	}
	else if(changeway>>5&1)      //向下降GODOWN
	{
		Eye.y -= fChangeRatio * m_fRiseSpeed;
	}




	// 下面是3种类型的旋转，牵扯到三个坐标轴	
	else if(changeway>>6&1 || changeway>>7&1)  //TurnLeft || TurnRight
	{
		D3DXMATRIX MatRotate;
		D3DXVECTOR3 VecTemp;
		float fAngle = 0.0f;
		if(changeway>>6&1)
			fAngle += -1 * fChangeRatio * m_fAngleSpeed;
		if(changeway>>7&1)
			fAngle += fChangeRatio * m_fAngleSpeed;

		D3DXVECTOR3 Vec;
		// 如果忽略Y轴，整个观察坐标系的三个轴就整体在世界空间的XZ面上转动即可，注意不要漏掉Head轴哦
		if(!m_bYMove)
		{
			Vec = D3DXVECTOR3(0, 1, 0);
			D3DXMatrixRotationAxis(&MatRotate, &Vec, fAngle);
			VecTemp = m_VecLook;
			D3DXVec3TransformCoord(&VecLook, &VecTemp, &MatRotate);
			VecTemp = m_VecRight_X;
			D3DXVec3TransformCoord(&VecRight_X, &VecTemp, &MatRotate);
			VecTemp = m_VecHead;
			D3DXVec3TransformCoord(&VecHead, &VecTemp, &MatRotate);
		}
		// 不忽略的话就按正常情况在Right_X和Look组成的面上围绕Head轴转动
		else
		{
			Vec = m_VecHead;
			D3DXMatrixRotationAxis(&MatRotate, &Vec, fAngle);
			VecTemp = m_VecLook;
			D3DXVec3TransformCoord(&VecLook, &VecTemp, &MatRotate);
			VecTemp = m_VecRight_X;
			D3DXVec3TransformCoord(&VecRight_X, &VecTemp, &MatRotate);
		}
	}
	
	else if(changeway>>8&1 || changeway>>9&1)      //向上或向下转TURN(UP/DOWN)
	{
		D3DXMATRIX MatRotate;
		D3DXVECTOR3 VecTemp;
		float fAngle = 0.0f;
		if(changeway>>8&1)
			fAngle += -1 * fChangeRatio * m_fAngleSpeed;
		if(changeway>>9&1)
			fAngle += fChangeRatio * m_fAngleSpeed;
		
		D3DXMatrixRotationAxis(&MatRotate, &m_VecRight_X, fAngle);
		VecTemp = m_VecLook;
		D3DXVec3TransformCoord(&VecLook, &VecTemp, &MatRotate);
		VecTemp = m_VecHead;
		D3DXVec3TransformCoord(&VecHead, &VecTemp, &MatRotate);
	}

	else if(changeway>>10&1 || changeway>>11&1)      //向左或向右Slice转动Slice(UP/DOWN)
	{
		D3DXMATRIX MatRotate;
		D3DXVECTOR3 VecTemp;
		float fAngle = 0.0f;
		if(changeway>>10&1)
			fAngle += -1 * fChangeRatio * m_fAngleSpeed;
		if(changeway>>11&1)
			fAngle += fChangeRatio * m_fAngleSpeed;
		
		D3DXMatrixRotationAxis(&MatRotate, &m_VecLook, fAngle);
		VecTemp = m_VecRight_X;
		D3DXVec3TransformCoord(&VecRight_X, &VecTemp, &MatRotate);
		VecTemp = m_VecHead;
		D3DXVec3TransformCoord(&VecHead, &VecTemp, &MatRotate);
	}


	// 判断旋转操作是否有效，即VecHead只能在Y轴正半球，有效就写入m_Vec
	if(VecHead.y >= 0.0f)
	{
		m_VecHead = VecHead;
		m_VecLook = VecLook;
		m_VecRight_X = VecRight_X;
	}


	// 得到Look点的坐标
	Look = Eye + m_VecLook;
}








/*
	if(changeway&1)               //GOFORWARD
	{
	    Eye.x+=sinf(Angle_H)* fChangeRatio * MoveStep;
        Eye.z+=cosf(Angle_H)* fChangeRatio * MoveStep;

		Look.x+=sinf(Angle_H)* fChangeRatio * MoveStep;
        Look.z+=cosf(Angle_H)* fChangeRatio * MoveStep;

		//这两行是确定Y的，就是完全沿着现在的视线走，不要它就只是在Y相同的一个平面上移动
		if(Y_AxisTranslate)
		{
			Eye.y-=sinf(Angle_V)* fChangeRatio * MoveStep;
		    Look.y-=sinf(Angle_V)* fChangeRatio * MoveStep;
		}

	}

	else if(changeway>>1&1)         //GOBACKWARD
	{
	    Eye.x-=sinf(Angle_H)* fChangeRatio * MoveStep;
        Eye.z-=cosf(Angle_H)* fChangeRatio * MoveStep;

		Look.x-=sinf(Angle_H)* fChangeRatio * MoveStep;
        Look.z-=cosf(Angle_H)* fChangeRatio * MoveStep;

		//这两行是确定Y的，就是完全沿着现在的视线走，不要它就只是在Y相同的一个平面上移动
		if(Y_AxisTranslate)
		{
			Eye.y+=sinf(Angle_V)* fChangeRatio * MoveStep;
		    Look.y+=sinf(Angle_V)* fChangeRatio * MoveStep;
		}
	}

	
	else if(changeway>>2&1)      //左平移GOLEFT
	{
	    Eye.x-=cosf(Angle_H)* fChangeRatio * RiseStep;
        Eye.z+=sinf(Angle_H)* fChangeRatio * RiseStep;

		Look.x-=cosf(Angle_H)* fChangeRatio * RiseStep;
        Look.z+=sinf(Angle_H)* fChangeRatio * RiseStep;
	}

	else if(changeway>>3&1)   //右平移GORIGHT
	{
	    Eye.x+=cosf(Angle_H)* fChangeRatio * RiseStep;
        Eye.z-=sinf(Angle_H)* fChangeRatio * RiseStep;

		Look.x+=cosf(Angle_H)* fChangeRatio * RiseStep;
        Look.z-=sinf(Angle_H)* fChangeRatio * RiseStep;
	}

	else if(changeway>>4&1)      //向上升GOUP
	{
		Eye.y+= fChangeRatio * RiseStep;
		Look.y+= fChangeRatio * RiseStep;
	}
	else if(changeway>>5&1)      //向下降GODOWN
	{
		Eye.y-= fChangeRatio * RiseStep;
		Look.y-= fChangeRatio * RiseStep;
	}
	
	else if(changeway>>6&1)  //TURNLEFT
	{
		//len是视线在XZ面上投影的长度，使用LEN是在LOOKY不为0的时候保证XZ和Y值比例匹配，不至于在上下转动之后左右转就会出现突然跳跃的情况
		float len=sqrtf((Look.x-Eye.x)*(Look.x-Eye.x)+(Look.z-Eye.z)*(Look.z-Eye.z));  //XZ视线长度
		Angle_H -= (fChangeRatio * AngleStep);
		
			Look.x = Eye.x+sinf(Angle_H)*len;
			Look.z = Eye.z+cosf(Angle_H)*len;
	}
	
	else if(changeway>>7&1)  //TURNRIGHT
	{
		float len=sqrtf((Look.x-Eye.x)*(Look.x-Eye.x)+(Look.z-Eye.z)*(Look.z-Eye.z));  //XZ视线长度
		Angle_H += (fChangeRatio * AngleStep);

		Look.x = Eye.x+sinf(Angle_H)*len;
		Look.z = Eye.z+cosf(Angle_H)*len;
	}

	else if(changeway>>8&1||changeway>>9&1)      //向上或向下转TURN(UP/DOWN)
	{
		float var;  //在XZ平面上的投影
		//上转角度减，下转角度加
		if(changeway>>8&1) Angle_V -= (fChangeRatio * AngleStep);
		else Angle_V += (fChangeRatio * AngleStep);
        
		// Angle_V不能为负数！！sin和cos都是以PI为对称的，为负数的话三角函数值就会在半球之间来回震荡，如果为负数就求2PI的补，让它的值域扩展到2PI，在2PI之间循环而不是在PI之间震荡
		if(Angle_V < 0) Angle_V = D3DX_PI*2 + Angle_V;

		Look.y = Eye.y-sinf(Angle_V);

		var=cosf(Angle_V);
		Look.x=Eye.x+sinf(Angle_H)*var;
		Look.z=Eye.z+cosf(Angle_H)*var;
		//需要检查头顶向量，XZ平面下半球头顶Y值取反
		for(float temp=Angle_V;temp>=D3DX_PI*2;temp-=D3DX_PI*2);   //得到目前的值（大于360度的不算）
		if(temp>=D3DX_PI/2&&temp<D3DX_PI*3/2) m_VecHead.y=-1;
		else m_VecHead.y=1;
	}
*/



/*使用矩阵来变换，只是参考而已
//从0到9位分别代表：前后移，左右移，上下移，左右转，上下转
void CAMERACHANGE::ChangeCamera(unsigned short int changeway)
{
	if(IsInit==FALSE) return;   //没初始化就返回
	D3DXMATRIX Transform;

	if(changeway&1)               //GOFORWARD
	{
		D3DXMatrixTranslation(&Transform, 0, 0, -1*MoveStep);
	}

	if(changeway>>1&1)         //GOBACKWARD
	{
		D3DXMatrixTranslation(&Transform, 0, 0, MoveStep);
	}

	
	if(changeway>>2&1)      //左平移GOLEFT
	{
		D3DXMatrixTranslation(&Transform, cosf(Angle_H)*RiseStep, 0, -1*sinf(Angle_H)*RiseStep);
	}

	if(changeway>>3&1)   //右平移GORIGHT
	{
		D3DXMatrixTranslation(&Transform, -1*cosf(Angle_H)*RiseStep, 0, sinf(Angle_H)*RiseStep);
	}

	if(changeway>>4&1)      //向上升GOUP
	{
		D3DXMatrixTranslation(&Transform, 0, -1*RiseStep, 0);
	}
	if(changeway>>5&1)      //向下降GODOWN
	{
		D3DXMatrixTranslation(&Transform, 0, RiseStep, 0);
	}
	
	if(changeway>>6&1)  //TURNLEFT
	{
		D3DXMatrixRotationY(&Transform, AngleStep);
	}
	
	if(changeway>>7&1)  //TURNRIGHT
	{
        D3DXMatrixRotationY(&Transform, -1*AngleStep);
	}

	if(changeway>>8&1)      //向上转TURNUP
	{
		D3DXVECTOR3 axis;
		float tg,arg;
		D3DXQUATERNION Qtn;
		
		//求出视线在XZ面上的投影向量和X正轴的夹角正切
		tg=(Look.z-Eye.z)/(Look.x-Eye.x);
		//求出轴向量和X正轴的夹角
		arg=atanf(tg)+D3DX_PI/2;
		//假设轴向量的长度为1,求出轴向量
		axis=D3DXVECTOR3(cosf(arg), 0, sinf(arg));
		D3DXQuaternionRotationAxis(&Qtn, &axis, -1*AngleStep);
		D3DXMatrixAffineTransformation(&Transform, 1, &D3DXVECTOR3(Eye.x,Eye.y,Eye.z), &Qtn,NULL);
	}

	if(changeway>>9&1)     //向下转TURNDOWN
	{
		D3DXVECTOR3 axis;
		float tg,arg;
		D3DXQUATERNION Qtn;
		
		//求出视线在XZ面上的投影向量和X正轴的夹角正切
		tg=(Look.z-Eye.z)/(Look.x-Eye.x);
		//求出轴向量和X正轴的夹角
		arg=atanf(tg)+D3DX_PI/2;
		//假设轴向量的长度为1,求出轴向量
		axis=D3DXVECTOR3(cosf(arg), 0, sinf(arg));
		D3DXQuaternionRotationAxis(&Qtn, &axis, AngleStep);
		D3DXMatrixAffineTransformation(&Transform, 1, &D3DXVECTOR3(Eye.x,Eye.y,Eye.z), &Qtn,NULL);
	}
	
	D3DXMatrixMultiply(&myview, &myview, &Transform);
}
*/

//需要说明的是上转、下转：把Y轴和视线在XZ平面上的投影线组成一个平面，即投影线的角度为0，投影线向下方转为正角度
//另外就是所有的旋转坐标确定方法：已知目前的视线和需要转的角度，如何确定新的LOOK点坐标呢？因为EYE不变，所以将EYE的对应轴坐标值加上需要转的角度在对应平面上的投影长度即可，具体分析程序。
//不要管LOOK坐标现在是多少，总是通过EYE和两个角度来确定LOOK坐标（向量长度假设为1就行了）




















/************************Trapezoidal Shadow Map****************************/
KTrapezoidalShadowMap::KTrapezoidalShadowMap()
{
	m_iCreateAttrib = 0;
	m_fVirtualEFNearPlane = 2.0f;
	m_fVirtualEFFarPlane = 200.0f;
	m_bForceUSM = false;
	m_bFourShadowMaps = false;
	m_PtLightPosition = D3DXVECTOR3(0, 0, 0);
	m_VecLightDirection = D3DXVECTOR3(0, 0, 1);
	m_VecHeadDirection = D3DXVECTOR3(0, 1, 0);
	m_VecRightDirection = D3DXVECTOR3(1, 0, 0);
	m_LightType = D3DLIGHT_SPOT;
	for(int i=0; i<4; i++)
		m_pTexShadowMap[i] = NULL;
	m_pOldBackBuffer = NULL;
	m_pOldDepthBuffer = NULL;
	m_pDepthBuffer = NULL;
}

KTrapezoidalShadowMap::~KTrapezoidalShadowMap()
{
	Release();
}

void KTrapezoidalShadowMap::Release()
{
	m_iCreateAttrib = 0;
	m_bForceUSM = false;
	m_bFourShadowMaps = false;
	m_PtLightPosition = D3DXVECTOR3(0, 0, 0);
	m_VecLightDirection = D3DXVECTOR3(0, 0, 1);
	m_VecHeadDirection = D3DXVECTOR3(0, 1, 0);
	m_VecRightDirection = D3DXVECTOR3(1, 0, 0);
	m_LightType = D3DLIGHT_SPOT;
	for(int i=0; i<4; i++)
		SAFE_RELEASE(m_pTexShadowMap[i]);
	SAFE_RELEASE(m_pOldBackBuffer);
	SAFE_RELEASE(m_pOldDepthBuffer);
	SAFE_RELEASE(m_pDepthBuffer);
}


HRESULT KTrapezoidalShadowMap::RenderShadowTexture(UINT iShadowMapNo /* = 0 */)
{
	if(!m_pTexShadowMap || !m_iCreateAttrib)
		return E_FAIL;

	if(FAILED(d3ddevice->GetRenderTarget(0, &m_pOldBackBuffer)))
		return E_FAIL;
	if(FAILED(d3ddevice->GetDepthStencilSurface(&m_pOldDepthBuffer)))
		return E_FAIL;

	// 直接设置新的，初始化的时候，新旧两套都已经准备好了
	V_RETURN(SetTexturedRenderTarget(0, m_pTexShadowMap[iShadowMapNo], m_pDepthBuffer));

	d3ddevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0);

	m_iCreateAttrib = 2;
	
	return S_OK;
}

HRESULT KTrapezoidalShadowMap::RestoreRenderTarget()
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

HRESULT KTrapezoidalShadowMap::SetShadowTexture(DWORD dwStage)
{
	if(dwStage > 6)
		return D3DERR_INVALIDCALL;
	if(!m_pTexShadowMap || m_iCreateAttrib < 2)
		return E_FAIL;
	
	for(int i=0; i<4; i++)
		d3ddevice->SetTexture(dwStage+i, m_pTexShadowMap[i]);
	
	return S_OK;
}

HRESULT KTrapezoidalShadowMap::SetLight(D3DXVECTOR3 PtLightPos, D3DXVECTOR3 VecLightDirection, D3DXVECTOR3 VecHeadDirection, float fMinRange /* = 1.0f */, float fMaxRange /* = 500.0f */, D3DLIGHTTYPE LightType /* = D3DLIGHT_DIRECTIONAL */, float fXRangeCoef /* = 1.0f */, float fYRangeCoef /* = 1.0f */)
{
	if(fMinRange <= 0 || fMinRange >= fMaxRange || D3DXVec3Length(&VecLightDirection) == 0.0f || D3DXVec3Length(&VecHeadDirection) == 0.0f || VecLightDirection == VecHeadDirection)
		return D3DERR_INVALIDCALL;
	
	// 初始化光源位置和光源方向
	m_PtLightPosition = PtLightPos;
	m_VecLightDirection = VecLightDirection;
	m_VecHeadDirection = VecHeadDirection;
	if(FAILED(GenerateViewSpace(&m_VecLightDirection, &m_VecHeadDirection, &m_VecRightDirection)))
		return D3DERR_INVALIDCALL;
	
	// 初始化VIEW矩阵
	D3DXMatrixLookAtLH(&m_MatLVPView, &m_PtLightPosition, &(m_PtLightPosition+m_VecLightDirection), &m_VecHeadDirection);
	
	// 根据光源类型初始化投影矩阵
	if(LightType == D3DLIGHT_SPOT)
	{
		if(fXRangeCoef <=0 || fXRangeCoef*fYRangeCoef >=2.0f || fYRangeCoef >=2.0f || fYRangeCoef <=0)
			return D3DERR_INVALIDCALL;

		// LVP空间为了使用Omni，绝对要保证fovx = fovy = 90degree
		D3DXMatrixPerspectiveFovLH(&m_MatLVPProj, D3DX_PI/2 * fYRangeCoef, 1.0f * fXRangeCoef, fMinRange, fMaxRange);
	}
	else if(LightType == D3DLIGHT_DIRECTIONAL)
	{
		if(fXRangeCoef <=0 || fYRangeCoef <=0)
			return D3DERR_INVALIDCALL;

		// 方向光没有Omni，无所谓XY投影的比例
		D3DXMatrixOrthoLH(&m_MatLVPProj, 2.0f * fXRangeCoef, 2.0f * fYRangeCoef, fMinRange, fMaxRange);
	}
	
	m_LightType = LightType;
	
	return S_OK;
}


HRESULT KTrapezoidalShadowMap::SetFrustumRange(float fNear, float fFar)
{
	if(fNear <= 0.0f || fFar <= 0.0f || fFar <= fNear)
		return D3DERR_INVALIDCALL;
	
	m_fVirtualEFNearPlane = fNear;
	m_fVirtualEFFarPlane = fFar;
	return S_OK;
}



HRESULT KTrapezoidalShadowMap::GetMatrix(D3DXMATRIX *pMatLVPProj, D3DXMATRIX pMatTSpace[4])
{
	if(!pMatLVPProj && !pMatTSpace)
		return D3DERR_INVALIDCALL;

	if(pMatLVPProj)
		*pMatLVPProj = m_MatLVPView * m_MatLVPProj;

	D3DXMATRIX Mat[4];
	if(*pMatTSpace)
	{
		if(FAILED(GetTSpaceMatrix(Mat)))
			return E_FAIL;
		if(!m_bFourShadowMaps)
			*pMatTSpace = m_MatLVPView * m_MatLVPProj * Mat[0];
		else
		{
			for(int i=0; i<4; i++)
				*(pMatTSpace+i) = m_MatLVPView * m_MatLVPProj * Mat[i];
		}
	}
	
	return S_OK;
}


HRESULT KTrapezoidalShadowMap::Init(UINT iWidth /* = 512 */, UINT iHeight /* = 0 */)
{
	if(m_iCreateAttrib)
		return S_OK;
	
	D3DSURFACE_DESC Desc;

	if(iHeight == 0)
		iHeight = iWidth;

	// 初始化阴影贴图
	D3DFORMAT Format = D3DFMT_A8R8G8B8;

#ifdef USE_FP16
	Format = D3DFMT_G16R16F;
#elif defined USE_FP32
	Format = D3DFMT_R32F;
#else
	Format = D3DFMT_A8R8G8B8;
#endif
	
	for(int i=0; i<4; i++)
		if(FAILED(D3DXCreateTexture(d3ddevice, iWidth, iHeight, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &m_pTexShadowMap[i])))
			return E_FAIL;
	

	// 初始化新的深度缓冲（临时使用），同时得到旧的深度缓冲
	if(FAILED(d3ddevice->GetDepthStencilSurface(&m_pOldDepthBuffer)))
		return E_FAIL;
	if(FAILED(m_pOldDepthBuffer->GetDesc(&Desc)))
		return E_FAIL;
	SAFE_RELEASE(m_pOldDepthBuffer);

	if(FAILED(d3ddevice->CreateDepthStencilSurface(iWidth, iHeight, Desc.Format, Desc.MultiSampleType, Desc.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL)))
		return E_FAIL;


	
	// 初始化默认光源
	if(FAILED(SetLight(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 1), D3DXVECTOR3(0, 1, 0))))
		return D3DERR_INVALIDCALL;
	// 初始化默认EF
	SetFrustumRange(2.0f, 200.0f);
	
	
	m_iCreateAttrib = 1;
	return S_OK;
}






/***************************内部使用*************************/
HRESULT KTrapezoidalShadowMap::GetTSpaceMatrix(D3DXMATRIX *pMatTSpace)
{
	// 特殊情况下可能会有四个梯形，所以用5*4
	D3DXVECTOR3 PtFrustum[9], PtTrapezoid[5*4];
	if(!pMatTSpace || D3DXVec3Length(&m_VecLightDirection) == 0)
		return D3DERR_INVALIDCALL;

	D3DXVECTOR3 VecLookAt;
	if(FAILED(CameraChange.GetEyeFrustum(PtFrustum, m_fVirtualEFNearPlane, m_fVirtualEFFarPlane)))
		return E_FAIL;
	ViewFrustumToLVPSpace(PtFrustum);
	
	// 经过上面那一步，已经可以确定是强制USM还是强制4 ShadowMaps了，下来这一步就根据标记来置1个梯形或4个梯形的数据
	if(FAILED(FrustumToTrapezoid(PtFrustum, PtTrapezoid)))
		return E_FAIL;
	
	// 正常情况
	if(!m_bFourShadowMaps)
		*pMatTSpace = TransformToTrapezoidSpace(PtTrapezoid[1], PtTrapezoid[2], PtTrapezoid[3], PtTrapezoid[4], PtTrapezoid[0]);
	else
	{
		// 4 ShadowMaps，必须预先传入4个矩阵数组的指针，否则会内存越界
		for(int i=0; i<4; i++)
			pMatTSpace[i] = TransformToTrapezoidSpace(PtTrapezoid[1+i*5], PtTrapezoid[2+i*5], PtTrapezoid[3+i*5], PtTrapezoid[4+i*5], PtTrapezoid[0+i*5]);
	}

	return S_OK;
}



void KTrapezoidalShadowMap::ViewFrustumToLVPSpace(D3DXVECTOR3 *PtFrustum)
{
	D3DXMATRIX MatProj, MatView;
	
	// 为了避免Transform时失败，这里先复制一份
	int i = 0;
	D3DXVECTOR3 PtFrustums[9];
	for(i=0; i<9; i++)
		PtFrustums[i] = PtFrustum[i];

	D3DXMATRIX Mat = m_MatLVPView * m_MatLVPProj;
	
	D3DXVECTOR4 PtFrustumsVec4[9];
	D3DXVec3TransformArray(PtFrustumsVec4, sizeof(D3DXVECTOR4), PtFrustums, sizeof(D3DXVECTOR3), &Mat, 9);
	

	// 下面是重头戏，直接关系到TSM的可用性，它的重要性又如滔滔江水。。。
	// 检测两种错误条件，强制USM或4ShadowMaps^_^
	m_bForceUSM = false;
	m_bFourShadowMaps = false;

	// 千万记得投影的时候可能会有Frustum的某些点Z值（即投影变换后的W值）在LVP空间的变成负数（就是在LVP空间该点跑到光源后面去了），因为我们只是求投影，并不牵扯剪裁，所以强行设置为将近无穷远
	// 即xy为正的变成正无穷，为负的变为负无穷，但不能设置为0，因为0会让xy为负值的也变为正无穷
	for(i=0; i<9; i++)
		if(PtFrustumsVec4[i].w < 0)
		{
			PtFrustumsVec4[i].w = 0.00000001f;
			// 其实这里不用管它的w值。。。强制USM即可
			m_bForceUSM = true;
			return;
		}

	for(i=0; i<9; i++)
		*(PtFrustum+i) = D3DXVECTOR3(PtFrustumsVec4[i].x/PtFrustumsVec4[i].w, PtFrustumsVec4[i].y/PtFrustumsVec4[i].w, PtFrustumsVec4[i].z/PtFrustumsVec4[i].w);

	// 如果Frustum某些点越过Far Plane或左右边界（即<-1或>1），也可以强制USM
	
	// 判断如果光方向接近平行于视线方向，即Intersect点在Near或Far内，强制USM
		D3DXVECTOR3 PtTemp[9], PtMin, PtMax;
		for(i=0; i<9; i++)
		{
			PtTemp[i] = PtFrustum[i];
			PtTemp[i].z = 0;
		}

/*		// 先在4个点中求出左上角和右下角（即MAX X/Y  Min X/Y），即简易的通过BoundingBox求是否在多边形内
		D3DXComputeBoundingBox(PtTemp+1, 4, sizeof(D3DXVECTOR3), &PtMin, &PtMax);
		if(PtTemp[0].x >= PtMin.x && PtTemp[0].y >= PtMin.y && PtTemp[0].x <= PtMax.x && PtTemp[0].y <= PtMax.y)
		{
			m_bForceUSM = true;
			return;
		}
		D3DXComputeBoundingBox(PtTemp+5, 4, sizeof(D3DXVECTOR3), &PtMin, &PtMax);
		if(PtTemp[0].x >= PtMin.x && PtTemp[0].y >= PtMin.y && PtTemp[0].x <= PtMax.x && PtTemp[0].y <= PtMax.y)
		{
			m_bForceUSM = true;
			return;
		}
*/
		// 精确的求法：四边形由两个三角形组成，求点是否在两个三角形内，不过这个方法需要确定四个点的左右位置关系，从左到右依次是1、2、3一个三角形，2、3、4一个三角形，这样才能正确
		D3DXVECTOR3 PtQuad[4], PtSwap;
		int j = 0;

		// 处理Far Plane，如果EYE在里面就强制4 ShadowMaps
		// 为什么要Far优先呢？因为如果EYE在Far中，则Near也必定在Far中
		for(i=5; i<9; i++)
			PtQuad[i-5] = PtTemp[i];
			// 冒泡法排序，按左右顺序排，即按X坐标，Z都为0,不必担心
		for(j=0; j<3; j++)
		{
			for(i=j+1; i<4; i++)
				if(PtQuad[i].x < PtQuad[j].x)
				{
					PtSwap = PtQuad[i];
					PtQuad[i] = PtQuad[j];
					PtQuad[j] = PtSwap;
				}
		}
		// 分割成两个三角形，分别求点是否在它们之内，若有一个成立就强制USM
		if(IsPointInTriangle(PtTemp[0], PtQuad[0], PtQuad[1], PtQuad[2]) || IsPointInTriangle(PtTemp[0], PtQuad[1], PtQuad[2], PtQuad[3]))
		{
			m_bFourShadowMaps = true;
			return;
		}

		// 处理Near Plane，如果EYE在里面就强制USM
		for(i=1; i<5; i++)
			PtQuad[i-1] = PtTemp[i];
		// 冒泡法排序，按左右顺序排，即按X坐标，Z都为0,不必担心
		for(j=0; j<3; j++)
		{
			for(i=j+1; i<4; i++)
				if(PtQuad[i].x < PtQuad[j].x)
				{
					PtSwap = PtQuad[i];
					PtQuad[i] = PtQuad[j];
					PtQuad[j] = PtSwap;
				}
		}
		// 分割成两个三角形，分别求点是否在它们之内，若有一个成立就强制USM
		if(IsPointInTriangle(PtTemp[0], PtQuad[0], PtQuad[1], PtQuad[2]) || IsPointInTriangle(PtTemp[0], PtQuad[1], PtQuad[2], PtQuad[3]))
		{
			m_bForceUSM = true;
			return;
		}
}



// 传进来的参数代表的是LVP空间的坐标（只有2D有效），Z都为0，传回的是将该梯形从LVP变换到它所在T-SPACE的矩阵
D3DXMATRIX KTrapezoidalShadowMap::TransformToTrapezoidSpace(D3DXVECTOR3 PtTopL, D3DXVECTOR3 PtTopR, D3DXVECTOR3 PtBottomL, D3DXVECTOR3 PtBottomR, D3DXVECTOR3 PtIntersect)
{
	D3DXMATRIX MatFinal, MatStep;
	D3DXVECTOR3 PtTemp1, PtTemp2;
	float fAngle = 0;
	D3DXMatrixIdentity(&MatFinal);
	
	// 两种错误情况，强制USM，不过若是4 ShadowMaps的状态，强制USM就失效，因为它优先级要低，将Far Plane直接作为梯形传入可以进一步提高阴影质量，尤其是方向光平行时
	if(m_bForceUSM && !m_bFourShadowMaps)
		return MatFinal;

	// TSM Demo中使用的，如果按回车就强制切换到USM
	if(!effectenable)
		return MatFinal;

	// 第一步，将梯形Top边的中点移到LVP中心，为了绕中心旋转，必须这么做
	D3DXMatrixIdentity(&MatStep);
	
	// Temp1就是中点，本来应该是(R-L) / 2 + L的，约去就得(R+L)/2
	PtTemp1 = (PtTopL + PtTopR) / 2.0f;
	// (0,0) - Temp1就是位移向量
	D3DXMatrixTranslation(&MatStep, -PtTemp1.x, -PtTemp1.y, 0);
	
	MatFinal *= MatStep;


	// 第二步，将Top边旋转到和x轴重合，并且将梯形倒过来，即Top在Bottom的下方
	D3DXMatrixIdentity(&MatStep);
	
	// 计算Top边与X轴的夹角（绝对值）
	D3DXVec3Normalize( &PtTemp1, &(PtTopR-PtTopL) );
	// 根据Top边向量的方向来计算角度（因为上面算出来的夹角是绝对值），然后构造旋转矩阵，在XY面上转动，当然是绕Z轴咯～
	fAngle = D3DXVec3Dot(&PtTemp1, &D3DXVECTOR3(1, 0, 0));
	if(fAngle <= -0.9999999f)
		fAngle = D3DX_PI;
	else
	fAngle = acosf(fAngle);
	if(PtTemp1.y < 0)
		D3DXMatrixRotationZ(&MatStep, fAngle + D3DX_PI);
	else
		D3DXMatrixRotationZ(&MatStep, -fAngle + D3DX_PI);
	
	MatFinal *= MatStep;
	
	// 第三步，移动梯形，让前交点和T-SPACE中心重合（Top的中点可能又会不在X轴上了，郁闷，当初还非要先移到X轴上）
	D3DXMatrixIdentity(&MatStep);
	
	// 计算前交点和中心的距离
	D3DXVec3TransformCoord(&PtTemp1, &PtIntersect, &MatFinal);
	PtTemp1 = -PtTemp1;
	// 构造矩阵
	D3DXMatrixTranslation(&MatStep, PtTemp1.x, PtTemp1.y, 0);
	
	MatFinal *= MatStep;
	
	// 第四步，变成等腰梯形且以Y轴对称（错切），就是说把中线扶正，嘿嘿，下面要放大，所以这里必须让它沿Y对称
	// 要计算两条边都相同的变量：中线倾斜度（tg），这样矩阵转换的时候会乘y，这个值就是x轴的delta
	D3DXMatrixIdentity(&MatStep);
	
		// 以Bottom为例，先计算Top的中点，当然也可以计算Top，因为中线倾斜度都是一样的，但最好用Bottom，因为Bottom比Top要大得多，Top因为太小可能会有产生误差
		D3DXVec3TransformCoord(&PtTemp1, &PtTopL, &MatFinal);
		D3DXVec3TransformCoord(&PtTemp2, &PtTopR, &MatFinal);
		PtTemp1 = (PtTemp2 + PtTemp1) / 2;
		// 计算倾斜度tan, fAngle
		fAngle = PtTemp1.x / PtTemp1.y;
		// 构造错切矩阵，这样的话yzw都不变，x = x - fAngle*y
		MatStep._21 = -fAngle;
	
	MatFinal *= MatStep;
	
	// 第五步，将沿Y轴对称的等腰梯形放大到top（在下方）和LVP空间的上底重合，Bottom不用关心
	D3DXMatrixIdentity(&MatStep);
	
		// 找TopRight和1,1的差距，因为是Y轴对称的，用Left点也可以，当然要换成和-1,1的差距咯
		D3DXVec3TransformCoord(&PtTemp1, &PtTopR, &MatFinal);
		PtTemp1.x = 1 / PtTemp1.x;
		PtTemp1.y = 1 / PtTemp1.y;
		// 构造缩放矩阵
		D3DXMatrixScaling(&MatStep, PtTemp1.x, PtTemp1.y, 1);		
	
	MatFinal *= MatStep;

	// 第六步，最神秘的一步，将梯形变成矩形，它将翻转top边到上面来，同时它和下面几步都有关系，尤其是最后一步的Homorous Divid
	D3DXMatrixIdentity(&MatStep);
	MatStep._44 = 0;
	MatStep._42 = MatStep._24 = 1; 
	
		// 
	
	MatFinal *= MatStep;


	// 第七步，将矩形（top在上bottom在下）平移到LVP中心（只是对Y操作）
	D3DXMatrixIdentity(&MatStep);

		// 得到当前矩形中心的坐标
		D3DXVec3TransformCoord(&PtTemp1, &PtTopL, &MatFinal);
		D3DXVec3TransformCoord(&PtTemp2, &PtBottomR, &MatFinal);
		PtTemp1 = (PtTemp1 + PtTemp2) / 2;
		// 构造矩阵，只平移Y，向下平移，所以是减
		D3DXMatrixTranslation(&MatStep, 0, -PtTemp1.y, 0);
	
	MatFinal *= MatStep;


	// 第八步，将已处在中心的对称矩形沿Y轴拉伸使得充满整个LVP空间，T-SPACE完全构造ok，这时的T-SPACE就等于LVP空间咯～
	D3DXMatrixIdentity(&MatStep);
	
		// 得到拉伸比例系数，反正是沿X轴对称的，以一条边为准就行了，这里以top为准
		D3DXVec3TransformCoord(&PtTemp1, &PtTopL, &MatFinal);
		// 构造缩放矩阵，只放大Y轴
		D3DXMatrixScaling(&MatStep, 1, 1 / PtTemp1.y, 1);
			
	MatFinal *= MatStep;

	
	return MatFinal;
}






// 把LVP空间的View Frustum转换成梯形，即找到最小的梯形包围区域（纯2D操作），用于在后面构建LVP到T-SPACE(EYE SPACE相关)的转换矩阵
// 输入9个VEC3，代表Frustum在LVP的坐标，0是摄像机；1～4是Near Plane左上、右上、左下、右下；5～8是Far Plane
// 输出5个VEC3，代表梯形在LVP的2D坐标，0是前交点Intersect，1～4是TopLeft TopRight BottomLeft BottomRight
HRESULT KTrapezoidalShadowMap::FrustumToTrapezoid(D3DXVECTOR3 *pPtFrustum, D3DXVECTOR3 *pPtTrapezoid)
{
	if(!pPtFrustum)
		return D3DERR_INVALIDCALL;
	
	int i = 0;
	
	// 如果LVP EYE进入Far Plane，就置四个梯形
	if(m_bFourShadowMaps)
	{
		// 先检查是否刚进入Far Plane，这个时候Intersect到Far Plane的四条边可能和Far两底接近平行，直接用4个梯形会有问题滴～
		D3DXVECTOR3 VecBottom = pPtFrustum[7] - pPtFrustum[8];
		D3DXVECTOR3 VecTop = pPtFrustum[5] - pPtFrustum[6];
		D3DXVECTOR3 VecSideLine1 = pPtFrustum[0] - pPtFrustum[5];
		D3DXVECTOR3 VecSideLine2 = pPtFrustum[0] - pPtFrustum[6];
		D3DXVECTOR3 VecSideLine3 = pPtFrustum[0] - pPtFrustum[7];
		D3DXVECTOR3 VecSideLine4 = pPtFrustum[0] - pPtFrustum[8];
		D3DXVec3Normalize(&VecBottom, &VecBottom);
		D3DXVec3Normalize(&VecTop, &VecTop);
		D3DXVec3Normalize(&VecSideLine1, &VecSideLine1);
		D3DXVec3Normalize(&VecSideLine2, &VecSideLine2);
		D3DXVec3Normalize(&VecSideLine3, &VecSideLine3);
		D3DXVec3Normalize(&VecSideLine4, &VecSideLine4);
		float fFovBottom1 = absf(D3DXVec3Dot(&VecBottom, &VecSideLine1));
		float fFovBottom2 = absf(D3DXVec3Dot(&VecBottom, &VecSideLine2));
		float fFovBottom3 = absf(D3DXVec3Dot(&VecBottom, &VecSideLine3));
		float fFovBottom4 = absf(D3DXVec3Dot(&VecBottom, &VecSideLine4));
		float fFovTop1 = absf(D3DXVec3Dot(&VecTop, &VecSideLine1));
		float fFovTop2 = absf(D3DXVec3Dot(&VecTop, &VecSideLine2));
		float fFovTop3 = absf(D3DXVec3Dot(&VecTop, &VecSideLine3));
		float fFovTop4 = absf(D3DXVec3Dot(&VecTop, &VecSideLine4));
		
		float fClampValue = 0.07f;		// 这个值一定要选合适，小了就没有效果，大了又会把本来可以正常用4SM的地方改成USM
		if(absf(1-fFovBottom1) < fClampValue || absf(1-fFovBottom2) < fClampValue || absf(1-fFovBottom3) < fClampValue || absf(1-fFovBottom4) < fClampValue
			|| absf(1-fFovTop1) < fClampValue || absf(1-fFovTop2) < fClampValue || absf(1-fFovTop3) < fClampValue || absf(1-fFovTop4) < fClampValue)
		{
			// 先求Far Plane侧边的交点作为梯形交点
			if(!GetIntersect2D(pPtTrapezoid, pPtFrustum[5], pPtFrustum[7], pPtFrustum[6], pPtFrustum[8]))
			{
				// 求交点失败，强制用USM
				m_bForceUSM = true;
			}
			else
			{
				// 把Far Plane作为梯形
				for(i=5; i<9; i++)
					pPtTrapezoid[i-4] = pPtFrustum[i];
			}

			m_bFourShadowMaps = false;
			return S_OK;
		}


		// 不符合特殊的错误修正条件，使用正常的4 ShadowMaps
		// 第一个梯形，向上
		i = 0;
		GetIntersect2D(&pPtTrapezoid[0+i*5], pPtFrustum[3], pPtFrustum[5], pPtFrustum[4], pPtFrustum[6]);
		pPtTrapezoid[1+i*5] = pPtFrustum[3];
		pPtTrapezoid[2+i*5] = pPtFrustum[4];
		pPtTrapezoid[3+i*5] = pPtFrustum[5];
		pPtTrapezoid[4+i*5] = pPtFrustum[6];
		
		// 第二个梯形，向左
		i++;
		GetIntersect2D(&pPtTrapezoid[0+i*5], pPtFrustum[4], pPtFrustum[7], pPtFrustum[2], pPtFrustum[5]);
		pPtTrapezoid[1+i*5] = pPtFrustum[4];
		pPtTrapezoid[2+i*5] = pPtFrustum[2];
		pPtTrapezoid[3+i*5] = pPtFrustum[7];
		pPtTrapezoid[4+i*5] = pPtFrustum[5];
		
		// 第三个梯形，向下
		i++;
		GetIntersect2D(&pPtTrapezoid[0+i*5], pPtFrustum[2], pPtFrustum[8], pPtFrustum[1], pPtFrustum[7]);
		pPtTrapezoid[1+i*5] = pPtFrustum[2];
		pPtTrapezoid[2+i*5] = pPtFrustum[1];
		pPtTrapezoid[3+i*5] = pPtFrustum[8];
		pPtTrapezoid[4+i*5] = pPtFrustum[7];

		// 第四个梯形，向右
		i++;
		GetIntersect2D(&pPtTrapezoid[0+i*5], pPtFrustum[1], pPtFrustum[6], pPtFrustum[3], pPtFrustum[8]);
		pPtTrapezoid[1+i*5] = pPtFrustum[1];
		pPtTrapezoid[2+i*5] = pPtFrustum[3];
		pPtTrapezoid[3+i*5] = pPtFrustum[6];
		pPtTrapezoid[4+i*5] = pPtFrustum[8];
		return S_OK;
	}



	D3DXMATRIX MatFinal, MatStep;
	D3DXVECTOR3 PtFrustums[9], PtFrustum[9], PtTrapezoid[5];
	D3DXVECTOR2 PtMidLines[2], PtMidLine[2];
	D3DXMatrixIdentity(&MatFinal);
	for(i=0; i<9; i++)
	{
		PtFrustums[i] = pPtFrustum[i];
		PtFrustums[i].z = 0;
	}

	// 第一步，计算Frustum的中线（底Far指向顶Near）及其中点，根据中点将Frustum平移到LVP中央，并旋转它使得中线朝向LVP正Y轴，这一步是为了方便计算Top和Bottom，因为要中线垂直X轴才比较好算梯形平行的底边嘛
	D3DXMatrixIdentity(&MatStep);

		D3DXVECTOR3 PtTopMidPoint = (PtFrustums[1] + PtFrustums[2] + PtFrustums[3] + PtFrustums[4]) / 4.0f;
		D3DXVECTOR3 PtBottomMidPoint = (PtFrustums[5] + PtFrustums[6] + PtFrustums[7] + PtFrustums[8]) / 4.0f;
		D3DXVECTOR3 PtMidLineMidPoint = (PtTopMidPoint + PtBottomMidPoint) / 2.0f;

		// 构造平移矩阵
		D3DXMatrixTranslation(&MatStep, -PtMidLineMidPoint.x, -PtMidLineMidPoint.y, 0);
		MatFinal *= MatStep;

		// 构造旋转矩阵
		D3DXMatrixIdentity(&MatStep);

		D3DXVECTOR3 VecMidLine = PtTopMidPoint - PtBottomMidPoint;
		D3DXVec3Normalize(&VecMidLine, &VecMidLine);
		float fAngle = D3DXVec3Dot(&VecMidLine, &D3DXVECTOR3(0, 1, 0));
		if(fAngle <= -0.9999999f)
			fAngle = D3DX_PI;
		else
		fAngle = acosf(fAngle);

		if(VecMidLine.x >= 0)
			D3DXMatrixRotationZ(&MatStep, fAngle);
		else
			D3DXMatrixRotationZ(&MatStep, -fAngle);

		// 变换Frustum的9个点，使得Far Plane在下，Near Plane在上，符合我们平时的习惯嘛
		MatFinal *= MatStep;
		D3DXVec3TransformCoordArray(PtFrustum, sizeof(D3DXVECTOR3), PtFrustums, sizeof(D3DXVECTOR3), &MatFinal, 9);



	// 第二步，计算View Frustum的AABB包围盒（2D），然后找到包围梯形的四个顶点
		D3DXVECTOR3 PtUpLeft, PtDownRight;
		if(FAILED(D3DXComputeBoundingBox(PtFrustum+1, 8, sizeof(D3DXVECTOR3), &PtUpLeft, &PtDownRight)))
			return E_FAIL;
		// 计算好包围盒，但千万别忘了，Near Plane在上，事实上因为PP空间Y轴是从下到上递增的，所以Near Plane的Y值反而比较大，那么包围盒的DownRight实际上挨着的是Near Plane而不是Far
		// 所以我们交换这两个值，使它符合我们的习惯
		D3DXVECTOR3 fTemp = PtDownRight;
		PtDownRight = PtUpLeft;
		PtUpLeft = fTemp;

		// 这时梯形的顶、底边都是平行于X轴的，也就是包围盒的上下边，那么下来就是要确定梯形四个顶点
		// 得到Frustum四条斜边（Near指向Far），然后找到其中夹角最大的角度，然后从Frustum的摄像机位置点开始引平行线，这样就可以保证将Frustum整个囊括其中，而梯形的交点Intersect就和Frustum的摄像机位置相同
		// 现在的摄像机坐标即Intersect一定在最上面，从上向下引一个梯形，而且我们要让这个梯形Top在上，Bottom在下
		D3DXVECTOR3 VecSlopeLine[4];
		float fLeftMaxAngle = 2, fRightMaxAngle = 2;	// Max Cos = Min Theta
		int iLeftMaxNo = 0, iRightMaxNo = 0;
		for(i=0; i<4; i++)
		{
			VecSlopeLine[i] = PtFrustum[1+i+4] - PtFrustum[1+i];
			D3DXVec3Normalize(&VecSlopeLine[i], &VecSlopeLine[i]);
		}
		
		// 找到夹角最大（即cos值最小）的，分左右两面进行
		for(i=0; i<4; i++)
		{
			// 求夹角cos
			fAngle = D3DXVec3Dot(&VecSlopeLine[i], &D3DXVECTOR3(0, -1, 0));
			// 左边
			if(VecSlopeLine[i].x < 0)
			{
				if(fAngle < fLeftMaxAngle)
				{
					fLeftMaxAngle = fAngle;
					iLeftMaxNo = i;
				}
			}
			else
			{
				if(fAngle < fRightMaxAngle)
				{
					fRightMaxAngle = fAngle;
					iRightMaxNo = i;
				}
			}
		}

		// 找到两边最大的夹角了，ok，从摄像机引两条直线，作为梯形的斜边，那么我们就可以得到和顶、底边交点，也就是梯形的四个顶点
		PtTrapezoid[0] = PtFrustum[0];	// 梯形Intersect和Frustum的摄像机是一致的
			// 两底边都和X轴平行，所以四个点的Y值和包围盒的上下边Y值保持一致，只是需要注意
		PtTrapezoid[1].y = PtTrapezoid[2].y = PtUpLeft.y;
		PtTrapezoid[3].y = PtTrapezoid[4].y = PtDownRight.y;

			// 先找到左边的X值（本身是负数，要取正），因为Frustum现在的中线在Y轴上，所以梯形也是对称的，但注意这个点是以摄像机为准引出来的，所以还要加上摄像机点的坐标偏移（当前摄像机并不在原点）
		PtTrapezoid[1].x = (PtUpLeft.y - PtFrustum[0].y) * tanf(acosf(fLeftMaxAngle)) + PtFrustum[0].x;
		PtTrapezoid[3].x = (PtDownRight.y - PtFrustum[0].y) * tanf(acosf(fLeftMaxAngle)) + PtFrustum[0].x;
			// 找到右边的X值（本身是正值，所以要取负），同理
		PtTrapezoid[2].x = -1 * (PtUpLeft.y - PtFrustum[0].y) * tanf(acosf(fRightMaxAngle)) + PtFrustum[0].x;
		PtTrapezoid[4].x = -1 * (PtDownRight.y - PtFrustum[0].y) * tanf(acosf(fRightMaxAngle)) + PtFrustum[0].x;


	// 最后一步，把Frustum和梯形变换回LVP空间，前面的旋转和平移是为了方便计算梯形两底边用的，这里还要还原
		D3DXMatrixInverse(&MatFinal, NULL, &MatFinal);
		//D3DXMatrixIdentity(&MatFinal);
		// 转换梯形的5个点并从Vector 2D恢复到Vector 3D，Z值总是为0（这只是View Frustum的变形而已，到后面的计算都是在2D面上进行的，其实牵扯不到Z值）
		D3DXVec3TransformCoordArray(pPtTrapezoid, sizeof(D3DXVECTOR3), PtTrapezoid, sizeof(D3DXVECTOR3), &MatFinal, 5);
		for(i=0; i<5; i++)
			pPtTrapezoid[i].z = 0;

		
		// 最后三步错误检测，如果计算出的梯形两侧边平行（即DOT为1或-1），就强制USM
		D3DXVECTOR3 VecSideLine1 = pPtTrapezoid[1] - pPtTrapezoid[3];
		D3DXVECTOR3 VecSideLine2 = pPtTrapezoid[2] - pPtTrapezoid[4];
		D3DXVec3Normalize(&VecSideLine1, &VecSideLine1);
		D3DXVec3Normalize(&VecSideLine2, &VecSideLine2);
		float fFov = absf(D3DXVec3Dot(&VecSideLine1, &VecSideLine2));
		if(absf(1-fFov) < 0.00001f)
		{
			m_bForceUSM = true;
			return S_OK;
		}

		// Bottom->Intersect如果和Near->Intersect异向，就失败
		D3DXVECTOR3 VecB2ELine1 = pPtTrapezoid[0] - pPtTrapezoid[3];
		D3DXVECTOR3 VecB2ELine2 = pPtTrapezoid[0] - pPtTrapezoid[4];
		D3DXVECTOR3 VecN2ELine1 = pPtTrapezoid[0] - pPtTrapezoid[1];
		D3DXVECTOR3 VecN2ELine2 = pPtTrapezoid[0] - pPtTrapezoid[2];
		D3DXVec3Normalize(&VecB2ELine1, &VecB2ELine1);
		D3DXVec3Normalize(&VecB2ELine2, &VecB2ELine2);
		D3DXVec3Normalize(&VecN2ELine1, &VecN2ELine1);
		D3DXVec3Normalize(&VecN2ELine2, &VecN2ELine2);
		float fFov1 = D3DXVec3Dot(&VecB2ELine1, &VecN2ELine1);
		float fFov2 = D3DXVec3Dot(&VecB2ELine2, &VecB2ELine2);
		if(absf(-1-fFov1) < 0.00001f || absf(-1-fFov2) < 0.00001f)
		{
			m_bForceUSM = true;
			return S_OK;
		}

		// 这一步是防止斜边和底边呈将近平行的状态，这就搞成三角形而非梯形了。。。
		D3DXVECTOR3 VecBottom = pPtTrapezoid[1] - pPtTrapezoid[2];
		D3DXVECTOR3 VecTop = pPtTrapezoid[3] - pPtTrapezoid[4];
		D3DXVec3Normalize(&VecBottom, &VecBottom);
		D3DXVec3Normalize(&VecTop, &VecTop);
		float fFovBottom1 = absf(D3DXVec3Dot(&VecBottom, &VecSideLine1));
		float fFovBottom2 = absf(D3DXVec3Dot(&VecBottom, &VecSideLine2));
		float fFovTop1 = absf(D3DXVec3Dot(&VecTop, &VecSideLine1));
		float fFovTop2 = absf(D3DXVec3Dot(&VecTop, &VecSideLine2));

		float fClampValue = 0.001f;
		if(absf(1-fFovBottom1) < fClampValue || absf(1-fFovBottom2) < fClampValue || absf(1-fFovTop1) < fClampValue || absf(1-fFovTop2) < fClampValue)
		{
			m_bForceUSM = true;
			return S_OK;
		}
		

		return S_OK;




	// Test
//		pPtTrapezoid[1] = PtUpLeft;
//		pPtTrapezoid[4] = PtDownRight;
//		pPtTrapezoid[2] = D3DXVECTOR3(PtDownRight.x, PtUpLeft.y, 0);
//		pPtTrapezoid[3] = D3DXVECTOR3(PtUpLeft.x, PtDownRight.y, 0);

//	pPtMidLine[0] = (pPtFrustum[5] + pPtFrustum[6] + pPtFrustum[7] + pPtFrustum[8]) / 4;
//	pPtMidLine[1] = (pPtFrustum[1] + pPtFrustum[2] + pPtFrustum[3] + pPtFrustum[4]) / 4;
}