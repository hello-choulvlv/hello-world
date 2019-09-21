/*
 *延迟着色前阶段着色
*/
#include<engine/RenderObject.h>
#include<engine/GLProgram.h>
#include<engine/GLTexture.h>
#include<engine/Shape.h>
class        RenderSphere:public  GLObject//public   RenderObject
{
private:
	GLProgram             *_glProgram;
	GLSphere			     *_sphere;
	GLTexture               *_texture;
	Matrix                     _mvpMatrix;
	Matrix                     _modelMatrix;
	Matrix3                     _normalMatrix;
//
	unsigned                 _mvpMatrixLoc;
	unsigned                 _modelMatrixLoc;
	unsigned                 _normalMatrixLoc;
	unsigned                 _baseMapLoc;
	float                        _deltaTime;
	GLVector3             _position;
private:
	
public:
	void                       initWithFile(const   char   *file_name);
	RenderSphere();
	~RenderSphere();
	static    RenderSphere         *create(const   char    *file_name);
	void                         draw(Matrix          &projMatrix,unsigned flag);
	void                         update(float           _deltaTime);
	void                         setPosition(GLVector3  &);
};