#pragma once

#include "myd3d.h"
#include "Shader.h"


// �������ͣ����Σ����Σ�
enum SPRING_TYPE
{
	SPRING_RECTANGLE,
	SPRING_SHEAR,
};

// ��ײ�����ͣ�ƽ�桢���塢��Χ�к������壩
enum COLLIDE_TYPE
{
	COLLIDE_PLANE,			// �����ƽ����һ�����׵������ƽ�棬������׼ȷ�ķ��򣬾��Ǹ��ݷ��ߣ�ֻ�ж������������ײ�����ŷ��ߵ���ײ����������ƽ��ֻ�ܴ�����֮�ϻ�֮�¹�������ײ�����������߶����
							// ��������޴�ƽ�棬���úܱ���BOX��Ellipse������
	COLLIDE_SPHERE,
	COLLIDE_BOX,
	COLLIDE_ELLIPSE,
};




struct COLLIDE_ATTRIBUTE
{
	COLLIDE_TYPE Type;
	// ��������
	D3DXVECTOR3 PtCentre;

	// Sphere�ã��뾶
	float fRadius;

	// Plane�ã�d
	float fd;

	// Plane��Box��Ellipse�ã��ᣨǿ�ƹ�񻯣�����������ֱ�ĳ��ȣ�����ߣ�����������ʾ�����ĵ�������ľ��루�������᳤��һ�룩
	// ��ƽ���У�VecAxisֻ��ǰһ����Ч��������
	D3DXVECTOR3 VecAxis[3];
	float fAxisLength[3];

	COLLIDE_ATTRIBUTE()
	{
		Type = COLLIDE_SPHERE;
		PtCentre = D3DXVECTOR3(0, 0, 0);
		fd = fRadius = 0.0f;

		fAxisLength[0] = fAxisLength[1] = fAxisLength[2] = 0.0f;
		VecAxis[0] = VecAxis[1] = VecAxis[2] = D3DXVECTOR3(0, 0, 0);
	}


	BOOL CheckAvailable()
	{
		if(Type == COLLIDE_BOX || Type == COLLIDE_ELLIPSE)
		{
			for(UINT i = 0; i < 3; i++)
			{
				D3DXVec3Normalize(&VecAxis[i], &VecAxis[i]);
				if(absf(D3DXVec3Length(&VecAxis[i])) < 0.00001f)
				{
					OutputDebugString("������Ϊ0���Ƿ���\n");
					return FALSE;
				}
				if(fAxisLength[i] < 0.0000001f)
				{
					OutputDebugString("�᳤��߱������0��\n");
					return FALSE;
				}
			}
		}
		else if(Type == COLLIDE_SPHERE)
		{
			if(fRadius < 0.0000001f)
			{
				OutputDebugString("�뾶�������0��\n");
				return FALSE;
			}
		}
		else if(Type == COLLIDE_PLANE)
		{
			D3DXVec3Normalize(&VecAxis[0], &VecAxis[0]);
			if(absf(D3DXVec3Length(&VecAxis[0])) < 0.00001f)
			{
				OutputDebugString("ƽ�淨��Ϊ0�������Ƿ���\n");
				return FALSE;
			}
		}
		else
			return FALSE;

		return TRUE;
	}
};






struct CLOTH_ATTRIBUTE
{
	// ����Spring��̬�ĵ�������
	UINT iIterateNum;

	// Mesh����
	UINT iWidth, iHeight;	// ���ӷָ�����һ��ȷ���Ͳ��ɸı�
	float fSquareWidth, fSquareHeight;	// �ܳ����������꣬�ڳ�ʼ����ʱ�����Mesh������ͼ�е�ֵ����ģ����������ֵֻ��Ӱ�쵯����������ʱ����̬���ѣ����Զ�̬�ı�

	// ��������
	float fm;		// ÿ���ʵ�����������ں���һ��������ٶȴ�С
	float fGravity;	// ����������Ĭ��9.8������ض���ֱ���£�-Y�ᣩ������Ϊ0��������������������ת��ɸ�����
	float fK;		// ���ᣬVerlet Intergration���ã�������״̬��1.0�����С��1.0����˵��������
	D3DXVECTOR3 VecWind;	// �����������ȱ�ʾ����ǿ�ȡ�����ж��������������������ͽ����Ƕ��ϳɵ��������������ھֲ������������Ե�����ײ�ﴦ��
	SPRING_TYPE SpringType;	// �������ͣ�������Rectancle���ɣ���������Shear����

	CLOTH_ATTRIBUTE()
	{
		iIterateNum = 0;
		iWidth = iHeight = 0;
		fSquareWidth = fSquareHeight = 0;

		fm = fK = 1.0f;
		fGravity = GravityConstant;
		VecWind = D3DXVECTOR3(0, 0, 0);
		SpringType = SPRING_RECTANGLE;
	}


	BOOL CheckAvailable()
	{
		if(!iWidth || !iHeight)
		{
			OutputDebugString("����ָ�������Ϊ0��\n");
			return FALSE;
		}
		if(fSquareWidth  < 0.0000001f || fSquareHeight < 0.0000001f)
		{
			OutputDebugString("���ϳ���������0��\n");
			return FALSE;
		}
		if(!iIterateNum)
		{
			OutputDebugString("���������������0��\n");
			return FALSE;
		}
		// �����޷�״̬�����Է����Ϊ�������������ж�
		if(fm < 0.0000001f)
		{
			OutputDebugString("�����������0��\n");
			return FALSE;
		}
		if(fK < 0.0000001f || fK > 1.0f)
		{
			OutputDebugString("����ϵ��������0��1֮���������\n");
			return FALSE;
		}

		return TRUE;
	}
};






// ���������ʱ��Ҫ�����������������ֵ��������ţ������Ա���ʹ��MRT
// ��ײ����ʱ����Ҫʹ�÷�֧���ٽ�������MRT��Clothģ���֧��SM3.0
class KClothSimulation
{
public:
	KClothSimulation();
	~KClothSimulation() {Release();}
	void Release();


	// ��ʼ��Clothģ���������ͼ������
	// PtLTPosition��ʾ���ϳ�ʼʱ�����Ͻ���ʼ���λ�ã�VecHorizon��ʾ�᷽�򣨵�Ȼ�ˣ��������������壬��Ҳ���Բ���Y��������������Mesh��Զ��-Y�����죬��Ϊ����������ֱ���µģ�-Y�ᣩ
	// ����ע�⣬����û������任����Ϊ����һ��ƽ�棬����������Զָ��-Y�ᣬ���Ҫ�Բ�������ת��ƽ�Ʋ������������˵�̶�����Ӧ��λ�ã��������Լ��ͻ�����ת��ȥ��
	HRESULT Init(CLOTH_ATTRIBUTE ClothData, D3DXVECTOR3 PtLTPosition, D3DXVECTOR3 VecHorizon);

	// ģ�⣬����ָ�������ͳ�ʼ��ʱ��ͬ������ͷ��ش���
	// ������Ϸ�Χ��С���Ƿֱ��ʣ��ı䣬��ֵ��Ӱ�쵯�������㣬�ǵ�Ҫ�ڼ��㵯����ʱ����Ӧ�Ĵ���
	// Ҫ��¼����һ�ε�ʱ�䣬���ݵ�ǰʱ��ʹ��DeltaTime������ģ�����
	HRESULT ClothSimulation(CLOTH_ATTRIBUTE ClothData, float fDeltaTime, BOOL bGenerateNormal = TRUE, BOOL bGenerateTangent = FALSE);


// Set
	// ���ݵ�ǰ���Ϸ�Χ��С������ָ�������Ͻ�λ�úͺ��᷽����������λ����ͼ
	HRESULT ResetPosition(D3DXVECTOR3 PtLTPosition, D3DXVECTOR3 VecHorizon);

	// ���貼�ϣ��ص���ʼ�����ӣ�����λ����ͼ��ȡ������Fix�㣩
	HRESULT ResetCloth(D3DXVECTOR3 PtLTPosition, D3DXVECTOR3 VecHorizon)
	{
		V_RETURN(ResetPosition(PtLTPosition, VecHorizon));
		V_RETURN(ClearAllFixPoint());
		return S_OK;
	}

	// �û�ǿ���趨һЩ���λ�ã��̶����û��ֶ��ƶ������������飬���Ա���
	// ͬʱ����FixedPoint�����ͼ��Position��ͼ�����pPositionΪ�գ��ͱ�ʾ���е�̶��ڵ�ǰλ�ã�ֻ�ı�Fix��ǣ����ı�Position��ֵ��
	// �ú�����CPUִ�У�����ĺ�ʱ������һ�μ���һֱ��Ч������Fix�ĵ�仯�ˣ���λ��ֵ�����ţ������򾡿����ٵ���
	HRESULT SetFixedPosition(UINT iPointNum, UINT *pX, UINT *pY, LPD3DXVECTOR3 pPosition);

	// ���ָ��Fixed���Fixed״̬
	// ֻ����FixedPoint�����ͼ��Position��Ȼ������ǰ״̬
	// ͬ�ϣ������ڲ�����PositionMap�����Ա������Ч��Ҫ�ߺܶ࣬����һ�μ���һֱ��Ч
	HRESULT FreeFixedPosition(UINT iPointNum, UINT *pX, UINT *pY);

	// ������й̶����״̬��������FixPoint��ͼ��Position��Ȼ������ǰ״̬��
	HRESULT ClearAllFixPoint();


	// ������ײ����ж��
	HRESULT SetCollides(UINT iCollideNum, COLLIDE_ATTRIBUTE *pCollideData);



// Get
public:
	// Get��������Simulate���У�ֻ�ǵõ�ָ�룬������������
	HRESULT GetPositionMap(LPDIRECT3DTEXTURE9 *pTexPosition, LPDIRECT3DTEXTURE9 *pTexPosition_Sign)
	{
		if(m_iCreateAttrib != 2)
			return D3DERR_NOTAVAILABLE;
		*pTexPosition = m_pTexNextP;
		*pTexPosition_Sign =  m_pTexNextP_Sign;
		return S_OK;
	}
	LPDIRECT3DTEXTURE9 GetNormalMap() 
	{
		if(m_iCreateAttrib != 2 || !m_bGenerateNormal)
			return NULL;
		return m_pTexNormal;
	}
	LPDIRECT3DTEXTURE9 GetTangentMap() 
	{
		if(m_iCreateAttrib != 2 || !m_bGenerateTangent)
			return NULL;
		return m_pTexTangent;
	}


	// ��������λ�õ�С���裬ÿ�����趼��NewP��Ϊ���룬������������NewP��
private:
	// Verlet Intergration�����������������ͷ�������һ�����
	HRESULT ApplyForce();
	// Spring
	HRESULT ApplySpring();
	// Collision��C++��ѭ����ÿ��ִ��Shader��һ��Collide�����
	HRESULT ApplyCollision();

private:
	HRESULT InitSpringPairTexture();						// ��ʼ�����ŵ�����Ե���ͼ

	PIXELSHADER *SelectCollideShader(COLLIDE_TYPE Type);	// ��������ѡ����ײ�õ�PixelShader�������Ĵ���
	HRESULT SetCollideConstant( UINT iCollideNo );

	HRESULT CopyTexture(LPDIRECT3DTEXTURE9 pTexDst, LPDIRECT3DTEXTURE9 pTexSrc)
	{
		return CommonComputeQuad(pTexSrc, NULL, NULL, NULL, pTexDst, NULL, &m_PSCopyTexture);
	}

	// FixPoint��ͼ��Զ�ڵ�һ�㣬�����������4����ͼ
	HRESULT CommonComputeQuad(LPDIRECT3DTEXTURE9 pSrcTex1, LPDIRECT3DTEXTURE9 pSrcTex2, LPDIRECT3DTEXTURE9 pSrcTex3, LPDIRECT3DTEXTURE9 pSrcTex4, LPDIRECT3DTEXTURE9 pRT1,LPDIRECT3DTEXTURE9 pRT2, PIXELSHADER *pPS);


private:
	UINT m_iCreateAttrib;
	BOOL m_bGenerateNormal;	// �Ƿ���GPU���ɷ��ߺ�����ͼ������֧��PS2.a��
	BOOL m_bGenerateTangent;

	float m_fDeltaTime;

	CLOTH_ATTRIBUTE m_ClothData;

	COLLIDE_ATTRIBUTE *m_pCollideData;
	UINT m_iCollideNum;

	// ���㻺��
	struct QUADVERTEXTYPE
	{
		float x,y,z;
		float tu,tv,tw,tx;	//������δ�ã�ֻ��Ϊ��ȥ��PS����Ĵ���
	};
	VERTEXSHADER m_VS;
	UINT m_iStride;
	LPDIRECT3DVERTEXBUFFER9 m_pVBQuad;

	LPDIRECT3DSURFACE9 m_pDepthBuffer;


	// ��ͼ����
public:
	// ÿһ�����Ǽ��㵽NextP�У�ֱ�����ռ��������λ��������Ȼ�����NextP�У�ģ�⿪ʼҲ�ǽ�������NowȻ����Ϊ����
	LPDIRECT3DTEXTURE9 m_pTexPrevP, m_pTexNowP, m_pTexNextP;	// ����Verlet���֣��õ���ǰ��λ�ã���ʼ�����ŵ�λ�õ����궼��ͬ
	LPDIRECT3DTEXTURE9 m_pTexPrevP_Sign, m_pTexNowP_Sign, m_pTexNextP_Sign;

	// ���ߺ����ߣ����ǵķ�Χ����0��1֮�䣬���ü�Sign����
	LPDIRECT3DTEXTURE9 m_pTexNormal, m_pTexTangent;

	// ��ʱʹ�������ɡ���ײ�����ã���CPU����Position��
	LPDIRECT3DTEXTURE9 m_pTexTemp[2], m_pTexTemp_Sign[2], m_pTexTempCPU[2];

	// CPU����
		// ������Ե���Ϣ��2ͨ����������rg��ź͵�ǰ����Եĵ���λ�ã��������꣩����CPU��ʼ��һ�μ��ɣ��ֱ��������ֲ�ͬ���͵ĵ��ɣ�4�ַ�����������
		// 0��3�ֱ�Ϊż���X��������������б���򣩡������X������б����ż���Y������б���������Y������б��
	LPDIRECT3DTEXTURE9 m_pTexSpringPairRectangle[4], m_pTexSpringPairShear[4];
	LPDIRECT3DTEXTURE9 m_pTexFixedPoint;				// ���ɵ�ϵ�����̶���Ϊ0.0f�����ɵ�Ϊ1.0f����ֻ��һ��alphaͨ����D3DFMT_A8



	// Shader����
private:
	PIXELSHADER m_PSSetToZero, m_PSCopyTexture;

	PIXELSHADER m_PSInitPosition;				// ���ݲ��Ϸ�Χ��С��ʼ��λ����ͼ

	PIXELSHADER m_PSApplyForce;					// ������ֻ���������ͷ������������ľֲ�������ת��Ϊ��ײ�
	PIXELSHADER m_PSApplySpring;				// ������������������
	PIXELSHADER m_PSGenerateNormal;				// ���ɷ���
	PIXELSHADER m_PSGenerateNormalandTangent;	// ���ɷ��ߺ�����

	PIXELSHADER m_PSCollide_Plane;		// ƽ����ײ����
	PIXELSHADER m_PSCollide_Sphere;		// ������ײ����
	PIXELSHADER m_PSCollide_Box;		// ������ײ����
	PIXELSHADER m_PSCollide_Ellipse;	// ��������ײ����
};
