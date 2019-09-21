/*
  *@aim:tga image
  */
#include<engine/TGAImage.h>
#include<engine/Tools.h>
#include<engine/GLContext.h>
#include<GL/glew.h>
#include<assert.h>
#include<math.h>
#include<stdlib.h>
__NS_GLK_BEGIN

#define NOISE_TABLE_MASK   255
//#define  M_PI                  3.14159265358
#define FLOOR(x)           ((int)(x) - ((x) < 0 && (x) != (int)(x)))
#define smoothstep(t)      ( t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f ) )
#define lerp(t, a, b)      ( a + t * (b - a) )
#ifdef _WIN32
#define srandom srand
#define random rand
#endif

// lattice gradients 3D noise
static float   gradientTable[256 * 3];

// permTable describes a random permutatin of 8-bit values from 0 to 255.
static unsigned char permTable[256] =
{
	0xE1, 0x9B, 0xD2, 0x6C, 0xAF, 0xC7, 0xDD, 0x90, 0xCB, 0x74, 0x46, 0xD5, 0x45, 0x9E, 0x21, 0xFC,
	0x05, 0x52, 0xAD, 0x85, 0xDE, 0x8B, 0xAE, 0x1B, 0x09, 0x47, 0x5A, 0xF6, 0x4B, 0x82, 0x5B, 0xBF,
	0xA9, 0x8A, 0x02, 0x97, 0xC2, 0xEB, 0x51, 0x07, 0x19, 0x71, 0xE4, 0x9F, 0xCD, 0xFD, 0x86, 0x8E,
	0xF8, 0x41, 0xE0, 0xD9, 0x16, 0x79, 0xE5, 0x3F, 0x59, 0x67, 0x60, 0x68, 0x9C, 0x11, 0xC9, 0x81,
	0x24, 0x08, 0xA5, 0x6E, 0xED, 0x75, 0xE7, 0x38, 0x84, 0xD3, 0x98, 0x14, 0xB5, 0x6F, 0xEF, 0xDA,
	0xAA, 0xA3, 0x33, 0xAC, 0x9D, 0x2F, 0x50, 0xD4, 0xB0, 0xFA, 0x57, 0x31, 0x63, 0xF2, 0x88, 0xBD,
	0xA2, 0x73, 0x2C, 0x2B, 0x7C, 0x5E, 0x96, 0x10, 0x8D, 0xF7, 0x20, 0x0A, 0xC6, 0xDF, 0xFF, 0x48,
	0x35, 0x83, 0x54, 0x39, 0xDC, 0xC5, 0x3A, 0x32, 0xD0, 0x0B, 0xF1, 0x1C, 0x03, 0xC0, 0x3E, 0xCA,
	0x12, 0xD7, 0x99, 0x18, 0x4C, 0x29, 0x0F, 0xB3, 0x27, 0x2E, 0x37, 0x06, 0x80, 0xA7, 0x17, 0xBC,
	0x6A, 0x22, 0xBB, 0x8C, 0xA4, 0x49, 0x70, 0xB6, 0xF4, 0xC3, 0xE3, 0x0D, 0x23, 0x4D, 0xC4, 0xB9,
	0x1A, 0xC8, 0xE2, 0x77, 0x1F, 0x7B, 0xA8, 0x7D, 0xF9, 0x44, 0xB7, 0xE6, 0xB1, 0x87, 0xA0, 0xB4,
	0x0C, 0x01, 0xF3, 0x94, 0x66, 0xA6, 0x26, 0xEE, 0xFB, 0x25, 0xF0, 0x7E, 0x40, 0x4A, 0xA1, 0x28,
	0xB8, 0x95, 0xAB, 0xB2, 0x65, 0x42, 0x1D, 0x3B, 0x92, 0x3D, 0xFE, 0x6B, 0x2A, 0x56, 0x9A, 0x04,
	0xEC, 0xE8, 0x78, 0x15, 0xE9, 0xD1, 0x2D, 0x62, 0xC1, 0x72, 0x4E, 0x13, 0xCE, 0x0E, 0x76, 0x7F,
	0x30, 0x4F, 0x93, 0x55, 0x1E, 0xCF, 0xDB, 0x36, 0x58, 0xEA, 0xBE, 0x7A, 0x5F, 0x43, 0x8F, 0x6D,
	0x89, 0xD6, 0x91, 0x5D, 0x5C, 0x64, 0xF5, 0x00, 0xD8, 0xBA, 0x3C, 0x53, 0x69, 0x61, 0xCC, 0x34,
};

static void initNoiseTable()
{
	int            i;
	float          a;
	float          x, y, z, r, theta;
	float          gradients[256 * 3];
	unsigned int   *p, *psrc;

	srandom(0);

	// build gradient table for 3D noise
	for (i = 0; i < 256; i++)
	{
		/*
		* calculate 1 - 2 * random number
		*/
		a = (random() % 32768) / 32768.0f;
		z = (1.0f - 2.0f * a);

		r = sqrtf(1.0f - z * z); // r is radius of circle

		a = (random() % 32768) / 32768.0f;
		theta = (2.0f * (float)M_PI * a);
		x = (r * cosf(a));
		y = (r * sinf(a));

		gradients[i * 3] = x;
		gradients[i * 3 + 1] = y;
		gradients[i * 3 + 2] = z;
	}

	// use the index in the permutation table to load the
	// gradient values from gradients to gradientTable
	p = (unsigned int *)gradientTable;
	psrc = (unsigned int *)gradients;

	for (i = 0; i < 256; i++)
	{
		int indx = permTable[i];
		p[i * 3] = psrc[indx * 3];
		p[i * 3 + 1] = psrc[indx * 3 + 1];
		p[i * 3 + 2] = psrc[indx * 3 + 2];
	}
}
//
// generate the value of gradient noise for a given lattice point
//
// (ix, iy, iz) specifies the 3D lattice position
// (fx, fy, fz) specifies the fractional part
//
static float glattice3D(int ix, int iy, int iz, float fx, float fy, float fz)
{
	float *g;
	int   indx, y, z;

	z = permTable[iz & NOISE_TABLE_MASK];
	y = permTable[(iy + z) & NOISE_TABLE_MASK];
	indx = (ix + y) & NOISE_TABLE_MASK;
	g = &gradientTable[indx * 3];
//�����ĵ��
	return (g[0] * fx + g[1] * fy + g[2] * fz);
}

//
// generate the 3D noise value
// f describes the input (x, y, z) position for which the noise value needs to be computed
// noise3D returns the scalar noise value
//
static   float noise3D(float *f)
{
	int   ix, iy, iz;
	float fx0, fx1, fy0, fy1, fz0, fz1;
	float wx, wy, wz;
	float vx0, vx1, vy0, vy1, vz0, vz1;

	ix = FLOOR(f[0]);
	fx0 = f[0] - ix;
	fx1 = fx0 - 1;
	wx = smoothstep(fx0);

	iy = FLOOR(f[1]);
	fy0 = f[1] - iy;
	fy1 = fy0 - 1;
	wy = smoothstep(fy0);

	iz = FLOOR(f[2]);
	fz0 = f[2] - iz;
	fz1 = fz0 - 1;
	wz = smoothstep(fz0);

	vx0 = glattice3D(ix, iy, iz, fx0, fy0, fz0);
	vx1 = glattice3D(ix + 1, iy, iz, fx1, fy0, fz0);
	vy0 = lerp(wx, vx0, vx1);
	vx0 = glattice3D(ix, iy + 1, iz, fx0, fy1, fz0);
	vx1 = glattice3D(ix + 1, iy + 1, iz, fx1, fy1, fz0);
	vy1 = lerp(wx, vx0, vx1);
	vz0 = lerp(wy, vy0, vy1);

	vx0 = glattice3D(ix, iy, iz + 1, fx0, fy0, fz1);
	vx1 = glattice3D(ix + 1, iy, iz + 1, fx1, fy0, fz1);
	vy0 = lerp(wx, vx0, vx1);
	vx0 = glattice3D(ix, iy + 1, iz + 1, fx0, fy1, fz1);
	vx1 = glattice3D(ix + 1, iy + 1, iz + 1, fx1, fy1, fz1);
	vy1 = lerp(wx, vx0, vx1);
	vz1 = lerp(wy, vy0, vy1);

	return lerp(wz, vz0, vz1);
}
/*************************************************************************************************/
TGAImage::TGAImage(const  char  *image_name)
{
	int    width;
	int    height;
	_buffer = (unsigned char *)Tools::loadImage(image_name, &width, &height, &_bitDepth);
	assert(_buffer);
	_width = width;
	_height = height;
	_textureId = 0;
}
TGAImage::~TGAImage()
{
	free(_buffer);
	_buffer = NULL;
}
//
unsigned    int       TGAImage::genTextureMap()
{
	if (_textureId)
		return _textureId;
	//�����ӵ�����,֧��8λ��ȵ�����
	static      int       format_table[5] = { 0, GL_RED, 0, GL_RGB, GL_RGBA };
	static      int       other_table[5] = {0,GL_R8,0,GL_RGB,GL_RGBA};
	const      int       _format_type = format_table[_bitDepth>>3];
	assert(_format_type);
//�������Թ���,Ŀǰ���������ʱ��֧��
//	float          _maxAnisotropy = 0.0f;
//	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &_maxAnisotropy);

	glGenTextures(1, &_textureId);
	glBindTexture(GL_TEXTURE_2D, _textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, other_table[_bitDepth >> 3], (int)_width, (int)_height, 0,
		_format_type, GL_UNSIGNED_BYTE, _buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//	 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, _maxAnisotropy);
	glBindTexture(GL_TEXTURE_2D,0);
	return     _textureId;
}

unsigned    TGAImage::genTextureMap(int    _wrapAttrib)
{
	assert(_wrapAttrib==GL_CLAMP || _wrapAttrib==GL_REPEAT || _wrapAttrib==GL_MIRRORED_REPEAT || _wrapAttrib == GL_CLAMP_TO_EDGE);
	if (_textureId)
		return _textureId;
	//�����ӵ�����,֧��8λ��ȵ�����
	static      int       format_table[5] = { 0, GL_RED, 0, GL_RGB, GL_RGBA };
	static      int       other_table[5] = { 0, GL_R8, 0, GL_RGB, GL_RGBA };
	const      int       _format_type = format_table[_bitDepth >> 3];
	assert(_format_type);
	//�������Թ���,Ŀǰ���������ʱ��֧��
	//	float          _maxAnisotropy = 0.0f;
	//	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &_maxAnisotropy);

	glGenTextures(1, &_textureId);
	glBindTexture(GL_TEXTURE_2D, _textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, other_table[_bitDepth >> 3], (int)_width, (int)_height, 0,
		_format_type, GL_UNSIGNED_BYTE, _buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapAttrib);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapAttrib);
	//	 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, _maxAnisotropy);
	glBindTexture(GL_TEXTURE_2D, 0);
	return     _textureId;
}

void          TGAImage::setImageWrapAttrib(int    _wrapAttrib)
{
	assert(_wrapAttrib == GL_CLAMP || _wrapAttrib == GL_REPEAT || _wrapAttrib == GL_MIRRORED_REPEAT || _wrapAttrib == GL_CLAMP_TO_EDGE);
	int   _nowTextureId;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &_nowTextureId);

	glBindTexture(GL_TEXTURE_2D, _textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapAttrib);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapAttrib);

	glBindTexture(GL_TEXTURE_2D, _nowTextureId);
}
unsigned   TGAImage::genVertexBuffer()
{
	GLContext		*_context = GLContext::getInstance();
	Size   _size = _context->getWinSize();
	float    scaleX = _width / _size.width;
	float    scaleY = _height / _size.height;
	float    VertexData[] = {
		-scaleX,scaleY,0.0f,     0.0f,1.0f,
		-scaleX,-scaleY,0.0f,   0.0f,0.0f,
		scaleX,scaleY,0.0f,      1.0f,1.0f,
		scaleX,-scaleY, 0.0,     1.0f,0.0f,
	};
	unsigned   vertexId;
	glGenBuffers(1, &vertexId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	return  vertexId;
}
//
unsigned    TGAImage::gen3DNoiseTextureMap(int textureSize, float frequency)
{
	GLuint    _3DNoiseTextureId = 0;
	GLfloat *texBuf = (GLfloat *) new float[sizeof(GLfloat) * textureSize * textureSize * textureSize];
	GLubyte *uploadBuf = (GLubyte *) new float[sizeof(GLubyte) * textureSize * textureSize * textureSize];
	int x, y, z;
	int index = 0;
	float min = 1000;
	float max = -1000;
	float range;

	initNoiseTable();

	for (z = 0; z < textureSize; z++)
	{
		for (y = 0; y < textureSize; y++)
		{
			for (x = 0; x < textureSize; x++)
			{
				float noiseVal;
				float pos[3] = { (float)x / (float)textureSize, (float)y / (float)textureSize, (float)z / (float)textureSize };
				pos[0] *= frequency;
				pos[1] *= frequency;
				pos[2] *= frequency;
				noiseVal = noise3D(pos);

				if (noiseVal < min)
				{
					min = noiseVal;
				}

				if (noiseVal > max)
				{
					max = noiseVal;
				}

				texBuf[index++] = noiseVal;
			}
		}
	}

	// Normalize to the [0, 1] range
	range = (max - min);
	index = 0;

	for (z = 0; z < textureSize; z++)
	{
		for (y = 0; y < textureSize; y++)
		{
			for (x = 0; x < textureSize; x++)
			{
				float noiseVal = texBuf[index];
				noiseVal = (noiseVal - min) / range;
				uploadBuf[index++] = (GLubyte)(noiseVal * 255.0f);
			}
		}
	}

	glGenTextures(1, &_3DNoiseTextureId);
	glBindTexture(GL_TEXTURE_3D, _3DNoiseTextureId);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, textureSize, textureSize, textureSize, 0,
		GL_RED, GL_UNSIGNED_BYTE, uploadBuf);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

	glBindTexture(GL_TEXTURE_3D, 0);

	free(texBuf);
	free(uploadBuf);
	return     _3DNoiseTextureId;
}
//����������
TGAImageCubeMap::TGAImageCubeMap(const char *imageName[6])
{
	_cubeMapId = 0;
	_imageName[0] = imageName[0];
	_imageName[1] = imageName[1];
	_imageName[2] = imageName[2];
	_imageName[3] = imageName[3];
	_imageName[4] = imageName[4];
	_imageName[5] = imageName[5];
}
TGAImageCubeMap::~TGAImageCubeMap()
{

}
//
int       TGAImageCubeMap::genTextureCubeMap()
{
	if (_cubeMapId)
		return   _cubeMapId;
	glGenTextures(1, (unsigned int *)&_cubeMapId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _cubeMapId);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP);
	//����
	const int         cubeMapEnum[6] = {
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,//0
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,//1
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,//2
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,//3
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,//4
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z//5
	};
	for (int i = 0; i < 6; ++i)
	{
		int       width, height, bitDepth;
		const   char   *buffer = Tools::loadImage(_imageName[i], &width, &height, &bitDepth);
		int      type = bitDepth == 24 ? GL_RGB : GL_RGBA;
		glTexImage2D(cubeMapEnum[i], 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, buffer);
		delete buffer;
	}
//	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP,0);
	return    _cubeMapId;
}

__NS_GLK_END
