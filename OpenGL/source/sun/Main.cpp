// Triangle.cpp
// Our first OpenGL program that will just draw a triangle on the screen.
#include<GLtools.h>
#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#include <GL/freeglut.h>            // Windows FreeGlut equivalent
#endif
#include<GL/glew.h>
#include<engine/GLContext.h>
#ifdef  _WIN32
#include<Windows.h>
#else
#include<sys/time.h>
#endif
//callback for event window size changed
static     void          __onChangeSize(int w, int h)
{
	         glViewport(0, 0, w, h);
}
///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.

//draw screen,default frame/second is 30
void         __OnDraw__()
{
	GLContext    *_context = GLContext::getInstance();
	if (_context->draw)
	{
		       _context->draw(_context);
			   glutSwapBuffers();
	}
}
void         __OnUpdate__(int   _tag)
{
		GLContext		*_context = GLContext::getInstance();
		int                     _newTickCount = 0;
#ifdef _WIN32
		_newTickCount = GetTickCount();
#else
		struct     timeval     val;
		gettimeofday(&val, NULL);
		_newTickCount = (val.tv_sec - _context->baseTickCount) * 1000 + val.tv_usec / 1000;
#endif
		if (_context->update)
		{
#ifdef _WIN32
			_context->update(_context, (_newTickCount - _context->lastTickCount) / 1000.0f);
			_context->lastTickCount = _newTickCount;
#else
			_context->update(_context,(_newTickCount-_context->lastTickCount)/1000.0f);
			_context->lastTickCount = _newTickCount;
#endif
		}
		glutPostRedisplay();
		int       _time = 0;
#ifdef  _WIN32
		_time = GetTickCount() - _newTickCount;
#else
		struct    timeval     _val;
		gettimeofday(&_val,NULL);
		_time = (_val.tv_sec - _context->baseTickCount) * 1000 + _val.tv_usec / 1000 - _newTickCount;
#endif
//如果执行回调函数的时间大于33.3毫秒
		int        _delayTime = 34 - _time;
		if (_delayTime <= 10)//修正时间,不至于让间隔变得太小
			          _delayTime = 20;
		glutTimerFunc(_delayTime, __OnUpdate__, 0);
}
///////////////////////////////////////////////////////////////////////////////
extern     void      glMain(GLContext    *);
// Main entry point for GLUT based programs
int      main(int argc, char* argv[])
{
//	gltSetWorkingDirectory(argv[0]);
	glutInit(&argc, argv);
	GLContext	 *_context = GLContext::getInstance();
	glMain(_context);
	glutInitDisplayMode(_context->displayMode);
	glutInitWindowSize((int)_context->winSize.width,(int)_context->winSize.height);
	glutInitWindowPosition((int)_context->winPosition.x,(int)_context->winPosition.y);
	glutCreateWindow(_context->winTitle);
	glutReshapeFunc(__onChangeSize);
	glutDisplayFunc(__OnDraw__);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
	}
//init函数必须实现,必须调用初始化函数
	_context->init(_context);
	glutTimerFunc(34, __OnUpdate__, 0);
	glutMainLoop();
	if (_context->finalize)//程序退出时的清理
		       _context->finalize(_context);
	if (_context->userObject)//释放申请的内存
		      free(_context->userObject);
	return 0;
}
