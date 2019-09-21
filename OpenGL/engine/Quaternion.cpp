/*
  *四元数实现,目前已经集成到了Geometryh/Geometry.cpp文件中
  *2016-5-23 19:18
  */
#include"engine/Quaternion.h"
#include<math.h>
#include<assert.h>
__NS_GLK_BEGIN

 Quaternion::Quaternion()
{
	w = 1.0f;
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

Quaternion::Quaternion(const float w,const float x, const float y, const float z)
{
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z;
}

Quaternion::Quaternion(const float angle, const GLVector3 &vec)
{
//半角
	float        _halfAngle = angle*MATH_PI /360.0f;
	float        _sinVector = sinf(_halfAngle);
//单位化旋转向量
	float        _vector_length = sqrtf(vec.x*vec.x+vec.y*vec.y+vec.z*vec.z);
	assert(_vector_length>=0.0001f);

	w = cosf(_halfAngle);
	x = vec.x*_sinVector / _vector_length;
	y = vec.y*_sinVector / _vector_length;
	z = vec.z*_sinVector / _vector_length;
}

Quaternion::Quaternion(const Matrix   &rotate)
{
	float        _lamda = rotate.m[0][0] + rotate.m[1][1] + rotate.m[2][2] + 1.0f;
	assert(_lamda>0.0001f &&  _lamda<=4.0f);
	w = 0.5f*sqrtf(_lamda );

	float         w4 = 4.0f*w;
	x = (rotate.m[1][2]-rotate.m[2][1])/w4;
	y = (rotate.m[2][0]-rotate.m[0][2])/w4;
	z = (rotate.m[0][1]-rotate.m[1][0])/w4;
//进一步验证生成的数据的合法性,所得到的向量必须是单位向量
	assert(fabs(x*x+y*y+z*z-1.0f)<=0.0001f);
}

void      Quaternion::multiply(Quaternion &p)
{
	float    aw = w*p.w - x*p.x - y*p.y - z*p.z;
	float    ax = w*p.x + x*p.w+y*p.z - z*p.y;
	float    ay = w*p.y-x*p.z+y*p.w+z*p.x;
	float    az = w*p.z+x*p.y-y*p.x+z*p.w;
	w = aw, x = ax, y = ay, z = az;
}

Quaternion		Quaternion::operator*(const Quaternion   &p)const
{
	float    aw = w*p.w - x*p.x - y*p.y - z*p.z;
	float    ax = w*p.x + x*p.w + y*p.z - z*p.y;
	float    ay = w*p.y - x*p.z + y*p.w + z*p.x;
	float    az = w*p.z + x*p.y - y*p.x + z*p.w;

	return    Quaternion(aw,ax,ay,az);
}
void      Quaternion::identity()
{
	w = 1.0f;
	x = y = z = 0.0f;
}

void       Quaternion::normalize()
{
	float         _length = sqrtf(w*w+x*x+y*y+z*z);
	assert(_length>0.0001f);

	w /= _length;
	x /= _length;
	y /= _length;
	z /= _length;
}

void         Quaternion::toRotateMatrix(Matrix &rotateMatrix)const
{
	float          xy = this->x * this->y;
	float          xz = this->x * this->z;
	float          yz = this->y * this->z;
	float         ww = this->w * this->w;

	rotateMatrix.m[0][0] = 1.0f - 2 * (x*x + ww);
	rotateMatrix.m[0][1] = 2.0f * (xy + w*z);
	rotateMatrix.m[0][2] = 2.0f*(xz - w*y);
	rotateMatrix.m[0][3] = 0.0f;

	rotateMatrix.m[1][0] = 2.0f*(xy - w*z);
	rotateMatrix.m[1][1] = 1.0f - 2.0f*(y*y + ww);
	rotateMatrix.m[1][2] = 2.0f*(yz * w*x);
	rotateMatrix.m[1][3] = 0.0f;

	rotateMatrix.m[2][0] = 2.0f*(xz + w*y);
	rotateMatrix.m[2][1] = 2.0f*(yz - w*x);
	rotateMatrix.m[2][2] = 1.0f - 2.0f*(z*z + ww);
	rotateMatrix.m[2][3] = 0.0f;

	rotateMatrix.m[3][0] = 0.0f;
	rotateMatrix.m[3][1] = 0.0f;
	rotateMatrix.m[3][2] = 0.0f;
	rotateMatrix.m[3][3] = 0.0f;
}

Matrix	    Quaternion::toRotateMatrix()
{
	Matrix      _rotate;
	float          xy = this->x * this->y;
	float          xz = this->x * this->z;
	float          yz = this->y * this->z;
	float         ww = this->w * this->w;

	_rotate.m[0][0] = -1.0f + 2.0f * (x*x + ww) ;//==>1.0f - 2.0f *(y*y+z*z)
	_rotate.m[0][1] =2.0f * (xy+ w*z) ;
	_rotate.m[0][2] = 2.0f*(xz - w*y);

	_rotate.m[1][0] = 2.0f*(xy - w*z);
	_rotate.m[1][1] = -1.0f + 2.0f*(y*y+ww);//==>1.0f - 2.0f*(x*x+z*z)
	_rotate.m[1][2] = 2.0f*(yz * w*x);

	_rotate.m[2][0] = 2.0f*(xz + w*y);
	_rotate.m[2][1] = 2.0f*(yz - w*x);
	_rotate.m[2][2] = -1.0f + 2.0f*(z*z+ww) ;//==>1.0f - 2.0f*(x*x+y*y)

	return _rotate;
}

Quaternion       Quaternion::conjugate()
{
	return   Quaternion(w,-x,-y,-z);
}

Quaternion      Quaternion::reverse()const
{
	//float    _length = sqrtf(w*w+x*x+y*y+z*z);
	//assert(_length>=__EPS__);
	return    Quaternion( w,-x,-y,-z   );
}

float   Quaternion::dot(const Quaternion &other)const
{
	return w*other.w + x*other.x + y*other.y + z*other.z;
}
//旋转3维向量
GLVector3 Quaternion::rotate(const GLVector3 &src)const
{
	const GLVector3 sinVec(x,y,z);
	const GLVector3 uv = sinVec.cross(src);
	const GLVector3 uuv = sinVec.cross(uv);

	return src+ uv *(2.0f *w) + uuv * 2.0f;
}

GLVector3 Quaternion::operator*(const GLVector3 &src)const
{
	const GLVector3 sinVec(x, y, z);
	const GLVector3 uv = sinVec.cross(src);
	const GLVector3 uuv = sinVec.cross(uv);

	return src + uv *(2.0f *w) + uuv * 2.0f;
}

//两个插值函数,以后再实现

Quaternion Quaternion::lerp(const Quaternion &p, const Quaternion &q, const float lamda)
{	
	assert(lamda>=0.0f && lamda<=1.0f);
	const float one_minus_t = 1.0f - lamda;
	const float w = one_minus_t * p.w + lamda * q.w;
	const float  x = one_minus_t * p.x + lamda *q.x;
	const float  y = one_minus_t * p.y + lamda * q.y;
	const float  z = one_minus_t * p.z + lamda * q.z;
	const float length = sqrtf(w*w+x*x+y*y+z*z);
	return Quaternion(w/ length,x/ length,y,z/ length);
}

Quaternion  Quaternion::slerp(const Quaternion &p,const Quaternion &q, const float lamda)
{
	assert(lamda>=0.0f && lamda<=1.0f);
	//检测两个四元数之间的夹角
	const float angleOfIntersect = p.dot(q);
	assert(angleOfIntersect>=0.0f);
	//如果夹角接近于0,则使用线性插值
	if (angleOfIntersect >= 1.0f - 0.01f)
		return lerp(p, q, lamda);
	//取连个四元数之间的最短路径
	const float sinValue = sqrtf(1.0f - angleOfIntersect*angleOfIntersect);
	const float angle = asinf(sinValue);

	const float sin_one_minus_t = sinf((1.0f-lamda)*angle);
	const float sin_t = sinf(lamda * angle);
	const float a = sin_one_minus_t / sinValue;
	const float b = sin_t / sinValue;

	const float w = a* p.w + b * q.w;
	const float x = a *p.x + b * q.x;
	const float y = a * p.y + b*q.y;
	const float z = a *p.z + b*q.z;
	const float length =sqrtf( w* w + x*x + y*y + z*z);
	return Quaternion(w/length,x/length,y/length,z/length);
}
__NS_GLK_END