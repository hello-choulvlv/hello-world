/*
  *�ɼ�����ɫ��ʵ�ֵ�ˮ��Ⱦ
  *2017-7-25
  *@Author:xiaohuaxiong
 */
#include "engine/Object.h"
#include "engine/Geometry.h"
#include "engine/GLProgram.h"
#include "engine/Camera.h"
#include "engine/Shape.h"
#include "engine/GLTexture.h"

#include "engine/event/TouchEventListener.h"

#include "WaterComputeShader.h"
#include "WaterNormalShader.h"
#include "RenderShader.h"

class ComputeWater :public glk::Object
{
	//����
	glk::Mesh			*_waterMesh;
	//����Ĵ�С
	const int               _meshSize;
	//ÿ����Ԫ�Ĵ�С
	const  int              _meshUnitSize;
	//��������ͼ
	glk::GLCubeMap  *_texCubeMap;
	//Shader
	WaterComputeShader    *_computeWaterShader;
	WaterNormalShader      *_computeNormalShader;
	RenderShader                 *_renderShader;
	//ˮ��Ⱦ����
	glk::GLVector4                 _waterParam;
	glk::Matrix                       _mMatrix;//ģ�;���
	glk::GLVector4                _waterColor;
	//������ϵ��
	glk::GLVector3                _freshnelParam;
	/*
	  *shader buffer
	 */
	unsigned              _heightFieldBuffer;
	unsigned              _velocityFieldBuffer;
	unsigned              _outHeightFieldBuffer;
	unsigned              _normalFieldBuffer;
	/*
	  *�����
	 */
	glk::Camera        *_camera;
	//���봥���¼�
	glk::TouchEventListener   *_touchEventListener;
	glk::GLVector2                    _offsetVec;//������ƫ����
	/*
	  *�����¼�
	 */
	glk::KeyEventListener       *_keyEventListener;
	int                                         _keyMask;//���̰�������
	//��ǰʱ���ۻ�
	float                     _deltaTime;
private:
	ComputeWater();
	ComputeWater(const ComputeWater &);
	void      init();
public:
	~ComputeWater();
	static ComputeWater  *create();
	/*
	  *����Shader Buffer����
	 */
	void    initShaderBuffer();
	/*
	  *��ʼ������ˮ��Ⱦ��uniform����
	 */
	void   initWaterParam();
	/*
	  *����ˮ��Ⱦ�����е�����߶ȳ�
	 */
	void   updateWaterComputeShader();
	/*
	  *���㷨��
	 */
	void   updateWaterNormal();
	/*
	  *����ˮ������
	*/
	void   drawWater();
	/*
	  *draw
	 */
	void  draw();
	/*
	  *update
	 */
	void  update(float deltaTime);
	/*
	  *�����¼�
	 */
	bool  onTouchBegan(const glk::GLVector2 *touchPoint);
	void  onTouchMoved(const glk::GLVector2 *touchPoint);
	void  onTouchEnded(const glk::GLVector2 *touchPoint);
	/*
	  *����Ļ�ϵ�ĳһ�����ˮ����
	*/
	void  onTouchWaterWave(const glk::GLVector2 *touchPoint);
	/*
	  *�����¼�
	*/
	bool onKeyPressed(const glk::KeyCodeType keyCode);
	void onKeyReleased(const glk::KeyCodeType keyCode);
};