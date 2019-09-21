#pragma once

//�����Ǹ�����������ƵĶ��壬
typedef enum CAMERACHANGEWAY
{
	CAM_NOCHANGE	=	0,		// δ�ƶ�
	CAM_GOFORWARD	=	1,		//ǰ������
	CAM_GOBACKWARD	=	2,
	CAM_GOLEFT		=	4,		//����ƽ��
    CAM_GORIGHT		=	8,
	CAM_GOUP		=	16,		//����ƽ��
	CAM_GODOWN		=	32,
	CAM_TURNLEFT	=	64,		//����ת���ӽ�
	CAM_TURNRIGHT	=	128,
	CAM_TURNUP		=	256,	//����ת���ӽ�
	CAM_TURNDOWN	=	512,
	CAM_SLICELEFT	=	1024,	//���ҵ���
	CAM_SLICERIGHT	=	2048,
}CameraChangeWay;


class CAMERACHANGE
{
public:
	D3DXVECTOR3 Eye;   //���������λ��
	D3DXVECTOR3 Look;     //���߷���㣨�������������������ֵ����VecLook+Eye��
	float m_fProjXYCoef;		// XY��ı���ϵ����PROJ�任��

	CAMERACHANGE();
	// ��ʼ���������EYE��ʾ�����λ�����꣬LOOK��ʾ���ߵ����꣬move��ÿ���ƶ�(ǰ��/����)�ľ��룬rise��ÿ��ƽ��(����/����)�ľ��룬angle��ÿ��ת��ĽǶȣ�Ϊ�˷��㣬���뻡��Ҳ���Զ�ƥ�䣩
	// �ɶ�γ�ʼ��������ǿ�Ƹı������λ��
	HRESULT InitCamera(float Eyex, float Eyey, float Eyez, float Lookx, float Looky, float Lookz, float Headx, float Heady, float Headz, float move, float rise, float angle);
	//changeway��ʾ��ͷ�ı�ķ�ʽ���������DEFINE�����Զ��ַ�ʽһ����У���ǰ�Ƽ���ת����|����
	// MoveCoef��ʾ�ƶ��ٶȣ����������ƣ���������ƶ��ľ��������ƻ�׼�ٶȣ�Initʱ�������ٶȣ��ı���
	void ChangeCamera(unsigned short int changeway, float fMoveCoef = 1.0f);
	// ��Slice��TurnUp/Down�ĽǶȹ̶�����������˵��Head��Y�غϣ���Look��Right_X��XZ����
	void ResetCamera();
	
	//���ݵ�ǰ����myproj��myview���й۲�任�ͼ��ñ任������ʹ�õĺ���
	void ViewTransform();
	void ProjTransform(float xycoef = 0);     //������XY��ı���ϵ����Ĭ��Ϊ��Ļ�ֱ����ݺ���֮��

	// �õ�View Frustum9������������ռ��е����ֻ꣬������͸��ͶӰ��Frustum�����Ҵ����ָ���������9��VECTOR����0��8�ֱ���Eye��Near�����������������¡�Far������������������
	HRESULT GetEyeFrustum(D3DXVECTOR3 *PtFrustum, float fNear, float fFar);

	void SetFrameTime();	// ���ñ�֡ʱ�䣬ÿ֡��Ҫ���У��ڲ�������ֻ��UserControl��ʹ��

	float GetFrameTime();	// �õ���һ֡��Ⱦǰ����֡��Ⱦǰ��ʱ��������λ�룩������Present�����������ƶ���������Ӧ��������ʱ����صĲ���



	float GetIdleTime();	// �õ��Ѿ�Idle�˶�ã���λ�룩�������ǰ���ƶ�״̬������0.0
	
	bool IsIdle() { return m_MoveAttrib==0 ? true : false; }			// ��ѯ�Ƿ��ھ�ֹ״̬
	bool IsMoving() { return m_MoveAttrib==1? true : false; }			// ��ѯ�Ƿ����ƶ�״̬
	bool IsIdleToMoving() { return m_MoveAttrib==2 ? true : false; }	// ��ѯ�Ƿ�մӾ�ֹ���ƶ�
	bool IsMovingToIdle() { return m_MoveAttrib==3 ? true : false; }	// ��ѯ�Ƿ�մ��ƶ�����ֹ

private:
	D3DXVECTOR3 m_VecLook;   //���߷�������
	D3DXVECTOR3 m_VecHead;   //ͷ����������
	D3DXVECTOR3 m_VecRight_X;	// X��������������������Look���һ�������ռ�
	
	bool m_bInit;		// �Ƿ��ѳ�ʼ��
	bool m_bIdle;	// true��ʾIdle״̬��false��ʾ�ƶ�״̬
	BYTE m_MoveAttrib;	// 0��ʾIdle��1��ʾMoving��2��ʾ�մ�Idle��Moving��3��ʾ�մ�Moving��Idle
	DWORD dwLastIdleTime;	// �ϴ�IDLE��ʱ�䣬���ڼ�������������Ѿ�Idle�˶�ã�����HDR��
	DWORD dwCurrentTime, dwLastFrameTime;	// ��ǰʱ�����һ֡��ʱ�䣨����������ʵʱ�䣬������֮֡���ʱ��
	DWORD dwTempLastFrameTime;	// ��ʱʹ�ã�����GetFrameTime
	
	float m_fMoveSpeed;		// ǰ��ʱÿ���ƶ����루��������ϵ�У�
	float m_fRiseSpeed;		// �����½�ʱÿ���ƶ����루��������ϵ�У�
	float m_fAngleSpeed;	// ת��ʱÿ��ת���ĽǶ�(����)
	
	bool m_bYMove;	// ǰ�������ƽ��ʱ�Ƿ���Y�ᣬ�翼����������ߺ�Y��нǵ����ƶ��ĸ߶ȣ��粻������ֻ����һ����XZƽ��ƽ�е�ƽ�����ƶ�����ʼΪ�رգ������ǣ�
};








// �����ֻ�Ǽ򻯲������ˣ�����ǣ����Shader
class KTrapezoidalShadowMap
{
public:
	KTrapezoidalShadowMap();
	~KTrapezoidalShadowMap();
	
	// ��ʼ��ת����ͼ��������Ӱ��ͼ
	HRESULT Init(UINT iWidth = 512, UINT iHeight = 0);
	void Release();
	
	// ���õڼ�����Ӱ��ͼΪRT��׼��������Ⱦ��
	HRESULT RenderShadowTexture(UINT iShadowMapNo = 0);
	// ��Ⱦ����Ӱ��ͼ֮��һ��Ҫ�ָ��ɵ�RT����Ȼ���
	HRESULT RestoreRenderTarget();
	// ������Ⱦ�õ���Ӱ��ͼ��׼����Ⱦ������Ӱ��ע�⿼�ǵ�ת������������Ҫռ������ͼ
	HRESULT SetShadowTexture(DWORD dwStage);

	// ���ù�Դ���ԣ�����λ�ã�ͬʱ����LVP��VIEW��PROJ����
	// fMinRange, fMaxRange�ǹ�ԴZ�᷶Χ��С����LVPԶ�������棩��fX/YRangeCoef�ǹ�ԴX��Y��ķ�Χ��Сϵ������ʼ״̬ʱ���ݹ�Դ���Ͳ�ͬ�ֱ�ΪPI/2,PI/2��͸�ӣ���2.0,2.0��������
	HRESULT SetLight(D3DXVECTOR3 PtLightPos, D3DXVECTOR3 VecLightDirection, D3DXVECTOR3 VecHeadDirection, float fMinRange = 1.0f, float fMaxRange = 500.0f, D3DLIGHTTYPE LightType = D3DLIGHT_DIRECTIONAL, float fXRangeCoef = 1.0f, float fYRangeCoef = 1.0f);

	// ��������EF��Զ�������棬���ڽ�����������С��Զ����Ӱ�����½������⣬Ĭ����2��200
	HRESULT SetFrustumRange(float fNear, float fFar);
	
	// �õ�WORLD->LVP�ռ�VIEW*PROJ�����WORLD->T�ռ�ת��������Ϻõ���������ֱ�ӿ���ʹ�ã���T�ռ������봫���������ָ���������Ϊ���ǵ��������ߺ͹���ƽ�е����⣬����Ҫ�õ�4�Ų�ͬ��ShadowMap
	HRESULT GetMatrix(D3DXMATRIX *pMatLVPProj, D3DXMATRIX pMatTSpace[4]);
	
	
	
	// �Ƿ�ǿ��USM����T����ǿ��ΪIdentity���������������Problem��������ǿ��ΪUSM
	bool m_bForceUSM;
	// ���LVP�ռ��У�EYE�ܵ�Far Plane����ȥ�ˣ����øñ�ǣ����������4��ShadowMaps��������Ϊ�˷���ֱ�ӽ�Far Plane��Ϊ���δ���
	bool m_bFourShadowMaps;
	

	
	
//������Щ�����ڲ����������������Ƶ�private��ȥ
	// �õ���LVP�ռ䵽Trapezoidal�ռ��ת�����󣬽�LVP��>T
	HRESULT GetTSpaceMatrix(D3DXMATRIX *pMatTSpace);
	
	// ��World�ռ��Eye FrustumͶӰ��LVP�ռ�ȥ
	void ViewFrustumToLVPSpace(D3DXVECTOR3 *PtFrustum);
	// ͨ��LVPͶӰ���Eye Frustum���������
	HRESULT FrustumToTrapezoid(D3DXVECTOR3 *pPtFrustum, D3DXVECTOR3 *pPtTrapezoid);
	// �����δ�LVP�ռ�任��T-SPACE���������յ�ת������
	D3DXMATRIX TransformToTrapezoidSpace(D3DXVECTOR3 PtTopL, D3DXVECTOR3 PtTopR, D3DXVECTOR3 PtBottomL, D3DXVECTOR3 PtBottomR, D3DXVECTOR3 PtIntersect);
	
	// ��Դ���Լ�����
	D3DXVECTOR3 m_VecLightDirection, m_VecHeadDirection, m_VecRightDirection;
	D3DXVECTOR3 m_PtLightPosition;
	D3DXMATRIX m_MatLVPProj, m_MatLVPView;
private:
	// ����ת����ͼ�Ƿ�ɹ��������Ƿ���Ⱦ����Ӱ��ͼ��1��ʾδ��Ⱦ����Ӱ��ͼ��2��ʾ���ɹ�
	UINT m_iCreateAttrib;
	
	// ��Դ���ͣ���ʱֻ֧��SPOT��DIRECTIONAL�����������;�����LVP��ͶӰ��������
	D3DLIGHTTYPE m_LightType;
	
	// ��TSM��Ӧ����EFԶ������������
	float m_fVirtualEFNearPlane, m_fVirtualEFFarPlane;

	// �¾���Ȼ���ͺ�̨���壬����״̬����ʱʹ��
	LPDIRECT3DSURFACE9 m_pOldBackBuffer, m_pOldDepthBuffer, m_pDepthBuffer;
	
	// ��Ӱ��ͼ
	LPDIRECT3DTEXTURE9 m_pTexShadowMap[4];
};