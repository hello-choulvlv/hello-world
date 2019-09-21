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
	float fFovY = D3DX_PI/2;	// Y����ļн�����̶�ΪD3DX_PI/2������ע���������ProjTransform�����иı��ˣ�FovY�ĳ�ʼֵҲҪ�ı䣬��������

	// ��һ���������������
	PtFrustum[0] = Eye;

	// ��������ռ�������View Frustum����9�����㣬�����ͻ���0,0,0Ϊ���ģ������λ�ã���Z����Ϊ���߷��򣬱��ڼ��㣬���ͻ�����������������������λ����ת��ƽ��
	float fNearY = 0, fNearX = 0, fFarY = 0, fFarX = 0;
	fTemp = tanf(fFovY / 2);
	fNearY = fTemp * fNear;
	fFarY = fTemp * fFar;

	// X�����fov����tan(fovY) * Coef������tan(fovY * Coef)
	fTemp = tanf(fFovY / 2) * m_fProjXYCoef;
	fNearX = fTemp * fNear;
	fFarX = fTemp * fFar;
	
	// �������ĸ�����Nearƽ����ĸ����㣬����������������
	D3DXVECTOR3 PtFrustums[9];
	PtFrustums[1] = D3DXVECTOR3(-fNearX, fNearY,  fNear);
	PtFrustums[2] = D3DXVECTOR3(fNearX,  fNearY,  fNear);
	PtFrustums[3] = D3DXVECTOR3(-fNearX, -fNearY, fNear);
	PtFrustums[4] = D3DXVECTOR3(fNearX,  -fNearY, fNear);

	// ����ĸ�����Farƽ����ĸ����㣬����������������
	PtFrustums[5] = D3DXVECTOR3(-fFarX, fFarY,  fFar);
	PtFrustums[6] = D3DXVECTOR3(fFarX,  fFarY,  fFar);
	PtFrustums[7] = D3DXVECTOR3(-fFarX, -fFarY, fFar);
	PtFrustums[8] = D3DXVECTOR3(fFarX,  -fFarY, fFar);

	// ��ת��ƽ�ƣ���������ռ�ת��������ռ�
	D3DXMATRIX Mat, MatTemp;
	D3DXMatrixIdentity(&Mat);
	D3DXMatrixIdentity(&MatTemp);
	float fAngle = 0;

	//Test
	D3DXMatrixInverse(&Mat, NULL, &myview);
	/*
		// ����������ռ������������������ռ����������ļнǣ���������ת����������������ռ�Z���������������ת������ռ���ȷ��λ��
		D3DXVECTOR3 VecView;
		D3DXVec3Normalize(&VecView, &m_VecLook);

		D3DXVECTOR3 VecViewCross;
		D3DXVec3Cross(&VecViewCross, &D3DXVECTOR3(0, 0, 1), &VecView);
		fAngle = D3DXVec3Dot(&VecView, &D3DXVECTOR3(0, 0, 1));
		if(fAngle <= -0.9999999f)
			fAngle = D3DX_PI;
		else
		fAngle = acosf(fAngle);
		//��ת��������ʱ�룬��һ������Ҫ�� fAngle = D3DX_PI*2 - fAngle;
		D3DXMatrixRotationAxis(&MatTemp, &VecViewCross, fAngle);

		Mat *= MatTemp;
		

		// ƽ��
		D3DXMatrixTranslation(&MatTemp, Eye.x, Eye.y, Eye.z);
		Mat *= MatTemp;

  		*VecLookAt = VecView;
*/
	// ���ݾ���ת����8����
	D3DXVec3TransformCoordArray(PtFrustum+1, sizeof(D3DXVECTOR3), PtFrustums+1, sizeof(D3DXVECTOR3), &Mat, 8);

	return S_OK;
}

HRESULT CAMERACHANGE::InitCamera(float Eyex, float Eyey, float Eyez, float Lookx, float Looky, float Lookz, float Headx, float Heady, float Headz, float move, float rise, float angle)
{
	if(D3DXVec3Length(&D3DXVECTOR3(Lookx-Eyex, Looky-Eyey, Lookz-Eyez)) == 0)
		return D3DERR_INVALIDCALL;

	// �ȼ��Angle������ǽǶȣ���ת�ɻ���
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
	//�ȼ���Angle_H��ֻ����XZƽ��
	Angle_H=asinf((Lookx-Eyex)/len);
	//�ټ���Angle_V�����ȼ���������XZƽ����ͶӰ�ĳ��ȣ�Ȼ����Y/�˳��ȼ��Ƕȵ�����
	Angle_V=atanf((Eyey-Looky)/len);
*/
	//��ʼ�����������ֵ
	m_fMoveSpeed = move;
	m_fRiseSpeed = rise;
	m_fAngleSpeed = angle;

	// ��ʼ�������ռ�������VIEW����
	Eye = D3DXVECTOR3(Eyex, Eyey, Eyez);
	D3DXVec3Normalize(&m_VecLook, &D3DXVECTOR3(Lookx-Eyex, Looky-Eyey, Lookz-Eyez));
	D3DXVec3Normalize(&m_VecHead, &D3DXVECTOR3(Headx, Heady, Headz));
	if(FAILED(GenerateViewSpace(&m_VecLook, &m_VecHead, &m_VecRight_X)))
		return D3DERR_INVALIDCALL;
	Look = Eye + m_VecLook;
	D3DXMatrixLookAtLH(&myview, &Eye, &Look, &m_VecHead);
	
	// ��ʼ��ʱ��
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
	// �����������Ŀ��������Slice��TurnUp/Down��Ҳ����˵Ҫ��ת������ʹ��HEAD��Y�غϣ�Look��Right_X��XZ����
	D3DXVECTOR3 VecProj, VecCross, VecTemp;
	D3DXMATRIX MatRotate;
	float fAngle = 0.0f;

	// �ȼ���Right_X��XZ���ϵ�ͶӰ����ת������Slice
	VecProj = D3DXVECTOR3(m_VecRight_X.x, 0, m_VecRight_X.z);
	D3DXVec3Normalize(&VecProj, &VecProj);
		// ����Ƕ�
	fAngle = D3DXVec3Dot(&VecProj, &m_VecRight_X);
	fAngle = acosf(fAngle);
		// ����нǹ�С�����ѿ���Slice�����Ͳ��ı䵱ǰ�趨��������ھ�������������޷��ֱ��С�нǵ���΢���Ļζ�
	if(fAngle > 0.05f)
	{
		// ���������Look����ת
		D3DXMatrixRotationAxis(&MatRotate, &m_VecLook, fAngle);
		// �任ʣ�µ�������
		VecTemp = m_VecRight_X;
		D3DXVec3TransformCoord(&m_VecRight_X, &VecTemp, &MatRotate);
		VecTemp = m_VecHead;
		D3DXVec3TransformCoord(&m_VecHead, &VecTemp, &MatRotate);
	}
	
	// �����и������ɼ�żȻ�Ķ���������Ϊʲô���ж���ת�᣿��Ϊ�������������ͶӰһ�Σ������޷��ж���ת��
	// Dot�����ҷ������޷��жϳ����ģ�������ò������ת��Ļ�����Headƫ��ĳ������ʱ�ͻ�����⣬����Ӧ�ó��෴�ķ���ת��������ȴ��ͬ����ת��������ƫ���������
	// �������������ж���Head�����������ж�Turn�ļнǣ�ͬʱ�������Slice��ƫ�����ʱRight_X��Look���Ѿ���XZ���غ��ˣ�����������ȷ����ת��Ϳ��������ȷ�Ľ����



	// �Ѿ�ת������XZ���ϣ��ټ���Head��Y�ļнǣ�����Right_Xת��������TurnUp/Down
		// ����Ƕ�
	fAngle = D3DXVec3Dot(&D3DXVECTOR3(0, 1, 0), &m_VecHead);
	fAngle = acosf(fAngle);
		// ������ת�ᣬ������ת����Ϊ���ܸ㶨��ת�����ҷ�������ʵ��m_Look��ֻ࣬�ǿ��ܻ�Ϊ���ᣬ���Ի��Զ�����������λ�ù�ϵ������ת�ķ���
	D3DXVec3Cross(&VecCross, &m_VecHead, &D3DXVECTOR3(0, 1, 0));
	D3DXVec3Normalize(&VecCross, &VecCross);
		// ����нǹ�С�����ѿ���TurnUp/Down�����Ͳ��ı䵱ǰ�趨��������ھ�������������޷��ֱ��С�нǵ���΢���Ļζ�
	if(fAngle > 0.05f)
	{
		// �������
		D3DXMatrixRotationAxis(&MatRotate, &VecCross, fAngle);
		// �任ʣ�µ�������
		VecTemp = m_VecHead;
		D3DXVec3TransformCoord(&m_VecHead, &VecTemp, &MatRotate);
		VecTemp = m_VecLook;
		D3DXVec3TransformCoord(&m_VecLook, &VecTemp, &MatRotate);
	}


	// ����LOOK��
	Look = Eye + m_VecLook;
}




// ��0��11λ�ֱ����ǰ���ƣ������ƣ������ƣ�����ת������ת������ҡ��
// ������������ʼ���ǹ�񻯵ģ�������ֻ����ת���������Բ���Ӱ�����ĳ���
void CAMERACHANGE::ChangeCamera(unsigned short int changeway, float fMoveCoef)
{
	if(!m_bInit) return;   //û��ʼ���ͷ���

	// ��ʱʹ�ã������Ƿ񳬹������ת�Ƕȣ�Headֻ��ת180�ȣ����жϸ���ת�����Ƿ���Ч����д��m_Vec��
	D3DXVECTOR3 VecHead = m_VecHead, VecLook = m_VecLook, VecRight_X = m_VecRight_X;

	
	// ���δ�ı䣬��ô��˵������Idle״̬
	if(changeway == CAM_NOCHANGE)
	{
		// ���bIdle��false��˵���յ�Idle״̬������
		if(!m_bIdle)
		{
			m_bIdle = true;
			m_MoveAttrib = 3;	// �մ�Moving��Idle
			dwLastIdleTime = timeGetTime();
		}
		else
			m_MoveAttrib = 0;	// һֱIdle
	}
	else
	{
		if(m_bIdle)
			m_MoveAttrib = 2;	// �մ�Idle��Moving
		else
			m_MoveAttrib = 1;	// һֱMoving
		m_bIdle = false;
	}

	// �õ���ǰʱ��
	dwCurrentTime = timeGetTime();
	float fChangeRatio = (float)(dwCurrentTime-dwLastFrameTime) / 1000;
	// ���ƶ��������루�����ƣ�
	fChangeRatio *= fMoveCoef;

	if(changeway&1)               //GOFORWARD
	{
		D3DXVECTOR3 VecChange;
		
		//��������ȷ��Y�ģ�������ȫ�������ڵ������ߣ���Ҫ����ֻ����Y��ͬ��һ��ƽ�����ƶ�
		if(m_bYMove)
			VecChange = D3DXVECTOR3(m_VecLook.x, 0, m_VecLook.z);
		else
			VecChange = m_VecLook;
	    Eye += VecChange * fChangeRatio * m_fMoveSpeed;
	}

	else if(changeway>>1&1)         //GOBACKWARD
	{
		D3DXVECTOR3 VecChange;
		
		//��������ȷ��Y�ģ�������ȫ�������ڵ������ߣ���Ҫ����ֻ����Y��ͬ��һ��ƽ�����ƶ�
		if(m_bYMove)
			VecChange = D3DXVECTOR3(m_VecLook.x, 0, m_VecLook.z);
		else
			VecChange = m_VecLook;
		Eye -= VecChange * fChangeRatio * m_fMoveSpeed;
	}

	
	else if(changeway>>2&1)      //��ƽ��GOLEFT
	{
		D3DXVECTOR3 VecChange;
		
		//��������ȷ��Y�ģ�������ȫ�������ڵ������ߣ���Ҫ����ֻ����Y��ͬ��һ��ƽ�����ƶ�
		if(m_bYMove)
			VecChange = D3DXVECTOR3(m_VecRight_X.x, 0, m_VecRight_X.z);
		else
			VecChange = m_VecRight_X;
		Eye -= VecChange * fChangeRatio * m_fMoveSpeed;
	}

	else if(changeway>>3&1)   //��ƽ��GORIGHT
	{
		D3DXVECTOR3 VecChange;
		
		//��������ȷ��Y�ģ�������ȫ�������ڵ������ߣ���Ҫ����ֻ����Y��ͬ��һ��ƽ�����ƶ�
		if(m_bYMove)
			VecChange = D3DXVECTOR3(m_VecRight_X.x, 0, m_VecRight_X.z);
		else
			VecChange = m_VecRight_X;
		Eye += VecChange * fChangeRatio * m_fMoveSpeed;
	}

	else if(changeway>>4&1)      //������GOUP
	{
		Eye.y += fChangeRatio * m_fRiseSpeed;
	}
	else if(changeway>>5&1)      //���½�GODOWN
	{
		Eye.y -= fChangeRatio * m_fRiseSpeed;
	}




	// ������3�����͵���ת��ǣ��������������	
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
		// �������Y�ᣬ�����۲�����ϵ�������������������ռ��XZ����ת�����ɣ�ע�ⲻҪ©��Head��Ŷ
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
		// �����ԵĻ��Ͱ����������Right_X��Look��ɵ�����Χ��Head��ת��
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
	
	else if(changeway>>8&1 || changeway>>9&1)      //���ϻ�����תTURN(UP/DOWN)
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

	else if(changeway>>10&1 || changeway>>11&1)      //���������Sliceת��Slice(UP/DOWN)
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


	// �ж���ת�����Ƿ���Ч����VecHeadֻ����Y����������Ч��д��m_Vec
	if(VecHead.y >= 0.0f)
	{
		m_VecHead = VecHead;
		m_VecLook = VecLook;
		m_VecRight_X = VecRight_X;
	}


	// �õ�Look�������
	Look = Eye + m_VecLook;
}








/*
	if(changeway&1)               //GOFORWARD
	{
	    Eye.x+=sinf(Angle_H)* fChangeRatio * MoveStep;
        Eye.z+=cosf(Angle_H)* fChangeRatio * MoveStep;

		Look.x+=sinf(Angle_H)* fChangeRatio * MoveStep;
        Look.z+=cosf(Angle_H)* fChangeRatio * MoveStep;

		//��������ȷ��Y�ģ�������ȫ�������ڵ������ߣ���Ҫ����ֻ����Y��ͬ��һ��ƽ�����ƶ�
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

		//��������ȷ��Y�ģ�������ȫ�������ڵ������ߣ���Ҫ����ֻ����Y��ͬ��һ��ƽ�����ƶ�
		if(Y_AxisTranslate)
		{
			Eye.y+=sinf(Angle_V)* fChangeRatio * MoveStep;
		    Look.y+=sinf(Angle_V)* fChangeRatio * MoveStep;
		}
	}

	
	else if(changeway>>2&1)      //��ƽ��GOLEFT
	{
	    Eye.x-=cosf(Angle_H)* fChangeRatio * RiseStep;
        Eye.z+=sinf(Angle_H)* fChangeRatio * RiseStep;

		Look.x-=cosf(Angle_H)* fChangeRatio * RiseStep;
        Look.z+=sinf(Angle_H)* fChangeRatio * RiseStep;
	}

	else if(changeway>>3&1)   //��ƽ��GORIGHT
	{
	    Eye.x+=cosf(Angle_H)* fChangeRatio * RiseStep;
        Eye.z-=sinf(Angle_H)* fChangeRatio * RiseStep;

		Look.x+=cosf(Angle_H)* fChangeRatio * RiseStep;
        Look.z-=sinf(Angle_H)* fChangeRatio * RiseStep;
	}

	else if(changeway>>4&1)      //������GOUP
	{
		Eye.y+= fChangeRatio * RiseStep;
		Look.y+= fChangeRatio * RiseStep;
	}
	else if(changeway>>5&1)      //���½�GODOWN
	{
		Eye.y-= fChangeRatio * RiseStep;
		Look.y-= fChangeRatio * RiseStep;
	}
	
	else if(changeway>>6&1)  //TURNLEFT
	{
		//len��������XZ����ͶӰ�ĳ��ȣ�ʹ��LEN����LOOKY��Ϊ0��ʱ��֤XZ��Yֵ����ƥ�䣬������������ת��֮������ת�ͻ����ͻȻ��Ծ�����
		float len=sqrtf((Look.x-Eye.x)*(Look.x-Eye.x)+(Look.z-Eye.z)*(Look.z-Eye.z));  //XZ���߳���
		Angle_H -= (fChangeRatio * AngleStep);
		
			Look.x = Eye.x+sinf(Angle_H)*len;
			Look.z = Eye.z+cosf(Angle_H)*len;
	}
	
	else if(changeway>>7&1)  //TURNRIGHT
	{
		float len=sqrtf((Look.x-Eye.x)*(Look.x-Eye.x)+(Look.z-Eye.z)*(Look.z-Eye.z));  //XZ���߳���
		Angle_H += (fChangeRatio * AngleStep);

		Look.x = Eye.x+sinf(Angle_H)*len;
		Look.z = Eye.z+cosf(Angle_H)*len;
	}

	else if(changeway>>8&1||changeway>>9&1)      //���ϻ�����תTURN(UP/DOWN)
	{
		float var;  //��XZƽ���ϵ�ͶӰ
		//��ת�Ƕȼ�����ת�Ƕȼ�
		if(changeway>>8&1) Angle_V -= (fChangeRatio * AngleStep);
		else Angle_V += (fChangeRatio * AngleStep);
        
		// Angle_V����Ϊ��������sin��cos������PIΪ�ԳƵģ�Ϊ�����Ļ����Ǻ���ֵ�ͻ��ڰ���֮�������𵴣����Ϊ��������2PI�Ĳ���������ֵ����չ��2PI����2PI֮��ѭ����������PI֮����
		if(Angle_V < 0) Angle_V = D3DX_PI*2 + Angle_V;

		Look.y = Eye.y-sinf(Angle_V);

		var=cosf(Angle_V);
		Look.x=Eye.x+sinf(Angle_H)*var;
		Look.z=Eye.z+cosf(Angle_H)*var;
		//��Ҫ���ͷ��������XZƽ���°���ͷ��Yֵȡ��
		for(float temp=Angle_V;temp>=D3DX_PI*2;temp-=D3DX_PI*2);   //�õ�Ŀǰ��ֵ������360�ȵĲ��㣩
		if(temp>=D3DX_PI/2&&temp<D3DX_PI*3/2) m_VecHead.y=-1;
		else m_VecHead.y=1;
	}
*/



/*ʹ�þ������任��ֻ�ǲο�����
//��0��9λ�ֱ����ǰ���ƣ������ƣ������ƣ�����ת������ת
void CAMERACHANGE::ChangeCamera(unsigned short int changeway)
{
	if(IsInit==FALSE) return;   //û��ʼ���ͷ���
	D3DXMATRIX Transform;

	if(changeway&1)               //GOFORWARD
	{
		D3DXMatrixTranslation(&Transform, 0, 0, -1*MoveStep);
	}

	if(changeway>>1&1)         //GOBACKWARD
	{
		D3DXMatrixTranslation(&Transform, 0, 0, MoveStep);
	}

	
	if(changeway>>2&1)      //��ƽ��GOLEFT
	{
		D3DXMatrixTranslation(&Transform, cosf(Angle_H)*RiseStep, 0, -1*sinf(Angle_H)*RiseStep);
	}

	if(changeway>>3&1)   //��ƽ��GORIGHT
	{
		D3DXMatrixTranslation(&Transform, -1*cosf(Angle_H)*RiseStep, 0, sinf(Angle_H)*RiseStep);
	}

	if(changeway>>4&1)      //������GOUP
	{
		D3DXMatrixTranslation(&Transform, 0, -1*RiseStep, 0);
	}
	if(changeway>>5&1)      //���½�GODOWN
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

	if(changeway>>8&1)      //����תTURNUP
	{
		D3DXVECTOR3 axis;
		float tg,arg;
		D3DXQUATERNION Qtn;
		
		//���������XZ���ϵ�ͶӰ������X����ļн�����
		tg=(Look.z-Eye.z)/(Look.x-Eye.x);
		//�����������X����ļн�
		arg=atanf(tg)+D3DX_PI/2;
		//�����������ĳ���Ϊ1,���������
		axis=D3DXVECTOR3(cosf(arg), 0, sinf(arg));
		D3DXQuaternionRotationAxis(&Qtn, &axis, -1*AngleStep);
		D3DXMatrixAffineTransformation(&Transform, 1, &D3DXVECTOR3(Eye.x,Eye.y,Eye.z), &Qtn,NULL);
	}

	if(changeway>>9&1)     //����תTURNDOWN
	{
		D3DXVECTOR3 axis;
		float tg,arg;
		D3DXQUATERNION Qtn;
		
		//���������XZ���ϵ�ͶӰ������X����ļн�����
		tg=(Look.z-Eye.z)/(Look.x-Eye.x);
		//�����������X����ļн�
		arg=atanf(tg)+D3DX_PI/2;
		//�����������ĳ���Ϊ1,���������
		axis=D3DXVECTOR3(cosf(arg), 0, sinf(arg));
		D3DXQuaternionRotationAxis(&Qtn, &axis, AngleStep);
		D3DXMatrixAffineTransformation(&Transform, 1, &D3DXVECTOR3(Eye.x,Eye.y,Eye.z), &Qtn,NULL);
	}
	
	D3DXMatrixMultiply(&myview, &myview, &Transform);
}
*/

//��Ҫ˵��������ת����ת����Y���������XZƽ���ϵ�ͶӰ�����һ��ƽ�棬��ͶӰ�ߵĽǶ�Ϊ0��ͶӰ�����·�תΪ���Ƕ�
//����������е���ת����ȷ����������֪Ŀǰ�����ߺ���Ҫת�ĽǶȣ����ȷ���µ�LOOK�������أ���ΪEYE���䣬���Խ�EYE�Ķ�Ӧ������ֵ������Ҫת�ĽǶ��ڶ�Ӧƽ���ϵ�ͶӰ���ȼ��ɣ������������
//��Ҫ��LOOK���������Ƕ��٣�����ͨ��EYE�������Ƕ���ȷ��LOOK���꣨�������ȼ���Ϊ1�����ˣ�




















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

	// ֱ�������µģ���ʼ����ʱ���¾����׶��Ѿ�׼������
	V_RETURN(SetTexturedRenderTarget(0, m_pTexShadowMap[iShadowMapNo], m_pDepthBuffer));

	d3ddevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0);

	m_iCreateAttrib = 2;
	
	return S_OK;
}

HRESULT KTrapezoidalShadowMap::RestoreRenderTarget()
{
	if(!m_pOldBackBuffer || !m_pOldDepthBuffer || m_iCreateAttrib<2)
		return E_FAIL;
	
	// �ָ��ɵĻ���
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
	
	// ��ʼ����Դλ�ú͹�Դ����
	m_PtLightPosition = PtLightPos;
	m_VecLightDirection = VecLightDirection;
	m_VecHeadDirection = VecHeadDirection;
	if(FAILED(GenerateViewSpace(&m_VecLightDirection, &m_VecHeadDirection, &m_VecRightDirection)))
		return D3DERR_INVALIDCALL;
	
	// ��ʼ��VIEW����
	D3DXMatrixLookAtLH(&m_MatLVPView, &m_PtLightPosition, &(m_PtLightPosition+m_VecLightDirection), &m_VecHeadDirection);
	
	// ���ݹ�Դ���ͳ�ʼ��ͶӰ����
	if(LightType == D3DLIGHT_SPOT)
	{
		if(fXRangeCoef <=0 || fXRangeCoef*fYRangeCoef >=2.0f || fYRangeCoef >=2.0f || fYRangeCoef <=0)
			return D3DERR_INVALIDCALL;

		// LVP�ռ�Ϊ��ʹ��Omni������Ҫ��֤fovx = fovy = 90degree
		D3DXMatrixPerspectiveFovLH(&m_MatLVPProj, D3DX_PI/2 * fYRangeCoef, 1.0f * fXRangeCoef, fMinRange, fMaxRange);
	}
	else if(LightType == D3DLIGHT_DIRECTIONAL)
	{
		if(fXRangeCoef <=0 || fYRangeCoef <=0)
			return D3DERR_INVALIDCALL;

		// �����û��Omni������νXYͶӰ�ı���
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

	// ��ʼ����Ӱ��ͼ
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
	

	// ��ʼ���µ���Ȼ��壨��ʱʹ�ã���ͬʱ�õ��ɵ���Ȼ���
	if(FAILED(d3ddevice->GetDepthStencilSurface(&m_pOldDepthBuffer)))
		return E_FAIL;
	if(FAILED(m_pOldDepthBuffer->GetDesc(&Desc)))
		return E_FAIL;
	SAFE_RELEASE(m_pOldDepthBuffer);

	if(FAILED(d3ddevice->CreateDepthStencilSurface(iWidth, iHeight, Desc.Format, Desc.MultiSampleType, Desc.MultiSampleQuality, FALSE, &m_pDepthBuffer, NULL)))
		return E_FAIL;


	
	// ��ʼ��Ĭ�Ϲ�Դ
	if(FAILED(SetLight(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 1), D3DXVECTOR3(0, 1, 0))))
		return D3DERR_INVALIDCALL;
	// ��ʼ��Ĭ��EF
	SetFrustumRange(2.0f, 200.0f);
	
	
	m_iCreateAttrib = 1;
	return S_OK;
}






/***************************�ڲ�ʹ��*************************/
HRESULT KTrapezoidalShadowMap::GetTSpaceMatrix(D3DXMATRIX *pMatTSpace)
{
	// ��������¿��ܻ����ĸ����Σ�������5*4
	D3DXVECTOR3 PtFrustum[9], PtTrapezoid[5*4];
	if(!pMatTSpace || D3DXVec3Length(&m_VecLightDirection) == 0)
		return D3DERR_INVALIDCALL;

	D3DXVECTOR3 VecLookAt;
	if(FAILED(CameraChange.GetEyeFrustum(PtFrustum, m_fVirtualEFNearPlane, m_fVirtualEFFarPlane)))
		return E_FAIL;
	ViewFrustumToLVPSpace(PtFrustum);
	
	// ����������һ�����Ѿ�����ȷ����ǿ��USM����ǿ��4 ShadowMaps�ˣ�������һ���͸��ݱ������1�����λ�4�����ε�����
	if(FAILED(FrustumToTrapezoid(PtFrustum, PtTrapezoid)))
		return E_FAIL;
	
	// �������
	if(!m_bFourShadowMaps)
		*pMatTSpace = TransformToTrapezoidSpace(PtTrapezoid[1], PtTrapezoid[2], PtTrapezoid[3], PtTrapezoid[4], PtTrapezoid[0]);
	else
	{
		// 4 ShadowMaps������Ԥ�ȴ���4�����������ָ�룬������ڴ�Խ��
		for(int i=0; i<4; i++)
			pMatTSpace[i] = TransformToTrapezoidSpace(PtTrapezoid[1+i*5], PtTrapezoid[2+i*5], PtTrapezoid[3+i*5], PtTrapezoid[4+i*5], PtTrapezoid[0+i*5]);
	}

	return S_OK;
}



void KTrapezoidalShadowMap::ViewFrustumToLVPSpace(D3DXVECTOR3 *PtFrustum)
{
	D3DXMATRIX MatProj, MatView;
	
	// Ϊ�˱���Transformʱʧ�ܣ������ȸ���һ��
	int i = 0;
	D3DXVECTOR3 PtFrustums[9];
	for(i=0; i<9; i++)
		PtFrustums[i] = PtFrustum[i];

	D3DXMATRIX Mat = m_MatLVPView * m_MatLVPProj;
	
	D3DXVECTOR4 PtFrustumsVec4[9];
	D3DXVec3TransformArray(PtFrustumsVec4, sizeof(D3DXVECTOR4), PtFrustums, sizeof(D3DXVECTOR3), &Mat, 9);
	

	// ��������ͷϷ��ֱ�ӹ�ϵ��TSM�Ŀ����ԣ�������Ҫ���������Ͻ�ˮ������
	// ������ִ���������ǿ��USM��4ShadowMaps^_^
	m_bForceUSM = false;
	m_bFourShadowMaps = false;

	// ǧ��ǵ�ͶӰ��ʱ����ܻ���Frustum��ĳЩ��Zֵ����ͶӰ�任���Wֵ����LVP�ռ�ı�ɸ�����������LVP�ռ�õ��ܵ���Դ����ȥ�ˣ�����Ϊ����ֻ����ͶӰ������ǣ�����ã�����ǿ������Ϊ��������Զ
	// ��xyΪ���ı�������Ϊ���ı�Ϊ���������������Ϊ0����Ϊ0����xyΪ��ֵ��Ҳ��Ϊ������
	for(i=0; i<9; i++)
		if(PtFrustumsVec4[i].w < 0)
		{
			PtFrustumsVec4[i].w = 0.00000001f;
			// ��ʵ���ﲻ�ù�����wֵ������ǿ��USM����
			m_bForceUSM = true;
			return;
		}

	for(i=0; i<9; i++)
		*(PtFrustum+i) = D3DXVECTOR3(PtFrustumsVec4[i].x/PtFrustumsVec4[i].w, PtFrustumsVec4[i].y/PtFrustumsVec4[i].w, PtFrustumsVec4[i].z/PtFrustumsVec4[i].w);

	// ���FrustumĳЩ��Խ��Far Plane�����ұ߽磨��<-1��>1����Ҳ����ǿ��USM
	
	// �ж�����ⷽ��ӽ�ƽ�������߷��򣬼�Intersect����Near��Far�ڣ�ǿ��USM
		D3DXVECTOR3 PtTemp[9], PtMin, PtMax;
		for(i=0; i<9; i++)
		{
			PtTemp[i] = PtFrustum[i];
			PtTemp[i].z = 0;
		}

/*		// ����4������������ϽǺ����½ǣ���MAX X/Y  Min X/Y���������׵�ͨ��BoundingBox���Ƿ��ڶ������
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
		// ��ȷ���󷨣��ı�����������������ɣ�����Ƿ��������������ڣ��������������Ҫȷ���ĸ��������λ�ù�ϵ��������������1��2��3һ�������Σ�2��3��4һ�������Σ�����������ȷ
		D3DXVECTOR3 PtQuad[4], PtSwap;
		int j = 0;

		// ����Far Plane�����EYE�������ǿ��4 ShadowMaps
		// ΪʲôҪFar�����أ���Ϊ���EYE��Far�У���NearҲ�ض���Far��
		for(i=5; i<9; i++)
			PtQuad[i-5] = PtTemp[i];
			// ð�ݷ����򣬰�����˳���ţ�����X���꣬Z��Ϊ0,���ص���
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
		// �ָ�����������Σ��ֱ�����Ƿ�������֮�ڣ�����һ��������ǿ��USM
		if(IsPointInTriangle(PtTemp[0], PtQuad[0], PtQuad[1], PtQuad[2]) || IsPointInTriangle(PtTemp[0], PtQuad[1], PtQuad[2], PtQuad[3]))
		{
			m_bFourShadowMaps = true;
			return;
		}

		// ����Near Plane�����EYE�������ǿ��USM
		for(i=1; i<5; i++)
			PtQuad[i-1] = PtTemp[i];
		// ð�ݷ����򣬰�����˳���ţ�����X���꣬Z��Ϊ0,���ص���
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
		// �ָ�����������Σ��ֱ�����Ƿ�������֮�ڣ�����һ��������ǿ��USM
		if(IsPointInTriangle(PtTemp[0], PtQuad[0], PtQuad[1], PtQuad[2]) || IsPointInTriangle(PtTemp[0], PtQuad[1], PtQuad[2], PtQuad[3]))
		{
			m_bForceUSM = true;
			return;
		}
}



// �������Ĳ����������LVP�ռ�����ֻ꣨��2D��Ч����Z��Ϊ0�����ص��ǽ������δ�LVP�任��������T-SPACE�ľ���
D3DXMATRIX KTrapezoidalShadowMap::TransformToTrapezoidSpace(D3DXVECTOR3 PtTopL, D3DXVECTOR3 PtTopR, D3DXVECTOR3 PtBottomL, D3DXVECTOR3 PtBottomR, D3DXVECTOR3 PtIntersect)
{
	D3DXMATRIX MatFinal, MatStep;
	D3DXVECTOR3 PtTemp1, PtTemp2;
	float fAngle = 0;
	D3DXMatrixIdentity(&MatFinal);
	
	// ���ִ��������ǿ��USM����������4 ShadowMaps��״̬��ǿ��USM��ʧЧ����Ϊ�����ȼ�Ҫ�ͣ���Far Planeֱ����Ϊ���δ�����Խ�һ�������Ӱ�����������Ƿ����ƽ��ʱ
	if(m_bForceUSM && !m_bFourShadowMaps)
		return MatFinal;

	// TSM Demo��ʹ�õģ�������س���ǿ���л���USM
	if(!effectenable)
		return MatFinal;

	// ��һ����������Top�ߵ��е��Ƶ�LVP���ģ�Ϊ����������ת��������ô��
	D3DXMatrixIdentity(&MatStep);
	
	// Temp1�����е㣬����Ӧ����(R-L) / 2 + L�ģ�Լȥ�͵�(R+L)/2
	PtTemp1 = (PtTopL + PtTopR) / 2.0f;
	// (0,0) - Temp1����λ������
	D3DXMatrixTranslation(&MatStep, -PtTemp1.x, -PtTemp1.y, 0);
	
	MatFinal *= MatStep;


	// �ڶ�������Top����ת����x���غϣ����ҽ����ε���������Top��Bottom���·�
	D3DXMatrixIdentity(&MatStep);
	
	// ����Top����X��ļнǣ�����ֵ��
	D3DXVec3Normalize( &PtTemp1, &(PtTopR-PtTopL) );
	// ����Top�������ķ���������Ƕȣ���Ϊ����������ļн��Ǿ���ֵ����Ȼ������ת������XY����ת������Ȼ����Z�Ῡ��
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
	
	// ���������ƶ����Σ���ǰ�����T-SPACE�����غϣ�Top���е�����ֻ᲻��X�����ˣ����ƣ���������Ҫ���Ƶ�X���ϣ�
	D3DXMatrixIdentity(&MatStep);
	
	// ����ǰ��������ĵľ���
	D3DXVec3TransformCoord(&PtTemp1, &PtIntersect, &MatFinal);
	PtTemp1 = -PtTemp1;
	// �������
	D3DXMatrixTranslation(&MatStep, PtTemp1.x, PtTemp1.y, 0);
	
	MatFinal *= MatStep;
	
	// ���Ĳ�����ɵ�����������Y��Գƣ����У�������˵�����߷������ٺ٣�����Ҫ�Ŵ������������������Y�Գ�
	// Ҫ���������߶���ͬ�ı�����������б�ȣ�tg������������ת����ʱ����y�����ֵ����x���delta
	D3DXMatrixIdentity(&MatStep);
	
		// ��BottomΪ�����ȼ���Top���е㣬��ȻҲ���Լ���Top����Ϊ������б�ȶ���һ���ģ��������Bottom����ΪBottom��TopҪ��ö࣬Top��Ϊ̫С���ܻ��в������
		D3DXVec3TransformCoord(&PtTemp1, &PtTopL, &MatFinal);
		D3DXVec3TransformCoord(&PtTemp2, &PtTopR, &MatFinal);
		PtTemp1 = (PtTemp2 + PtTemp1) / 2;
		// ������б��tan, fAngle
		fAngle = PtTemp1.x / PtTemp1.y;
		// ������о��������Ļ�yzw�����䣬x = x - fAngle*y
		MatStep._21 = -fAngle;
	
	MatFinal *= MatStep;
	
	// ���岽������Y��ԳƵĵ������ηŴ�top�����·�����LVP�ռ���ϵ��غϣ�Bottom���ù���
	D3DXMatrixIdentity(&MatStep);
	
		// ��TopRight��1,1�Ĳ�࣬��Ϊ��Y��ԳƵģ���Left��Ҳ���ԣ���ȻҪ���ɺ�-1,1�Ĳ�࿩
		D3DXVec3TransformCoord(&PtTemp1, &PtTopR, &MatFinal);
		PtTemp1.x = 1 / PtTemp1.x;
		PtTemp1.y = 1 / PtTemp1.y;
		// �������ž���
		D3DXMatrixScaling(&MatStep, PtTemp1.x, PtTemp1.y, 1);		
	
	MatFinal *= MatStep;

	// �������������ص�һ���������α�ɾ��Σ�������תtop�ߵ���������ͬʱ�������漸�����й�ϵ�����������һ����Homorous Divid
	D3DXMatrixIdentity(&MatStep);
	MatStep._44 = 0;
	MatStep._42 = MatStep._24 = 1; 
	
		// 
	
	MatFinal *= MatStep;


	// ���߲��������Σ�top����bottom���£�ƽ�Ƶ�LVP���ģ�ֻ�Ƕ�Y������
	D3DXMatrixIdentity(&MatStep);

		// �õ���ǰ�������ĵ�����
		D3DXVec3TransformCoord(&PtTemp1, &PtTopL, &MatFinal);
		D3DXVec3TransformCoord(&PtTemp2, &PtBottomR, &MatFinal);
		PtTemp1 = (PtTemp1 + PtTemp2) / 2;
		// �������ֻƽ��Y������ƽ�ƣ������Ǽ�
		D3DXMatrixTranslation(&MatStep, 0, -PtTemp1.y, 0);
	
	MatFinal *= MatStep;


	// �ڰ˲������Ѵ������ĵĶԳƾ�����Y������ʹ�ó�������LVP�ռ䣬T-SPACE��ȫ����ok����ʱ��T-SPACE�͵���LVP�ռ俩��
	D3DXMatrixIdentity(&MatStep);
	
		// �õ��������ϵ������������X��ԳƵģ���һ����Ϊ׼�����ˣ�������topΪ׼
		D3DXVec3TransformCoord(&PtTemp1, &PtTopL, &MatFinal);
		// �������ž���ֻ�Ŵ�Y��
		D3DXMatrixScaling(&MatStep, 1, 1 / PtTemp1.y, 1);
			
	MatFinal *= MatStep;

	
	return MatFinal;
}






// ��LVP�ռ��View Frustumת�������Σ����ҵ���С�����ΰ�Χ���򣨴�2D�������������ں��湹��LVP��T-SPACE(EYE SPACE���)��ת������
// ����9��VEC3������Frustum��LVP�����꣬0���������1��4��Near Plane���ϡ����ϡ����¡����£�5��8��Far Plane
// ���5��VEC3������������LVP��2D���꣬0��ǰ����Intersect��1��4��TopLeft TopRight BottomLeft BottomRight
HRESULT KTrapezoidalShadowMap::FrustumToTrapezoid(D3DXVECTOR3 *pPtFrustum, D3DXVECTOR3 *pPtTrapezoid)
{
	if(!pPtFrustum)
		return D3DERR_INVALIDCALL;
	
	int i = 0;
	
	// ���LVP EYE����Far Plane�������ĸ�����
	if(m_bFourShadowMaps)
	{
		// �ȼ���Ƿ�ս���Far Plane�����ʱ��Intersect��Far Plane�������߿��ܺ�Far���׽ӽ�ƽ�У�ֱ����4�����λ�������Ρ�
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
		
		float fClampValue = 0.07f;		// ���ֵһ��Ҫѡ���ʣ�С�˾�û��Ч���������ֻ�ѱ�������������4SM�ĵط��ĳ�USM
		if(absf(1-fFovBottom1) < fClampValue || absf(1-fFovBottom2) < fClampValue || absf(1-fFovBottom3) < fClampValue || absf(1-fFovBottom4) < fClampValue
			|| absf(1-fFovTop1) < fClampValue || absf(1-fFovTop2) < fClampValue || absf(1-fFovTop3) < fClampValue || absf(1-fFovTop4) < fClampValue)
		{
			// ����Far Plane��ߵĽ�����Ϊ���ν���
			if(!GetIntersect2D(pPtTrapezoid, pPtFrustum[5], pPtFrustum[7], pPtFrustum[6], pPtFrustum[8]))
			{
				// �󽻵�ʧ�ܣ�ǿ����USM
				m_bForceUSM = true;
			}
			else
			{
				// ��Far Plane��Ϊ����
				for(i=5; i<9; i++)
					pPtTrapezoid[i-4] = pPtFrustum[i];
			}

			m_bFourShadowMaps = false;
			return S_OK;
		}


		// ����������Ĵ�������������ʹ��������4 ShadowMaps
		// ��һ�����Σ�����
		i = 0;
		GetIntersect2D(&pPtTrapezoid[0+i*5], pPtFrustum[3], pPtFrustum[5], pPtFrustum[4], pPtFrustum[6]);
		pPtTrapezoid[1+i*5] = pPtFrustum[3];
		pPtTrapezoid[2+i*5] = pPtFrustum[4];
		pPtTrapezoid[3+i*5] = pPtFrustum[5];
		pPtTrapezoid[4+i*5] = pPtFrustum[6];
		
		// �ڶ������Σ�����
		i++;
		GetIntersect2D(&pPtTrapezoid[0+i*5], pPtFrustum[4], pPtFrustum[7], pPtFrustum[2], pPtFrustum[5]);
		pPtTrapezoid[1+i*5] = pPtFrustum[4];
		pPtTrapezoid[2+i*5] = pPtFrustum[2];
		pPtTrapezoid[3+i*5] = pPtFrustum[7];
		pPtTrapezoid[4+i*5] = pPtFrustum[5];
		
		// ���������Σ�����
		i++;
		GetIntersect2D(&pPtTrapezoid[0+i*5], pPtFrustum[2], pPtFrustum[8], pPtFrustum[1], pPtFrustum[7]);
		pPtTrapezoid[1+i*5] = pPtFrustum[2];
		pPtTrapezoid[2+i*5] = pPtFrustum[1];
		pPtTrapezoid[3+i*5] = pPtFrustum[8];
		pPtTrapezoid[4+i*5] = pPtFrustum[7];

		// ���ĸ����Σ�����
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

	// ��һ��������Frustum�����ߣ���Farָ��Near�������е㣬�����е㽫Frustumƽ�Ƶ�LVP���룬����ת��ʹ�����߳���LVP��Y�ᣬ��һ����Ϊ�˷������Top��Bottom����ΪҪ���ߴ�ֱX��űȽϺ�������ƽ�еĵױ���
	D3DXMatrixIdentity(&MatStep);

		D3DXVECTOR3 PtTopMidPoint = (PtFrustums[1] + PtFrustums[2] + PtFrustums[3] + PtFrustums[4]) / 4.0f;
		D3DXVECTOR3 PtBottomMidPoint = (PtFrustums[5] + PtFrustums[6] + PtFrustums[7] + PtFrustums[8]) / 4.0f;
		D3DXVECTOR3 PtMidLineMidPoint = (PtTopMidPoint + PtBottomMidPoint) / 2.0f;

		// ����ƽ�ƾ���
		D3DXMatrixTranslation(&MatStep, -PtMidLineMidPoint.x, -PtMidLineMidPoint.y, 0);
		MatFinal *= MatStep;

		// ������ת����
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

		// �任Frustum��9���㣬ʹ��Far Plane���£�Near Plane���ϣ���������ƽʱ��ϰ����
		MatFinal *= MatStep;
		D3DXVec3TransformCoordArray(PtFrustum, sizeof(D3DXVECTOR3), PtFrustums, sizeof(D3DXVECTOR3), &MatFinal, 9);



	// �ڶ���������View Frustum��AABB��Χ�У�2D����Ȼ���ҵ���Χ���ε��ĸ�����
		D3DXVECTOR3 PtUpLeft, PtDownRight;
		if(FAILED(D3DXComputeBoundingBox(PtFrustum+1, 8, sizeof(D3DXVECTOR3), &PtUpLeft, &PtDownRight)))
			return E_FAIL;
		// ����ð�Χ�У���ǧ������ˣ�Near Plane���ϣ���ʵ����ΪPP�ռ�Y���Ǵ��µ��ϵ����ģ�����Near Plane��Yֵ�����Ƚϴ���ô��Χ�е�DownRightʵ���ϰ��ŵ���Near Plane������Far
		// �������ǽ���������ֵ��ʹ���������ǵ�ϰ��
		D3DXVECTOR3 fTemp = PtDownRight;
		PtDownRight = PtUpLeft;
		PtUpLeft = fTemp;

		// ��ʱ���εĶ����ױ߶���ƽ����X��ģ�Ҳ���ǰ�Χ�е����±ߣ���ô��������Ҫȷ�������ĸ�����
		// �õ�Frustum����б�ߣ�Nearָ��Far����Ȼ���ҵ����мн����ĽǶȣ�Ȼ���Frustum�������λ�õ㿪ʼ��ƽ���ߣ������Ϳ��Ա�֤��Frustum�����������У������εĽ���Intersect�ͺ�Frustum�������λ����ͬ
		// ���ڵ���������꼴Intersectһ���������棬����������һ�����Σ���������Ҫ���������Top���ϣ�Bottom����
		D3DXVECTOR3 VecSlopeLine[4];
		float fLeftMaxAngle = 2, fRightMaxAngle = 2;	// Max Cos = Min Theta
		int iLeftMaxNo = 0, iRightMaxNo = 0;
		for(i=0; i<4; i++)
		{
			VecSlopeLine[i] = PtFrustum[1+i+4] - PtFrustum[1+i];
			D3DXVec3Normalize(&VecSlopeLine[i], &VecSlopeLine[i]);
		}
		
		// �ҵ��н���󣨼�cosֵ��С���ģ��������������
		for(i=0; i<4; i++)
		{
			// ��н�cos
			fAngle = D3DXVec3Dot(&VecSlopeLine[i], &D3DXVECTOR3(0, -1, 0));
			// ���
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

		// �ҵ��������ļн��ˣ�ok���������������ֱ�ߣ���Ϊ���ε�б�ߣ���ô���ǾͿ��Եõ��Ͷ����ױ߽��㣬Ҳ�������ε��ĸ�����
		PtTrapezoid[0] = PtFrustum[0];	// ����Intersect��Frustum���������һ�µ�
			// ���ױ߶���X��ƽ�У������ĸ����Yֵ�Ͱ�Χ�е����±�Yֵ����һ�£�ֻ����Ҫע��
		PtTrapezoid[1].y = PtTrapezoid[2].y = PtUpLeft.y;
		PtTrapezoid[3].y = PtTrapezoid[4].y = PtDownRight.y;

			// ���ҵ���ߵ�Xֵ�������Ǹ�����Ҫȡ��������ΪFrustum���ڵ�������Y���ϣ���������Ҳ�ǶԳƵģ���ע����������������Ϊ׼�������ģ����Ի�Ҫ����������������ƫ�ƣ���ǰ�����������ԭ�㣩
		PtTrapezoid[1].x = (PtUpLeft.y - PtFrustum[0].y) * tanf(acosf(fLeftMaxAngle)) + PtFrustum[0].x;
		PtTrapezoid[3].x = (PtDownRight.y - PtFrustum[0].y) * tanf(acosf(fLeftMaxAngle)) + PtFrustum[0].x;
			// �ҵ��ұߵ�Xֵ����������ֵ������Ҫȡ������ͬ��
		PtTrapezoid[2].x = -1 * (PtUpLeft.y - PtFrustum[0].y) * tanf(acosf(fRightMaxAngle)) + PtFrustum[0].x;
		PtTrapezoid[4].x = -1 * (PtDownRight.y - PtFrustum[0].y) * tanf(acosf(fRightMaxAngle)) + PtFrustum[0].x;


	// ���һ������Frustum�����α任��LVP�ռ䣬ǰ�����ת��ƽ����Ϊ�˷�������������ױ��õģ����ﻹҪ��ԭ
		D3DXMatrixInverse(&MatFinal, NULL, &MatFinal);
		//D3DXMatrixIdentity(&MatFinal);
		// ת�����ε�5���㲢��Vector 2D�ָ���Vector 3D��Zֵ����Ϊ0����ֻ��View Frustum�ı��ζ��ѣ�������ļ��㶼����2D���Ͻ��еģ���ʵǣ������Zֵ��
		D3DXVec3TransformCoordArray(pPtTrapezoid, sizeof(D3DXVECTOR3), PtTrapezoid, sizeof(D3DXVECTOR3), &MatFinal, 5);
		for(i=0; i<5; i++)
			pPtTrapezoid[i].z = 0;

		
		// ������������⣬�������������������ƽ�У���DOTΪ1��-1������ǿ��USM
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

		// Bottom->Intersect�����Near->Intersect���򣬾�ʧ��
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

		// ��һ���Ƿ�ֹб�ߺ͵ױ߳ʽ���ƽ�е�״̬����͸�������ζ��������ˡ�����
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