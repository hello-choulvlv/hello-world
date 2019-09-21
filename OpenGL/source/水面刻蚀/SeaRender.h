/*
  *���ߺ�ƽ��Ŀ�ʴ
  *2016-11-2 17:00:07
  *@Author:С����
  */
#ifndef  __SEA_RENDER_H__
#define __SEA_RENDER_H__
#include<engine/GLProgram.h>
#include<engine/Geometry.h>
/*
  *����
 */
class       GroundRender:public GLObject
{
public:
	GLProgram        *_groundProgram;
	unsigned            _mvpMatrixLoc;
	unsigned            _modelMatrixLoc;
	unsigned            _normalMatrixLoc;
	unsigned            _baseMapLoc;
	unsigned            _lightMapLoc;
	unsigned            _timeLoc;
	unsigned            _planeEquationLoc;
	GroundRender();
	~GroundRender();
};

class    SeaRender:public GLObject
{
public:
	GLProgram          *_seaProgram;
	unsigned                _mvpMatrixLoc;
	unsigned                _modelMatrix;
	unsigned                _normalMatrix;
	unsigned                _baseMapLoc;
};
#endif
