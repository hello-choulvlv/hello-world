/*
  *公共函数实现
  *2019年2月9日
  *@author:xiaohuaxiong
 */
#include "csk_types.h"
#include<fstream>
#include<FreeImage/FreeImage.h>

cl_program   create_program(cl_context context, cl_device_id device, const char *filename)
{
	std::ifstream    input_file(filename, std::ios::in | std::ios::binary);
	if (!input_file.is_open())
	{
		printf("file '%s' can not open\n", filename);
		return nullptr;
	}
	input_file.seekg(0, std::ios::end);
	int  size = input_file.tellg();
	input_file.seekg(0, std::ios::beg);

	char  *buffer = new char[size + 2];
	input_file.read(buffer, size);
	buffer[size] = 0;
	input_file.close();
	//Compile
	cl_program  program = clCreateProgramWithSource(context, 1, (const char **)&buffer, nullptr, nullptr);
	if (!program)
	{
		printf("failed to create program,file->%s\n", filename);
		return nullptr;
	}
	int result = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
	if (result != CL_SUCCESS)
	{
		size_t  buffer_size = 0;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &buffer_size);
		char  *buffer = new char[buffer_size + 2];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, buffer_size, buffer, nullptr);
		buffer[buffer_size] = 0;
		printf("Compile error:%s\n", buffer);
		delete[] buffer;
		buffer = nullptr;
		clReleaseProgram(program);
		program = nullptr;
	}
	return program;
}

cl_mem  create_texture(cl_context context, const char *filename, int *width, int *height)
{
	FREE_IMAGE_FORMAT   image_format = FreeImage_GetFileType(filename,0);
	FIBITMAP  *image = FreeImage_Load(image_format, filename);

	FIBITMAP  *t_image = FreeImage_ConvertTo32Bits(image);
	FreeImage_Unload(image);

	int  width_t = FreeImage_GetWidth(t_image);
	int  height_t = FreeImage_GetHeight(t_image);

	BYTE  *image_data = FreeImage_GetBits(t_image);

	cl_image_format  image_format_info = {
		CL_RGBA,
		CL_UNORM_INT8,
	};
	cl_int  result = 0;
	cl_mem   image_buffer = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &image_format_info, width_t, height_t, 0, image_data, &result);
	if (width)  *width = width_t;
	if (height) *height = height_t;

	FreeImage_Unload(t_image);
	return image_buffer;
}

void  save_texture(cl_context context,const char *buffer, const char *filename, int width, int height)
{
	//保存为png
	FREE_IMAGE_FORMAT   image_format = FREE_IMAGE_FORMAT::FIF_PNG;
	FIBITMAP   *image_raw = FreeImage_ConvertFromRawBits((BYTE*)(buffer),width,height,width * 4,32,0xFF000000,0xFF0000,0xFF00,0);

	FreeImage_Save(image_format, image_raw, filename, 0);
}