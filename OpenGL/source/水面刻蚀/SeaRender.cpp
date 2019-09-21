/*
  *光线的刻蚀实现
  *2016-11-2 19:25:40
  @Author:小花熊
  */
#include "SeaRender.h"

GroundRender::GroundRender()
{
	_groundProgram = GLProgram::createWithFile("shader/water/sea_ground.vsh", "shader/water/sea_ground.fsh");
	_mvpMatrixLoc = _groundProgram->getUniformLocation("u_mvpMatrix");
	_modelMatrixLoc = _groundProgram->getUniformLocation("u_modelMatrix");
	_normalMatrixLoc = _groundProgram->getUniformLocation("u_normalMatrix");
	_baseMapLoc = _groundProgram->getUniformLocation("u_baseMap");
	_lightMapLoc = _groundProgram->getUniformLocation("u_lightMap");
	_timeLoc = _groundProgram->getUniformLocation("u_time");
	_planeEquationLoc = _groundProgram->getUniformLocation("u_planeEquation");
}

GroundRender::~GroundRender()
{
	_groundProgram->release();
}