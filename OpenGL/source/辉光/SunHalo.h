/*
  *����ʵ��
  *2017-07-14
  *@Author:xiaohuaxiong
 */
#ifndef __SUN_HALO_H__
#define __SUN_HALO_H__
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/GLTexture.h"
#include "engine/RenderTexture.h"
#include "engine/Shape.h"
#include "engine/event/TouchEventListener.h"

#include "BlurShader.h"
#include "HaloShader.h"
#include "RenderShader.h"
#include "NormalShader.h"
/*
  *ʵʱ�Թ�
 */
class SunHalo :public glk::Object
{
	//����֡����������,�����ʹ����Ⱦ-->ģ������
	glk::RenderTexture  *_drawSunTexture;// ��̫��
	// �����ȵ�ģ��
	glk::RenderTexture  *_drawThickTexture0;
	glk::RenderTexture  *_drawThickTexture1;
	//ϸ���ȵ�ģ��
	glk::RenderTexture  *_drawPreciseTexture0;
	glk::RenderTexture  *_drawPreciseTexture1;
	//���
	glk::RenderTexture   *_drawMixTexture;
	//����
	glk::GLTexture          *_dirtyTexture;
	//��
	glk::Sphere                 *_sphereMesh;
	//��ͼ����/ͶӰ����
	glk::Matrix                  _viewMatrix;
	glk::Matrix                  _projMatrix;
	glk::Matrix                  _viewProjMatrix;
	//Shader
	BlurShader                  *_blurShader;
	HaloShader                 *_haloShader;
	RenderShader             *_renderShader;
	NormalShader            *_normalShader;
	//event
	glk::TouchEventListener   *_touchEventListener;
	/*
	  *�ֽ���ͼ�����ʱ��ʹ�õ�����
	 */
	glk::GLVector3           _rotateVec;
	glk::GLVector3           _translateVec;
	//ƽ�ƹ�������
	glk::GLVector2           _offsetVec;
private:
	SunHalo();
	void				init();
	//��ʼ����ͼ����/ͶӰ����
	void            initCamera(const glk::GLVector3 &eyePosition,const glk::GLVector3 &targetPosition);
public:
	~SunHalo();
	static		   SunHalo *create();
	void            update(float dt);
	void            draw();
	/*
	  *��̫��
	 */
	void           drawSunTexture();
	/*
	  *������ģ��֡����������
	 */
	void           drawThickTexture();
	/*
	  *ϸ����ģ��֡����������
	 */
	void          drawPreciseTexture();
	/*
	  *���
	 */
	void         drawMixTexture();
	/*
	  *ֱͨ
	 */
	void        drawNormalTexture();
	/*
	  *���Ժ���,����Ⱦ�����������
	*/
	void        drawTexture(int textureId);
	/*
	  *�����ص�����
	 */
	bool       onTouchBegan(const glk::GLVector2 *touchPoint);
	void       onTouchMoved(const glk::GLVector2 *touchPoint);
	void       onTouchEnded(const glk::GLVector2 *touchPoint);
};
#endif