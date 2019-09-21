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
	KA_UNUSED,		// ��ʵ�����ڳ���̧��״̬������ʱ��û���κ����õģ�����û���õ������
	KA_RELEASE,		// ��һ��̧�𣬱�ʾ���º��̧����Ϊ�����ͷŰ�����˲ʱ���ݣ������ͱ�ΪUnused
	KA_DOWN,		// ���ֵ�һ�ΰ��µ���Ϊ��˲ʱ���ݣ�����Ӧ���ϱ�ΪDownHold
	KA_DOWNHOLD,	// DownHold��ʾ����Down�������²��ţ�����Hold��˫��Hold���������ʱ����ƶ�������Drag��ע�ⲻ����UpHold
	KA_DOUBLECLICK,	// ˫��
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
	// ���õ�ǰ���꣬������ֵ����Դ��ڵ�ƫ�ƣ��ͻ������꣩
	void SetPosition(int iX, int iY)
	{
		// ���Ǳ�ʾ��һ���趨���꣬������������Ϊ��ͬ����ò��������λ��
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


	// �������ָ����ʾ����������ʾ����Ҫ�����ָ������ƶ�
	void SetShowCursor(BOOL bShowCursor)
	{
		m_bShowMouseCursor = bShowCursor;
		ShowCursor(bShowCursor);
	}


	// �����������꣬��WM_MOUSEWHEEL��Ӧ����Ϣ��(short) HIWORD (wParam)��ע�����ֵ��Ƚϴ󣬹���ÿ��λ�Գɰٵ���
	void SetMidAxis(short int iDelta)
	{
		m_iLastMidAxis = m_iCurrentMidAxis;
		m_iCurrentMidAxis += iDelta;
	}


	// ���ð���״̬����һ��������ʵ���Ǵ�����Ϣ���ͣ��ڶ��������Ǹ��ӵ�ѡ����Լ���Ƿ�ͬʱ������ϵͳ������MK_����������ж�
	void SetKeyAttribute(LONG lCurrentKeyType, WPARAM wParam)
	{
		switch(lCurrentKeyType)
		{
		case WM_LBUTTONUP:
			// Win��Ϣ�ǲ�����Asyncһ������UP�ģ���������ϻص���Ϣ��Down/Hold����ô�ٴ�UP��˵���ͷŵ���ť�ˣ���δ��
			if(m_LeftButton == KA_DOWN || m_LeftButton == KA_DOWNHOLD || m_LeftButton == KA_DOUBLECLICK)
				m_LeftButton = KA_RELEASE;
			break;
		case WM_LBUTTONDOWN:
			// ���£�ǿ��ת������״̬��˲ʱ����
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
		// ���е�˲ʱ���ݣ��������
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


		// Last��Currentֻ������ʱ��ȡ�ƶ����룬����ÿ֡������ָ���������겻�ƶ�ʱ�����ò���WM_MouseMove��Ϣ���޷�SetPosition��������޷���ʱ����
		// �����ʾ���ָ�룬�Ͳ������ƶ�
		if(m_bShowMouseCursor)
		{
			m_LastPos.x = m_CurrentPos.x;
			m_LastPos.y = m_CurrentPos.y;
			ShowCursor(TRUE);
		}
		// ����ʾ���ָ�룬��ʾ��Ҫ���������ƶ�����ô�ƶ�һ�ξ�Ҫ�����ָ�����ԭλ�������ſ��������ƶ�
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
	POINT m_CurrentPos, m_LastPos;			// ������꣬ͨ�������������㵱ǰ�ƶ���
	int m_iCurrentMidAxis, m_iLastMidAxis;	// �м����λ������
	KEYATTRIBUTE m_LeftButton, m_RightButton, m_MidButton;	// ����������״̬
	BOOL m_bShowMouseCursor;				// �Ƿ���ʾ���ָ�룬�����ʾ���Ͳ������ƶ�
};

extern KMouseControl g_MouseControl;

#endif