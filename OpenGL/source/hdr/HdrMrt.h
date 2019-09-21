/*
 *hdr + mrt :����������
 *2016-9-22 10:33:47
 *@Author:С����
  */
#ifndef  __HDR_MRT_H__
#define __HDR_MRT_H__
#include<engine/GLObject.h>

class    HdrMrt :public  GLObject
{
private:
//֡����������
	unsigned         _hdr_mrt_framebufferId;
	unsigned         _origin_framebufferId;
//location=0��������
	unsigned         _normal_textureId;
//location=1��������
	unsigned         _hdr_textureId;
//���������
	unsigned         _depth_textureId;
	int                   _framebuffer_width;
	int                   _framebuffer_height;
private:
	HdrMrt();
	HdrMrt(HdrMrt &);
	void                 initHdrMrt(int    width,int   height);
public:
	~HdrMrt();
	static         HdrMrt			*createHdrMrt(int  width,int  height);
//��ȡlocation=0���������
	unsigned        normalTexture();
//��ȡlocation=1���������
	unsigned        hdrTexture();
//��֡����������
	void                activateFramebuffer();
//�ָ�ԭ����֡�����������
	void                resumeFramebuffer();
};
#endif
