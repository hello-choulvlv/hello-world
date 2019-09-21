#pragma once

#include "myd3d.h"
#include "Shader.h"


// 弹簧类型（矩形，菱形）
enum SPRING_TYPE
{
	SPRING_RECTANGLE,
	SPRING_SHEAR,
};

// 碰撞物类型（平面、球体、包围盒和椭球体）
enum COLLIDE_TYPE
{
	COLLIDE_PLANE,			// 这里的平面是一个简易的无穷大平面，而且有准确的方向，就是根据法线，只判定逆向而来的碰撞，沿着法线的碰撞不处理。所以平面只能处理它之上或之下过来的碰撞，不可能两边都兼顾
							// 如果是有限大平面，就用很薄的BOX或Ellipse来代替
	COLLIDE_SPHERE,
	COLLIDE_BOX,
	COLLIDE_ELLIPSE,
};




struct COLLIDE_ATTRIBUTE
{
	COLLIDE_TYPE Type;
	// 中心坐标
	D3DXVECTOR3 PtCentre;

	// Sphere用，半径
	float fRadius;

	// Plane用，d
	float fd;

	// Plane、Box和Ellipse用，轴（强制规格化）方向及三个轴分别的长度（长宽高），这里的轴表示从中心到四周面的距离（即真正轴长的一半）
	// 在平面中，VecAxis只有前一项有效，即法向
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
					OutputDebugString("轴向量为0！非法！\n");
					return FALSE;
				}
				if(fAxisLength[i] < 0.0000001f)
				{
					OutputDebugString("轴长宽高必须大于0！\n");
					return FALSE;
				}
			}
		}
		else if(Type == COLLIDE_SPHERE)
		{
			if(fRadius < 0.0000001f)
			{
				OutputDebugString("半径必须大于0！\n");
				return FALSE;
			}
		}
		else if(Type == COLLIDE_PLANE)
		{
			D3DXVec3Normalize(&VecAxis[0], &VecAxis[0]);
			if(absf(D3DXVec3Length(&VecAxis[0])) < 0.00001f)
			{
				OutputDebugString("平面法向为0向量！非法！\n");
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
	// 计算Spring稳态的迭代次数
	UINT iIterateNum;

	// Mesh属性
	UINT iWidth, iHeight;	// 格子分割数，一旦确定就不可改变
	float fSquareWidth, fSquareHeight;	// 总长宽，世界坐标，在初始化的时候决定Mesh的坐标图中的值，在模拟过程中这个值只是影响弹簧内力计算时的稳态而已，可以动态改变

	// 物理属性
	float fm;		// 每个质点的质量，用于和力一起决定加速度大小
	float fGravity;	// 重力常量，默认9.8，方向必定竖直向下（-Y轴），可以为0（无重力环境）或负数（转变成浮力）
	float fK;		// 阻尼，Verlet Intergration中用，无阻尼状态是1.0，如果小于1.0，就说明有阻尼
	D3DXVECTOR3 VecWind;	// 风向，向量长度表示风力强度。如果有多个方向的整体作用力，就将它们都合成到风向中来；至于局部作用力，可以当作碰撞物处理
	SPRING_TYPE SpringType;	// 弹簧类型：方格型Rectancle弹簧，还是菱形Shear弹簧

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
			OutputDebugString("网格分割数不能为0！\n");
			return FALSE;
		}
		if(fSquareWidth  < 0.0000001f || fSquareHeight < 0.0000001f)
		{
			OutputDebugString("布料长宽必须大于0！\n");
			return FALSE;
		}
		if(!iIterateNum)
		{
			OutputDebugString("迭代次数必须大于0！\n");
			return FALSE;
		}
		// 允许无风状态，所以风可以为零向量，不做判断
		if(fm < 0.0000001f)
		{
			OutputDebugString("质量必须大于0！\n");
			return FALSE;
		}
		if(fK < 0.0000001f || fK > 1.0f)
		{
			OutputDebugString("阻尼系数必须是0～1之间的正数！\n");
			return FALSE;
		}

		return TRUE;
	}
};






// 由于输出的时候要用两张纹理（坐标绝对值、坐标符号），所以必须使用MRT
// 碰撞检测的时候需要使用分支，再结合上面的MRT，Cloth模拟仅支持SM3.0
class KClothSimulation
{
public:
	KClothSimulation();
	~KClothSimulation() {Release();}
	void Release();


	// 初始化Cloth模拟所需的贴图和数据
	// PtLTPosition表示布料初始时，左上角起始点的位置，VecHorizon表示横方向（当然了，布料是柔性物体，它也可以不和Y轴正交），布料Mesh永远和-Y面延伸，因为重力都是竖直向下的（-Y轴）
	// 另外注意，布料没有世界变换：因为它是一个平面，而且竖轴永远指向-Y轴，如果要对布料做旋转或平移操作，就让两端点固定在相应的位置，这样它自己就会整个转过去了
	HRESULT Init(CLOTH_ATTRIBUTE ClothData, D3DXVECTOR3 PtLTPosition, D3DXVECTOR3 VecHorizon);

	// 模拟，网格分割数必须和初始化时相同，否则就返回错误
	// 如果布料范围大小（非分辨率）改变，该值仅影响弹簧力计算，记得要在计算弹簧力时做相应的处理
	// 要记录下上一次的时间，根据当前时间使用DeltaTime来计算模拟过程
	HRESULT ClothSimulation(CLOTH_ATTRIBUTE ClothData, float fDeltaTime, BOOL bGenerateNormal = TRUE, BOOL bGenerateTangent = FALSE);


// Set
	// 根据当前布料范围大小、参数指定的左上角位置和横轴方向，重置两张位置贴图
	HRESULT ResetPosition(D3DXVECTOR3 PtLTPosition, D3DXVECTOR3 VecHorizon);

	// 重设布料，回到初始的样子（重置位置贴图，取消所有Fix点）
	HRESULT ResetCloth(D3DXVECTOR3 PtLTPosition, D3DXVECTOR3 VecHorizon)
	{
		V_RETURN(ResetPosition(PtLTPosition, VecHorizon));
		V_RETURN(ClearAllFixPoint());
		return S_OK;
	}

	// 用户强制设定一些点的位置（固定或用户手动移动），三个数组，各自保存
	// 同时更新FixedPoint标记贴图和Position贴图，如果pPosition为空，就表示所有点固定在当前位置（只改变Fix标记，不改变Position的值）
	// 该函数用CPU执行，极其的耗时，设置一次即可一直生效。除非Fix的点变化了（其位置值或点序号），否则尽可能少调用
	HRESULT SetFixedPosition(UINT iPointNum, UINT *pX, UINT *pY, LPD3DXVECTOR3 pPosition);

	// 解除指定Fixed点的Fixed状态
	// 只更新FixedPoint标记贴图，Position仍然保留当前状态
	// 同上，但由于不考虑PositionMap，所以比上面的效率要高很多，设置一次即可一直生效
	HRESULT FreeFixedPosition(UINT iPointNum, UINT *pX, UINT *pY);

	// 清除所有固定点的状态（仅更新FixPoint贴图，Position仍然保留当前状态）
	HRESULT ClearAllFixPoint();


	// 设置碰撞物，可有多个
	HRESULT SetCollides(UINT iCollideNum, COLLIDE_ATTRIBUTE *pCollideData);



// Get
public:
	// Get，必须先Simulate才行，只是得到指针，并不复制数据
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


	// 三个计算位置的小步骤，每个步骤都以NewP作为输入，并计算好输出到NewP中
private:
	// Verlet Intergration计算外力，将重力和风力合在一起计算
	HRESULT ApplyForce();
	// Spring
	HRESULT ApplySpring();
	// Collision，C++中循环，每次执行Shader对一个Collide做检测
	HRESULT ApplyCollision();

private:
	HRESULT InitSpringPairTexture();						// 初始化两张弹簧配对点贴图

	PIXELSHADER *SelectCollideShader(COLLIDE_TYPE Type);	// 根据类型选择碰撞用的PixelShader及常量寄存器
	HRESULT SetCollideConstant( UINT iCollideNo );

	HRESULT CopyTexture(LPDIRECT3DTEXTURE9 pTexDst, LPDIRECT3DTEXTURE9 pTexSrc)
	{
		return CommonComputeQuad(pTexSrc, NULL, NULL, NULL, pTexDst, NULL, &m_PSCopyTexture);
	}

	// FixPoint贴图永远在第一层，后面可以设置4层贴图
	HRESULT CommonComputeQuad(LPDIRECT3DTEXTURE9 pSrcTex1, LPDIRECT3DTEXTURE9 pSrcTex2, LPDIRECT3DTEXTURE9 pSrcTex3, LPDIRECT3DTEXTURE9 pSrcTex4, LPDIRECT3DTEXTURE9 pRT1,LPDIRECT3DTEXTURE9 pRT2, PIXELSHADER *pPS);


private:
	UINT m_iCreateAttrib;
	BOOL m_bGenerateNormal;	// 是否用GPU生成法线和切线图（必须支持PS2.a）
	BOOL m_bGenerateTangent;

	float m_fDeltaTime;

	CLOTH_ATTRIBUTE m_ClothData;

	COLLIDE_ATTRIBUTE *m_pCollideData;
	UINT m_iCollideNum;

	// 顶点缓冲
	struct QUADVERTEXTYPE
	{
		float x,y,z;
		float tu,tv,tw,tx;	//后两个未用，只是为了去掉PS编译的错误
	};
	VERTEXSHADER m_VS;
	UINT m_iStride;
	LPDIRECT3DVERTEXBUFFER9 m_pVBQuad;

	LPDIRECT3DSURFACE9 m_pDepthBuffer;


	// 贴图部分
public:
	// 每一步都是计算到NextP中，直到最终计算出来的位置坐标仍然存放在NextP中，模拟开始也是将它换到Now然后作为输入
	LPDIRECT3DTEXTURE9 m_pTexPrevP, m_pTexNowP, m_pTexNextP;	// 用于Verlet积分，得到当前点位置，初始，三张的位置点坐标都相同
	LPDIRECT3DTEXTURE9 m_pTexPrevP_Sign, m_pTexNowP_Sign, m_pTexNextP_Sign;

	// 法线和切线，它们的范围都是0～1之间，不用加Sign修正
	LPDIRECT3DTEXTURE9 m_pTexNormal, m_pTexTangent;

	// 临时使用做弹簧、碰撞迭代用，及CPU更新Position用
	LPDIRECT3DTEXTURE9 m_pTexTemp[2], m_pTexTemp_Sign[2], m_pTexTempCPU[2];

	// CPU控制
		// 弹簧配对点信息，2通道浮点纹理，rg存放和当前点配对的弹簧位置（纹理坐标）。用CPU初始化一次即可，分别用于两种不同类型的弹簧，4种方向用来迭代
		// 0～3分别为偶序号X方向（菱形中是左斜方向）、奇序号X方向（左斜）、偶序号Y方向（右斜）、奇序号Y方向（右斜）
	LPDIRECT3DTEXTURE9 m_pTexSpringPairRectangle[4], m_pTexSpringPairShear[4];
	LPDIRECT3DTEXTURE9 m_pTexFixedPoint;				// 自由点系数（固定点为0.0f，自由点为1.0f），只有一个alpha通道，D3DFMT_A8



	// Shader部分
private:
	PIXELSHADER m_PSSetToZero, m_PSCopyTexture;

	PIXELSHADER m_PSInitPosition;				// 根据布料范围大小初始化位置贴图

	PIXELSHADER m_PSApplyForce;					// 外力（只包含重力和风力，看不见的局部力可以转换为碰撞物）
	PIXELSHADER m_PSApplySpring;				// 弹簧内力，迭代过程
	PIXELSHADER m_PSGenerateNormal;				// 生成法线
	PIXELSHADER m_PSGenerateNormalandTangent;	// 生成法线和切线

	PIXELSHADER m_PSCollide_Plane;		// 平面碰撞处理
	PIXELSHADER m_PSCollide_Sphere;		// 球体碰撞处理
	PIXELSHADER m_PSCollide_Box;		// 盒子碰撞处理
	PIXELSHADER m_PSCollide_Ellipse;	// 椭球体碰撞处理
};
