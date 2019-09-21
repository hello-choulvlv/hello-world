#ifndef __USE_CONTROL_H__
#define __USE_CONTROL_H__
#include<windows.h>
enum MOUSEKEYNAME
{
	MKN_LEFT,
	MKN_RIGHT,
	MKN_MIDDLE,
};

enum KEYATTRIBUTE
{
	KA_UNUSED,		// 其实是属于持续抬起状态，但这时是没有任何作用的，等于没有用到这个键
	KA_RELEASE,		// 第一次抬起，表示按下后的抬起行为，即释放按键，瞬时数据，下来就变为Unused
	KA_DOWN,		// 出现第一次按下的行为，瞬时数据，下来应马上变为DownHold
	KA_DOWNHOLD,	// DownHold表示持续Down，即按下不放（单击Hold或双击Hold），如果这时鼠标移动，就是Drag，注意不会有UpHold
	KA_DOUBLECLICK,	// 双击
};

class KMouseControl
{
public:
	KMouseControl()
	{
		m_CurrentPos.x = m_LastPos.x = 0;
		m_CurrentPos.y = m_LastPos.y = 0;
		m_iCurrentMidAxis = m_iLastMidAxis = 0;
		m_LeftButton = m_RightButton = m_MidButton = KA_UNUSED;
		m_bShowMouseCursor = FALSE;
	}
	// 设置当前坐标，这两个值是相对窗口的偏移（客户区坐标）
	void SetPosition(int iX, int iY)
	{
		// 这是表示第一次设定坐标，两个坐标设置为相同，免得产生错误的位移
		if(m_CurrentPos.x == 0 && m_LastPos.x == 0 && m_CurrentPos.y == 0 && m_LastPos.y == 0)
		{
			m_CurrentPos.x = m_LastPos.x = iX;
			m_CurrentPos.y = m_LastPos.y = iY;
		}
		else
		{
			m_LastPos.x = m_CurrentPos.x;
			m_LastPos.y = m_CurrentPos.y;
			m_CurrentPos.x = iX;
			m_CurrentPos.y = iY;
		}
	}


	// 设置鼠标指针显示与否，如果不显示，就要用鼠标指针控制移动
	void SetShowCursor(BOOL bShowCursor)
	{
		m_bShowMouseCursor = bShowCursor;
		ShowCursor(bShowCursor);
	}


	// 设置中轴坐标，即WM_MOUSEWHEEL对应的消息，(short) HIWORD (wParam)，注意这个值会比较大，滚轮每单位以成百递增
	void SetMidAxis(short int iDelta)
	{
		m_iLastMidAxis = m_iCurrentMidAxis;
		m_iCurrentMidAxis += iDelta;
	}


	// 设置按键状态，第一个参数事实上是窗口消息类型，第二个参数是附加的选项，可以检查是否同时按下了系统键，和MK_与操作即可判断
	void SetKeyAttribute(LONG lCurrentKeyType, WPARAM wParam)
	{
		switch(lCurrentKeyType)
		{
		case WM_LBUTTONUP:
			// Win消息是不会像Async一样连续UP的，所以如果上回的消息是Down/Hold，那么再次UP就说明释放掉按钮了，即未用
			if(m_LeftButton == KA_DOWN || m_LeftButton == KA_DOWNHOLD || m_LeftButton == KA_DOUBLECLICK)
				m_LeftButton = KA_RELEASE;
			break;
		case WM_LBUTTONDOWN:
			// 按下，强制转到按下状态，瞬时数据
			m_LeftButton = KA_DOWN;
			break;
		case WM_LBUTTONDBLCLK:
			m_LeftButton = KA_DOUBLECLICK;
			break;

		case WM_RBUTTONUP:
			if(m_RightButton == KA_DOWN || m_RightButton == KA_DOWNHOLD || m_RightButton == KA_DOUBLECLICK)
				m_RightButton = KA_RELEASE;
			break;
		case WM_RBUTTONDOWN:
			m_RightButton = KA_DOWN;
			break;
		case WM_RBUTTONDBLCLK:
			m_RightButton = KA_DOUBLECLICK;
			break;

		case WM_MBUTTONUP:
			if(m_MidButton == KA_DOWN || m_MidButton == KA_DOWNHOLD || m_MidButton == KA_DOUBLECLICK)
				m_MidButton = KA_RELEASE;
			break;
		case WM_MBUTTONDOWN:
			m_MidButton = KA_DOWN;
			break;
		case WM_MBUTTONDBLCLK:
			m_MidButton = KA_DOUBLECLICK;
			break;
		}
	}


	void Breath()
	{
		// 所有的瞬时数据，都清除掉
		if(m_LeftButton == KA_RELEASE)
			m_LeftButton = KA_UNUSED;
		if(m_LeftButton == KA_DOWN)
			m_LeftButton = KA_DOWNHOLD;

		if(m_RightButton == KA_RELEASE)
			m_RightButton = KA_UNUSED;
		if(m_RightButton == KA_DOWN)
			m_RightButton = KA_DOWNHOLD;

		if(m_MidButton == KA_RELEASE)
			m_MidButton = KA_UNUSED;
		if(m_MidButton == KA_DOWN)
			m_MidButton = KA_DOWNHOLD;


		// Last和Current只用于临时读取移动距离，所以每帧都必须恢复，否则鼠标不移动时，将得不到WM_MouseMove消息，无法SetPosition，距离差无法及时消除
		// 如果显示鼠标指针，就不可以移动
		if(m_bShowMouseCursor)
		{
			m_LastPos.x = m_CurrentPos.x;
			m_LastPos.y = m_CurrentPos.y;
			ShowCursor(TRUE);
		}
		// 不显示鼠标指针，表示需要用鼠标控制移动，那么移动一次就要把鼠标指针设回原位，这样才可以无限移动
		else
		{
			if(m_LastPos.x != m_CurrentPos.x || m_LastPos.y != m_CurrentPos.y)
			{
				POINT PtMouse = m_LastPos;
				ClientToScreen(hWnd, &PtMouse);
				SetCursorPos(PtMouse.x, PtMouse.y);
			}
			m_CurrentPos.x = m_LastPos.x;
			m_CurrentPos.y = m_LastPos.y;
			ShowCursor(FALSE);
		}
	}


	KEYATTRIBUTE GetKeyAttribute(MOUSEKEYNAME Key)
	{
		if(Key == MKN_LEFT)
			return m_LeftButton;
		if(Key == MKN_MIDDLE)
			return m_MidButton;
		if(Key == MKN_RIGHT)
			return m_RightButton;

		return KA_UNUSED;
	}

	POINT GetMouseMove()
	{
		POINT MoveDistance, ZeroDistance;
		MoveDistance.x = m_CurrentPos.x - m_LastPos.x;
		MoveDistance.y = m_CurrentPos.y - m_LastPos.y;
		if(m_bShowMouseCursor)
		{
			ZeroDistance.x = ZeroDistance.y = 0;
			return ZeroDistance;
		}
		return MoveDistance;
	}


private:
	POINT m_CurrentPos, m_LastPos;			// 鼠标坐标，通过这两个来计算当前移动差
	int m_iCurrentMidAxis, m_iLastMidAxis;	// 中间轴的位置坐标
	KEYATTRIBUTE m_LeftButton, m_RightButton, m_MidButton;	// 三个按键的状态
	BOOL m_bShowMouseCursor;				// 是否显示鼠标指针，如果显示，就不可以移动
};

extern KMouseControl g_MouseControl;

#endif