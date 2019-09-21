/*
  *@aim:程序对象实现,最重要的OpenGL对象封装
  &2016-3-7 16:47:14
  */
//Version 3.0:增加对几何着色器的支持,2016-7-19 10:13:26
#include<engine/Tools.h>
#include<engine/GLProgram.h>
#include<engine/GLState.h>

#ifdef  __ENABLE_PROGRAM_CACHE__
#include<engine/GLCacheManager.h>
#endif

#include<assert.h>
#include<stdio.h>
#include<map>
#include<GL/glew.h>
__NS_GLK_BEGIN
//编译着色器
static    GLuint   __compile_shader(unsigned    _type, const   char    *_shader_source);
static   const     char    *__now_compile_file_name;
GLProgram::GLProgram()
{
	_object = 0;
	_vertex = 0;
	_frame = 0;
#ifdef  __GEOMETRY_SHADER__
	_geometry = 0;
#endif
}
//析构
GLProgram::~GLProgram()
{
	glDetachShader(_object, _vertex);
	glDetachShader(_object, _frame);
#ifdef __GEOMETRY_SHADER__
	glDetachShader(_object, _geometry);
	glDeleteShader(_geometry);
#endif
	glDeleteShader(_vertex);
	glDeleteShader(_frame);
	glDeleteProgram(_object);
	_object = 0;
}
GLProgram               *GLProgram::createWithFile(const   char    *vertex_file_name, const   char *frame_file_name)
{	
	GLProgram           *_glProgram;
#ifdef __ENABLE_PROGRAM_CACHE__
	std::string     key(vertex_file_name);
	key.append(frame_file_name);
	_glProgram= GLCacheManager::getInstance()->findGLProgram(key);
	if (_glProgram)
	{
		_glProgram->retain();
		return _glProgram;
	}
#endif
	_glProgram = new           GLProgram();
	__now_compile_file_name = frame_file_name;
	_glProgram->initWithFile(vertex_file_name, frame_file_name);
#ifdef __ENABLE_PROGRAM_CACHE__
	//_glProgram->retain();
	GLCacheManager::getInstance()->inserGLProgram(key, _glProgram);
	__now_compile_file_name = NULL;
#endif
	return   _glProgram;
}
//
GLProgram              *GLProgram::createWithString(const  char  *vertex_string, const  char *frame_string)
{
	GLProgram           *_glProgram = new           GLProgram();
	_glProgram->initWithString(vertex_string, frame_string);
	return      _glProgram;
}
//使用着色器对象
void      GLProgram::enableObject()
{
	glUseProgram(_object);
}

void      GLProgram::perform()const
{
	glUseProgram(_object);
}

void      GLProgram::disable()const
{
	glUseProgram(0);
}
//disable
void     GLProgram::disableObject()const
{
	glUseProgram(0);
}
//获取程序对象,
//GLuint      GLProgram::getObject()
//{
//	return     _object;
//}
//
GLuint      GLProgram::getUniformLocation(const   char  *_name)const
{
	return     glGetUniformLocation(_object, _name);
}
GLuint       GLProgram::getAttribLocation(const char *_name)const
{
	return   glGetAttribLocation(_object, _name);
}
//使用文件初始化
bool      GLProgram::initWithFile(const  char  *_vertex_file, const   char  *_frame_file)
{
	const    char   *_vertex_buff = Tools::getFileContent(_vertex_file);
	const    char   *_frame_buff = Tools::getFileContent(_frame_file);
	if (!_vertex_file )
		printf("file %s don't exist !\n",_vertex_file);
	if (!_frame_file)
		printf("file %s don't exist!\n",_frame_file);

	if (!_vertex_buff || !_frame_buff)
	{
		printf("file '%s' or file '%s' do not be found!\n",_vertex_file,_frame_file);
		assert(0);
	}
	__now_compile_file_name = _vertex_file;
	bool    _result = this->initWithString(_vertex_buff, _frame_buff);
	delete    _vertex_buff;
	delete    _frame_buff;
	return    _result;
}
#ifdef  __GEOMETRY_SHADER__
GLProgram      *GLProgram::createWithFile(const char *vertex_file, const char *geometry_file, const char *frame_file)
{
	GLProgram           *_glProgram;
#ifdef __ENABLE_PROGRAM_CACHE__
	std::string     key(vertex_file);
	key.append(geometry_file).append(frame_file);
	_glProgram= GLCacheManager::getInstance()->findGLProgram(key);
	if (_glProgram)
		return _glProgram;
#endif
	_glProgram = new           GLProgram();
	_glProgram->initWithFile(vertex_file, geometry_file, frame_file);
#ifdef __ENABLE_PROGRAM_CACHE__
	GLCacheManager::getInstance()->inserGLProgram(key, _glProgram);
#endif
	return   _glProgram;
}

GLProgram       *GLProgram::createWithString(const char *vertex_string, const char *geometry_string, const char *frame_string)
{
	GLProgram           *_glProgram = new           GLProgram();
	_glProgram->initWithString(vertex_string, geometry_string,frame_string);
	return      _glProgram;
}

bool     GLProgram::initWithFile(const char *vertex, const char *geometry, const char *frame)
{
	const    char   *_vertex_buff = Tools::getFileContent(vertex);
	const    char   *_geometry_buffer = Tools::getFileContent(geometry);
	const    char   *_frame_buff = Tools::getFileContent(frame);
//顶点着色器
	if (!_vertex_buff)
		printf("file %s don't exist !\n", vertex);
//几何着色器
	if (!_geometry_buffer)
		printf("file %s don't exist !\n",geometry);
	if (!_frame_buff)
		printf("file %s don't exist!\n", frame);

	assert(_vertex_buff && _geometry_buffer && _frame_buff);
	__now_compile_file_name = vertex;
	bool    _result = this->initWithString(_vertex_buff,_geometry_buffer ,_frame_buff);
	delete    _vertex_buff;
	delete    _geometry_buffer;
	delete    _frame_buff;
	return    _result;
}

bool     GLProgram::initWithString(const char *vertex_string, const char *geometry_string, const char *frame_string)
{
	_vertex = __compile_shader(GL_VERTEX_SHADER, vertex_string);
	_geometry = __compile_shader(GL_GEOMETRY_SHADER, geometry_string);
	_frame = __compile_shader(GL_FRAGMENT_SHADER, frame_string);
	//首先必须编译成功
	assert(_vertex && _geometry && _frame);
	//创建程序对象
	_object = glCreateProgram();
	glAttachShader(_object, _vertex);
	glAttachShader(_object,_geometry);
	glAttachShader(_object, _frame);
	//链接
	GLint     _result;
	glLinkProgram(_object);
	glGetProgramiv(_object, GL_LINK_STATUS, &_result);
	if (!_result)//如果没有连接成功
	{
		GLint      _size = 0;
		glGetProgramiv(_object, GL_INFO_LOG_LENGTH, &_size);
		if (_size > 0)
		{
			char    *_buff = new   char[_size + 2];
			glGetProgramInfoLog(_object, _size + 1, NULL, _buff);
			_buff[_size] = '\0';
			printf("%s\n", _buff);
			delete    _buff;
		}
		glDeleteProgram(_object);
		_object = 0;
		assert(false);
	}
	return   true;
}
#endif
//使用字符串
bool      GLProgram::initWithString(const  char  *_vertex_string, const  char  *_frame_string)
{
	_vertex = __compile_shader(GL_VERTEX_SHADER, _vertex_string);
	_frame = __compile_shader(GL_FRAGMENT_SHADER, _frame_string);
	//首先必须编译成功
	assert(_vertex && _frame);
	//创建程序对象
	_object = glCreateProgram();
	glAttachShader(_object, _vertex);
	glAttachShader(_object, _frame);
	//链接
	GLint     _result;
	glLinkProgram(_object);
	glGetProgramiv(_object, GL_LINK_STATUS, &_result);
	if (!_result)//如果没有连接成功
	{
		GLint      _size = 0;
		glGetProgramiv(_object, GL_INFO_LOG_LENGTH, &_size);
		if (_size > 0)
		{
			char    *_buff = new   char[_size + 2];
			glGetProgramInfoLog(_object, _size + 1, NULL, _buff);
			_buff[_size] = '\0';
			printf("%s\n", _buff);
			delete    _buff;
		}
		glDeleteProgram(_object);
		_object = 0;
		if (__now_compile_file_name)
		{
			printf("file '%s'  has some syntax error", __now_compile_file_name);
		}
		assert(false);
	}
	return   true;
}
void          GLProgram::feedbackVaryingsWith(const char *_varyings[], int _count, int _attr_type)
{
	assert(_attr_type == GL_INTERLEAVED_ATTRIBS || _attr_type == GL_SEPARATE_ATTRIBS);
	glTransformFeedbackVaryings(_object, _count, _varyings, _attr_type);
	glLinkProgram(_object);
	int       result = 0;
	glGetProgramiv(_object, GL_LINK_STATUS, &result);
	if (!result)
	{
		int          _size = 0;
		glGetProgramiv(_object, GL_INFO_LOG_LENGTH, &_size);
		if (_size > 0)
		{
			char     *buffer = new   char[_size + 2];
			glGetProgramInfoLog(_object, _size + 1, NULL, buffer);
			buffer[_size] = '\0';
			printf("%s\n", buffer);
			delete    buffer;
		}
		glDeleteProgram(_object);
		_object = 0;
		assert(0);
	}
}
//着色器编译
GLuint      __compile_shader(GLenum _type, const char *_shader_source)
{
//限定只能是顶点着色器或者片段着色器,如果是OpenGL平台,也可以是几何着色器,计算着色器
#ifdef     __OPENGL_VERSION__
	assert(_type == GL_VERTEX_SHADER || _type == GL_FRAGMENT_SHADER || _type == GL_GEOMETRY_SHADER || _type==GL_COMPUTE_SHADER);
#else
	assert(_type == GL_VERTEX_SHADER || _type == GL_FRAGMENT_SHADER);
#endif
	GLuint     _shader = glCreateShader(_type);
	assert(_shader);
	//附着代码
	glShaderSource(_shader, 1, &_shader_source, NULL);
	glCompileShader(_shader);
	GLint    _result = 0;
	glGetShaderiv(_shader, GL_COMPILE_STATUS, &_result);
	if (!_result)
	{
		GLint      _size = 0;
		char        *_buff = NULL;
		glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &_size);
		if (_size > 0)
		{
			_buff = new   char[_size + 2];
			glGetShaderInfoLog(_shader, _size + 1, NULL, _buff);
			_buff[_size] = '\0';
			printf("%s\n", _buff);
			delete     _buff;
		}
		glDeleteShader(_shader);
		_shader = 0;
		if (__now_compile_file_name)
			printf("file '%s' has some syntax error\n", __now_compile_file_name);
		assert(false);
	}
	return    _shader;
}
///////////////////////////////计算着色器//////////////////////////
#ifdef  __OPENGL_VERSION__
ComputeShader::ComputeShader()
{
	_object = 0;
	_computeShader = 0;
}
ComputeShader::~ComputeShader()
{
	glDetachShader(_object, _computeShader);
	glDeleteShader(_computeShader);
	glDeleteProgram(_object);
	_object = 0;
	_computeShader = 0;
}
void      ComputeShader::initWithString(const char *shader_string)
{
	_computeShader = __compile_shader(GL_COMPUTE_SHADER, shader_string);
	_object = glCreateProgram();
	glAttachShader(_object, _computeShader);
	glLinkProgram(_object);

	int      _result;
	glGetProgramiv(_object, GL_LINK_STATUS, &_result);
	if (!_result)//如果没有连接成功
	{
		GLint      _size = 0;
		glGetProgramiv(_object, GL_INFO_LOG_LENGTH, &_size);
		if (_size > 0)
		{
			char    *_buff = new   char[_size + 2];
			glGetProgramInfoLog(_object, _size + 1, NULL, _buff);
			_buff[_size] = '\0';
			printf("%s\n", _buff);
			delete    _buff;
		}
		glDeleteProgram(_object);
		_object = 0;
		assert(false);
	}
}

void      ComputeShader::initWithFile(const char *file_name)
{
	const  char  *file_content = Tools::getFileContent(file_name);
	if (!file_content)
	{
		printf("file  %s is  not  exist !\n",file_name);
		assert(0);
	}
	printf("Compile Compute Shader file '%s'\n",file_name);
	this->initWithString(file_content);
	delete   file_content;
}

ComputeShader	*ComputeShader::createWithFile(const char *file_name)
{
	ComputeShader		*_glProgram = new   ComputeShader();
	_glProgram->initWithFile(file_name);
	return  _glProgram;
}

ComputeShader	*ComputeShader::createWithString(const char *shader_string)
{
	ComputeShader	*_glProgram = new   ComputeShader();
	_glProgram->initWithString(shader_string);
	return  _glProgram;
}

void       ComputeShader::dispatch(int dispatch_x_size, int dispatch_y_size, int dispatch_z_size)const
{
	//glUseProgram(_object);
	glDispatchCompute(dispatch_x_size, dispatch_y_size, dispatch_z_size);
}

void       ComputeShader::perform()const
{
	glUseProgram(_object);
}

int         ComputeShader::getUniformLocation(const char *name)const
{
	return  glGetUniformLocation(_object, name);
}

unsigned ComputeShader::getObject()const
{
	return _object;
}
#endif

__NS_GLK_END