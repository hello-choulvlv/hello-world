/*
 *hdr双纹理融合
 *2016-9-22 15:39:53
 *@Author:小花熊
 */
#include<GL/glew.h>
#include<stdio.h>
#include "HdrMerge.h"

HdrMerge::HdrMerge()
{
	_glProgram = NULL;
	_vertex_bufferId = 0;
}
HdrMerge::~HdrMerge()
{
	_glProgram->release();
	_glProgram = NULL;
	glDeleteBuffers(1, &_vertex_bufferId);
	_vertex_bufferId = 0;
}

void             HdrMerge::initHdrMerge(float   exposure)
{
	_exposure = exposure;
	_glProgram = GLProgram::createWithFile("shader/hdr/hdr_merge.vsh", "shader/hdr/hdr_merge.fsh");
	_glProgram->retain();
	_mvpMatrixLoc = _glProgram->getUniformLocation("u_mvpMatrix");
	_baseMapLoc = _glProgram->getUniformLocation("u_baseMap");
	_hdrMapLoc = _glProgram->getUniformLocation("u_hdrMap");
	_exposureLoc = _glProgram->getUniformLocation("u_exposure");
//
	float           vVertex3f2f[20] = {
		-1.0f,1.0f,0.0f,   0.0f,1.0f,
		-1.0f,-1.0f,0.0f,  0.0f,0.0f,
		1.0f,1.0f,0.0f,    1.0f,1.0f,
		1.0f,-1.0f,0.0f,  1.0f,0.0f,
	};
	glGenBuffers(1, &_vertex_bufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _vertex_bufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vVertex3f2f), vVertex3f2f, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	_mvpMatrix.identity();
}

HdrMerge	  *HdrMerge::createHdrMerge(float exposure)
{
	HdrMerge	*_hdr = new   HdrMerge();
	_hdr->initHdrMerge(exposure);
	return  _hdr;
}
void       HdrMerge::setExposure(float exposure)
{
	_exposure = exposure;
}

void       HdrMerge::drawMerge(unsigned baseMap, unsigned hdrMap)
{
	_glProgram->enableObject();

	glBindBuffer(GL_ARRAY_BUFFER, _vertex_bufferId);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void *)(sizeof(float)*3));
//矩阵传导
	glUniformMatrix4fv(_mvpMatrixLoc, 1, GL_FALSE, _mvpMatrix.pointer());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, baseMap);
	glUniform1i(_baseMapLoc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, hdrMap);
	glUniform1i(_hdrMapLoc, 1);
//高光指数
	glUniform1f(_exposureLoc, _exposure);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}