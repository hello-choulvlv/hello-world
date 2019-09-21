/*
  *第一个Vulkan三角形程序
  *2018年9月12日
  *@author:xiaohuaxiong
 */
#define GLFW_INCLUDE_VULKAN
#include "glfw/glfw3.h"
#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "Geometry.h"
#include "stb_image.h"

#define CSVK_Assert(condition,msg)  \
if(!(condition))\
{\
std::cout<<"error occurred,in file" <<__FILE__<<", function"<<__FUNCTION__<<__LINE__<<",because "<<msg<<std::endl;\
assert(condition);\
}
#define CSVK_Clear(m)   memset(m,0,sizeof(m))
#define max_f(x,y) (x)>(y)?(x):(y)
#define min_f(x,y) (x)<(y)?(x):(y)

static const  int  s_window_width = 800;
static const  int  s_window_height = 600;

static  const char *s_validate_layer_name ="VK_LAYER_LUNARG_standard_validation";
static  const char *s_device_extensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

static const  bool   s_enable_validate_layer = true;

struct  CSVKSample
{
	VkPhysicalDevice		physicalDevice;//物理设备
	VkDevice						logicalDevice;//逻辑设备
	VkInstance                   csvkInstance;//Vulkan实例化

	VkPhysicalDeviceMemoryProperties  physicalDeviceMemoryProperty;

	uint32_t						graphicsFamilyIndex;//图形队列家族索引
	uint32_t						presentFamilyIndex;//提交队列家族索引
	VkQueue                       graphicsQueue, presentQueue;

	VkSwapchainKHR		swapChainKHR;//交换链
	VkFormat                      swapChainImageFormat;
	VkExtent2D                  swapChainExtent;
	uint32_t                        swapChainImageCount;
	std::vector<VkImage>           swapchainImages;
	std::vector<VkImageView>  swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	//深度缓冲区对象
	VkImage                        depthImage;
	VkDeviceMemory        depthMemory;
	VkImageView               depthImageView;
	VkFormat                      depthFormat;

	GLFWwindow             *window;//glfw-window指针
	VkSurfaceKHR            csvkSurfaceKHR;//表面Surface
	VkSurfaceCapabilitiesKHR  csvkSurfaceCapability;//Surface-Capability
	VkSurfaceFormatKHR		    csvkSurfaceFormat;//表面的格式
	VkPresentModeKHR              presentModel;//提交的模式
	//硬件的Surface格式数目/支持提交的模式
	uint32_t									surfaceFormatCount;
	std::vector<VkSurfaceFormatKHR>			surfaceFormats;
	uint32_t                                    presentModeCount;
	std::vector<VkPresentModeKHR>             presentModes;
	//渲染通道
	VkRenderPass              renderPass;
	VkPipelineLayout        pipelineLayout;
	VkPipeline                     graphicsPipeline;
	//命令缓冲区对象
	VkCommandPool        commandPool;
	std::vector<VkCommandBuffer>  commandBuffers;
	//顶点缓冲区对象,以及内存绑定
	VkBuffer                        vertexBuffer;
	VkDeviceMemory        vertexMemory;
	//索引缓冲区对象
	VkBuffer                         indexBuffer;
	VkDeviceMemory         indexMemory;
	//uniform变量缓冲区对象
	VkBuffer                         uniformBuffer;
	VkDeviceMemory         uniformMemory;
	//texture-image-view
	VkImage                                texture;
	VkDeviceMemory                textureMemory;
	VkImageView                       textureView;
	VkSampler                            sampler;
	//描述符对象池
	VkDescriptorPool				descriptorPool;
	VkDescriptorSetLayout	    descriptorSetLayout;
	VkDescriptorSet                  descriptorSet;
	//信号灯
	VkSemaphore               imageSemaphore, renderSemaphore;
	/*debug 回调*/
	VkDebugReportCallbackEXT   debugCallback;
};
/*
  *创建Vulkan实例
 */
void  init_vulkan_instance(CSVKSample &CSSample)
{
	/*create vulkan instance.*/
	VkApplicationInfo    csvkAppInfo = {};
	csvkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	csvkAppInfo.pApplicationName = "CSVKSample";
	csvkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	csvkAppInfo.pEngineName = "CSVK_engine";
	csvkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	csvkAppInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	/*加载Vulkan需要的扩展*/
	uint32_t    glfw_require_extension_count = 0;
	const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_require_extension_count);
	char   **csvk_need_extensions = new  char*[glfw_require_extension_count + 2];
	memcpy(csvk_need_extensions, glfw_extensions, sizeof(char*)*glfw_require_extension_count);
	csvk_need_extensions[glfw_require_extension_count] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
	csvk_need_extensions[glfw_require_extension_count + 1] = nullptr;

	/*VKInstanceCreateInfo*/
	VkInstanceCreateInfo   csvkInstanceCreateInfo = {};
	csvkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	csvkInstanceCreateInfo.pApplicationInfo = &csvkAppInfo;
	csvkInstanceCreateInfo.enabledExtensionCount = glfw_require_extension_count + 1;
	csvkInstanceCreateInfo.ppEnabledExtensionNames = csvk_need_extensions;
	if (s_enable_validate_layer)
	{
		csvkInstanceCreateInfo.enabledLayerCount = 1;
		csvkInstanceCreateInfo.ppEnabledLayerNames = &s_validate_layer_name;
	}
	VkResult  result = vkCreateInstance(&csvkInstanceCreateInfo, nullptr, &CSSample.csvkInstance);
	CSVK_Assert(result == VK_SUCCESS, "could not create vulkan instance.");
	delete csvk_need_extensions;
	csvk_need_extensions = nullptr;
}
/*
  *Vulkan的错误回调函数
 */
VKAPI_ATTR  VkBool32  VKAPI_CALL  CSVK_debugCallback(VkDebugReportFlagsEXT flags,VkDebugReportObjectTypeEXT  objectType,uint64_t  obj,size_t location,int32_t  code,const char *layerPrefix,const char *msg,void *userdata)
{
	std::cerr << "validation layer: "<<msg << std::endl;
	return VK_FALSE;
}
/*
  *注册Vulkan错误回调函数
 */
VkResult  init_debug_callback(CSVKSample  &CSSample)
{
	VkDebugReportCallbackCreateInfoEXT  debug_create_info = {};
	debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debug_create_info.pfnCallback = CSVK_debugCallback;

	auto implicit_func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(CSSample.csvkInstance, "vkCreateDebugReportCallbackEXT");
	if (implicit_func != nullptr)
		return implicit_func(CSSample.csvkInstance,&debug_create_info,nullptr,&CSSample.debugCallback);
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}
/*
  *检测物理设备的队列家族属性
 */
bool check_physical_device_queue_family(CSVKSample &CSSample,VkPhysicalDevice  physical_device,uint32_t  *graphics_index,uint32_t *present_index)
{
	uint32_t  queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties>   queue_family_property(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_property.data());

	uint32_t   index_g = -1,index_p =-1;
	for (int k = 0; k < queue_family_count; ++k)
	{
		auto &queue = queue_family_property[k];
		if (queue.queueCount > 0 && queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			index_g = k;
		//检测该队列是否支持提交
		VkBool32  present_support = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, k, CSSample.csvkSurfaceKHR, &present_support);
		if (queue.queueCount > 0 && present_support)
			index_p = k;
		if (index_g != -1 && index_p != -1)
			break;
	}

	*graphics_index = index_g;
	*present_index = index_p;

	return index_g != -1 && index_p != -1;
}
/*
  *检测物理设备是否支持交换链
 */
bool check_physical_device_support_swapchain(VkPhysicalDevice  physical_device)
{
	uint32_t  extension_count = 0;
	vkEnumerateDeviceExtensionProperties(physical_device,nullptr,&extension_count,nullptr);
	std::vector<VkExtensionProperties>  extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data());

	/*check*/
	for (auto it = extensions.begin(); it != extensions.end(); ++it)
	{
		if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, it->extensionName))
			return true;
	}
	return false;
}
/*
  *检测设备的交换链对颜色格式以及提交模式的支持
 */
bool check_swapchain_format_present_support(CSVKSample  &CSSample,VkPhysicalDevice physical_device)
{
	uint32_t   devvice_surface_format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, CSSample.csvkSurfaceKHR, &devvice_surface_format_count, nullptr);

	std::vector<VkSurfaceFormatKHR>  device_surface_formats(devvice_surface_format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, CSSample.csvkSurfaceKHR, &devvice_surface_format_count, device_surface_formats.data());

	uint32_t device_surface_present_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, CSSample.csvkSurfaceKHR, &device_surface_present_count, nullptr);

	std::vector<VkPresentModeKHR>  device_surface_presents(device_surface_present_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, CSSample.csvkSurfaceKHR, &device_surface_present_count,device_surface_presents.data());

	return devvice_surface_format_count * device_surface_present_count != 0;
}
/*
  *选择物理设备
 */
void init_select_physical_device(CSVKSample  &CSSample)
{
	uint32_t  physical_device_count = 0;
	vkEnumeratePhysicalDevices(CSSample.csvkInstance, &physical_device_count, nullptr);
	VkPhysicalDevice  *physical_devices = new VkPhysicalDevice[physical_device_count+1];
	vkEnumeratePhysicalDevices(CSSample.csvkInstance, &physical_device_count, physical_devices);

	CSVK_Assert(physical_device_count > 0,"Could not find physical device.");
	CSSample.physicalDevice = nullptr;
	/*检查物理设备是否满足要求*/
	for (int k = 0; k < physical_device_count; ++k)
	{
		bool  family_support = check_physical_device_queue_family(CSSample, physical_devices[k], &CSSample.graphicsFamilyIndex, &CSSample.presentFamilyIndex);
		bool  swapchain_support = check_physical_device_support_swapchain(physical_devices[k]);
		bool swapchain_format_present_adequate = check_swapchain_format_present_support(CSSample, physical_devices[k]);

		if (family_support && swapchain_support && swapchain_format_present_adequate)
		{
			CSSample.physicalDevice = physical_devices[k];
			break;
		}
	}

	delete physical_devices;
	physical_devices = nullptr;
	CSVK_Assert(CSSample.physicalDevice!=nullptr,"Could not find a fitable physical device.");

	/*获取物理设备的内存属性*/
	vkGetPhysicalDeviceMemoryProperties(CSSample.physicalDevice, &CSSample.physicalDeviceMemoryProperty);
}

void  init_logic_device(CSVKSample  &CSSample)
{
	uint32_t    queue_family[2] = {CSSample.graphicsFamilyIndex,0};
	uint32_t    queue_count = 1;
	if (CSSample.graphicsFamilyIndex != CSSample.presentFamilyIndex)
		queue_family[queue_count++] = CSSample.presentFamilyIndex;
	//create queue
	VkDeviceQueueCreateInfo    device_queue_info[2];
	float  queue_priority = 1.0;
	for (int k = 0; k < queue_count; ++k)
	{
		device_queue_info[k].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_info[k].pNext = 0;
		device_queue_info[k].flags = 0;
		device_queue_info[k].queueFamilyIndex = queue_family[k];
		device_queue_info[k].queueCount = 1;
		device_queue_info[k].pQueuePriorities = &queue_priority;
	}

	VkPhysicalDeviceFeatures    physical_device_feature = {};
	physical_device_feature.samplerAnisotropy = VK_TRUE;

	const char *extensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	const char *layer_names = "VK_LAYER_LUNARG_standard_validation";

	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = queue_count;
	device_create_info.pQueueCreateInfos = device_queue_info;
	device_create_info.pEnabledFeatures = &physical_device_feature;
	device_create_info.enabledExtensionCount = 1;
	device_create_info.ppEnabledExtensionNames = &extensions;
	device_create_info.enabledLayerCount = 1;
	device_create_info.ppEnabledLayerNames = &layer_names;


	VkResult  result = vkCreateDevice(CSSample.physicalDevice, &device_create_info, nullptr, &CSSample.logicalDevice);
	CSVK_Assert(result == VK_SUCCESS,"create vulkan logical device failed");

	vkGetDeviceQueue(CSSample.logicalDevice, CSSample.graphicsFamilyIndex, 0, &CSSample.graphicsQueue);
	vkGetDeviceQueue(CSSample.logicalDevice,CSSample.presentFamilyIndex,0,&CSSample.presentQueue);
}
/*
  *查询创建交换链时硬件的支持
 */
void  init_query_physical_device_support(CSVKSample  &Sample)
{
	/*查询硬件的性能Capability*/
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Sample.physicalDevice, Sample.csvkSurfaceKHR, &Sample.csvkSurfaceCapability);

	/*查询Surface支持的格式*/
	vkGetPhysicalDeviceSurfaceFormatsKHR(Sample.physicalDevice, Sample.csvkSurfaceKHR, &Sample.surfaceFormatCount, nullptr);
	Sample.surfaceFormats.resize(Sample.surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(Sample.physicalDevice, Sample.csvkSurfaceKHR, &Sample.surfaceFormatCount, Sample.surfaceFormats.data());
	
	/*查询硬件支持提交的模式*/
	vkGetPhysicalDeviceSurfacePresentModesKHR(Sample.physicalDevice, Sample.csvkSurfaceKHR, &Sample.presentModeCount, nullptr);
	Sample.presentModes.resize(Sample.presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(Sample.physicalDevice, Sample.csvkSurfaceKHR, &Sample.presentModeCount, Sample.presentModes.data());
}
/*
  *创建交换链
 */
void  init_create_swapchain(CSVKSample &Sample)
{
	/*查询交换链对应的物理硬件的支持颜色格式/提交模式的支持*/
	init_query_physical_device_support(Sample);
	/*对获取的颜色格式进行分析*/
	VkSurfaceFormatKHR   surface_format_khr;
	if (Sample.surfaceFormats.size() == 1 && Sample.surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		surface_format_khr.format = VK_FORMAT_B8G8R8A8_UNORM;
		surface_format_khr.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	}
	else
	{
		bool   check_found = false;
		for (auto it = Sample.surfaceFormats.begin(); it != Sample.surfaceFormats.end(); ++it)
		{
			if (it->format == VK_FORMAT_B8G8R8A8_UNORM && it->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				surface_format_khr.format = VK_FORMAT_B8G8R8A8_UNORM;
				surface_format_khr.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
				check_found = true;
				break;
			}
		}
		if (!check_found)
			surface_format_khr = Sample.surfaceFormats[0];
	}

	/*对获取的提交模式进行分析*/
	VkPresentModeKHR  present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (auto it = Sample.presentModes.begin(); it != Sample.presentModes.end(); ++it)
	{
		if (*it == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			present_mode = *it;
			break;
		}
		else if (*it == VK_PRESENT_MODE_IMMEDIATE_KHR)
			present_mode = *it;
	}

	/*获取交换链的matrice*/
	VkExtent2D    extent;
	if(Sample.csvkSurfaceCapability.currentExtent.width != -1)
		extent = Sample.csvkSurfaceCapability.currentExtent;
	else
	{
		extent.width = s_window_width;
		extent.height = s_window_height;

		extent.width = max_f(Sample.csvkSurfaceCapability.minImageExtent.width,min_f(Sample.csvkSurfaceCapability.maxImageExtent.width,extent.width));
		extent.height = max_f(Sample.csvkSurfaceCapability.minImageExtent.height,min_f(Sample.csvkSurfaceCapability.maxImageExtent.height,extent.height));
	}

	/*交换链中图像的数目*/
	uint32_t   image_count = Sample.csvkSurfaceCapability.minImageCount + 1;
	if (Sample.csvkSurfaceCapability.maxImageCount > 0 && image_count > Sample.csvkSurfaceCapability.maxImageCount)
		image_count = Sample.csvkSurfaceCapability.maxImageCount;

	/*将以上得出的数据记录到数据结构中*/
	Sample.swapChainImageCount = image_count;
	Sample.swapChainImageFormat = surface_format_khr.format;
	Sample.swapChainExtent = extent;

	VkSwapchainCreateInfoKHR   swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = Sample.csvkSurfaceKHR;
	swapchain_create_info.minImageCount = image_count;
	swapchain_create_info.imageFormat = surface_format_khr.format;
	swapchain_create_info.imageColorSpace = surface_format_khr.colorSpace;
	swapchain_create_info.imageExtent = extent;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//队列家族索引
	uint32_t   queue_family_index2[2] = {Sample.graphicsFamilyIndex,Sample.presentFamilyIndex};
	if (Sample.graphicsFamilyIndex != Sample.presentFamilyIndex)
	{
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = queue_family_index2;
	}
	else
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.preTransform = Sample.csvkSurfaceCapability.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = present_mode;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(Sample.logicalDevice, &swapchain_create_info, nullptr, &Sample.swapChainKHR);
	CSVK_Assert(result == VK_SUCCESS,"Create Swapchain Failed.");

	vkGetSwapchainImagesKHR(Sample.logicalDevice, Sample.swapChainKHR, &Sample.swapChainImageCount, nullptr);
	Sample.swapchainImages.resize(Sample.swapChainImageCount);
	vkGetSwapchainImagesKHR(Sample.logicalDevice,Sample.swapChainKHR,&Sample.swapChainImageCount,Sample.swapchainImages.data());
}
/*
  *创建交换链中的ImageView
 */
void  init_swapchain_imageview(CSVKSample  &Sample)
{
	Sample.swapChainImageViews.resize(Sample.swapChainImageCount);
	for (int k = 0; k < Sample.swapChainImageCount; ++k)
	{
		VkImageViewCreateInfo  image_view_info = {};
		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.image = Sample.swapchainImages[k];
		image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_info.format = Sample.swapChainImageFormat;
		image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.layerCount = 1;
		image_view_info.subresourceRange.levelCount = 1;

		VkResult result = vkCreateImageView(Sample.logicalDevice, &image_view_info, nullptr, &Sample.swapChainImageViews[k]);
		CSVK_Assert(result == VK_SUCCESS,"Create Image View Failed.");
	}
}
/*
  *生成渲染通道
 */
void  init_renderpass(CSVKSample  &Sample)
{
	VkAttachmentDescription  colorDepthAttach[2] = {};
	colorDepthAttach[0].flags = 0;
	colorDepthAttach[0].format = Sample.swapChainImageFormat;
	colorDepthAttach[0].samples = VK_SAMPLE_COUNT_1_BIT;
	colorDepthAttach[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorDepthAttach[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorDepthAttach[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorDepthAttach[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorDepthAttach[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorDepthAttach[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	colorDepthAttach[1].flags = 0;
	colorDepthAttach[1].format = Sample.depthFormat;
	colorDepthAttach[1].samples = VK_SAMPLE_COUNT_1_BIT;
	colorDepthAttach[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorDepthAttach[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorDepthAttach[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorDepthAttach[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorDepthAttach[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	/*attachment之间的引用*/
	VkAttachmentReference    colorAttachRef = {};
	colorAttachRef.attachment = 0;
	colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	/*子通道之间的描述*/
	VkSubpassDescription    subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &colorAttachRef;

	/*通道之间的依赖关系*/
	VkSubpassDependency   subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstSubpass = 0;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo  renderpass_info = {};
	renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpass_info.attachmentCount = 2;
	renderpass_info.pAttachments = colorDepthAttach;
	renderpass_info.subpassCount = 1;
	renderpass_info.pSubpasses = &subpassDesc;
	renderpass_info.dependencyCount = 1;
	renderpass_info.pDependencies = &subpassDependency;

	VkResult  result = vkCreateRenderPass(Sample.logicalDevice, &renderpass_info, nullptr, &Sample.renderPass);
	CSVK_Assert(result == VK_SUCCESS,"Create RenderPass Failed.");
}
/*
  *从文件中读取二进制内容
 */
bool   csvk_read_file(const char *filename, char **code, uint32_t  &length_s)
{
	std::ifstream   stream_input(filename, std::ios::binary);
	if (!stream_input.is_open())
		return false;
	stream_input.seekg(0,std::ios::end);
	length_s = stream_input.tellg();

	*code = new char[length_s + 4];
	stream_input.seekg(0,std::ios::beg);
	stream_input.read(*code, length_s);
	stream_input.close();

	return true;
}
/*
  *加载SPV-Shader模块
 */
VkShaderModule  init_create_spv_shader(CSVKSample &Sample,char *code, uint32_t  length_s)
{
	VkShaderModuleCreateInfo  shader_module_info = {};
	shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_info.codeSize = length_s;
	shader_module_info.pCode = reinterpret_cast<uint32_t*>(code);

	VkShaderModule  shader_module;
	VkResult result = vkCreateShaderModule(Sample.logicalDevice,&shader_module_info,nullptr,&shader_module);
	CSVK_Assert(result == VK_SUCCESS,"Create Shader Module Failed.");
	return shader_module;
}
/*
  *创建图形管线
 */
void  init_create_graphics_pipeline(CSVKSample  &Sample)
{
	char  *code = nullptr;
	uint32_t	length_s = 0;
	bool    success = csvk_read_file("shaders/vert.spv",&code,length_s);
	CSVK_Assert(success,"Could not read file shader/vert.spv");
	VkShaderModule  shader_vert_module = init_create_spv_shader(Sample, code, length_s);
	delete code;
	length_s = 0;

	success = csvk_read_file("shaders/frag.spv",&code,length_s);
	CSVK_Assert(success,"Could not read file shader/frag.spv");
	VkShaderModule  shader_frag_module = init_create_spv_shader(Sample, code, length_s);
	delete code;
	length_s = 0;

	/*Pipeline Shader Stage*/
	VkPipelineShaderStageCreateInfo	 shader_stage_info[2] = {};
	shader_stage_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stage_info[0].module = shader_vert_module;
	shader_stage_info[0].pName = "main";

	shader_stage_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stage_info[1].module = shader_frag_module;
	shader_stage_info[1].pName = "main";

	/*顶点输入状态绑定描述*/
	VkVertexInputBindingDescription   vertex_input_bind_des = {};
	vertex_input_bind_des.binding = 0;
	vertex_input_bind_des.stride = sizeof(float) * 8;
	vertex_input_bind_des.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	/*顶点属性描述符*/
	VkVertexInputAttributeDescription  vertex_input_attrib_desc[3] = {};
	vertex_input_attrib_desc[0].binding = 0;
	vertex_input_attrib_desc[0].location = 0;
	vertex_input_attrib_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertex_input_attrib_desc[0].offset = 0;

	vertex_input_attrib_desc[1].binding = 0;
	vertex_input_attrib_desc[1].location = 1;
	vertex_input_attrib_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertex_input_attrib_desc[1].offset = sizeof(float)*3;

	vertex_input_attrib_desc[2].binding = 0;
	vertex_input_attrib_desc[2].location = 2;
	vertex_input_attrib_desc[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertex_input_attrib_desc[2].offset = sizeof(float)*6;

	/*输入汇编阶段*/
	VkPipelineInputAssemblyStateCreateInfo  input_assembly_info = {};
	input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;

	/*顶点输入阶段*/
	VkPipelineVertexInputStateCreateInfo   vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &vertex_input_bind_des;
	vertex_input_info.vertexAttributeDescriptionCount = 3;
	vertex_input_info.pVertexAttributeDescriptions = vertex_input_attrib_desc;

	/*视口变换*/
	VkViewport  viewport_info = {};
	viewport_info.x = 0;
	viewport_info.y = 0;
	viewport_info.width = Sample.swapChainExtent.width;
	viewport_info.height = Sample.swapChainExtent.height;
	viewport_info.minDepth = 0.0f;
	viewport_info.maxDepth = 1.0f;

	/*裁剪阶段*/
	VkRect2D    scissor_info = {};
	scissor_info.offset.x = 0;
	scissor_info.offset.y = 0;
	scissor_info.extent = Sample.swapChainExtent;

	/*适口变换阶段*/
	VkPipelineViewportStateCreateInfo   viewport_create_info = {};
	viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_create_info.pNext = nullptr;
	viewport_create_info.viewportCount = 1;
	viewport_create_info.pViewports = &viewport_info;
	viewport_create_info.scissorCount = 1;
	viewport_create_info.pScissors = &scissor_info;

	/*光栅化阶段*/
	VkPipelineRasterizationStateCreateInfo   rasterization_create_info = {};
	rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_create_info.depthClampEnable = VK_FALSE;
	rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_create_info.lineWidth = 1.0f;
	rasterization_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_create_info.depthBiasEnable = VK_FALSE;
	
	/*深度测试阶段*/
	VkPipelineDepthStencilStateCreateInfo  depthstencil_create_info = {};
	depthstencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthstencil_create_info.depthTestEnable = VK_TRUE;
	depthstencil_create_info.depthWriteEnable = VK_TRUE;
	depthstencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthstencil_create_info.depthBoundsTestEnable = VK_FALSE;
	depthstencil_create_info.stencilTestEnable = VK_FALSE;
	depthstencil_create_info.minDepthBounds = 0.0f;
	depthstencil_create_info.maxDepthBounds = 1.0f;

	/*多重采样阶段*/
	VkPipelineMultisampleStateCreateInfo  multisample_create_info = {};
	multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_create_info.sampleShadingEnable = VK_FALSE;
	multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	/*颜色混溶附着阶段*/
	VkPipelineColorBlendAttachmentState  colorblend_attach = {};
	colorblend_attach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT| VK_COLOR_COMPONENT_A_BIT;
	colorblend_attach.blendEnable = VK_FALSE;

	/*颜色混溶阶段*/
	VkPipelineColorBlendStateCreateInfo  colorblend_create_info = {};
	colorblend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorblend_create_info.logicOpEnable = VK_FALSE;
	colorblend_create_info.logicOp = VK_LOGIC_OP_COPY;
	colorblend_create_info.attachmentCount = 1;
	colorblend_create_info.pAttachments = &colorblend_attach;

	/*管线布局*/
	VkPipelineLayoutCreateInfo  layout_create_info = {};
	layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_create_info.setLayoutCount = 1;
	layout_create_info.pSetLayouts = &Sample.descriptorSetLayout;
	layout_create_info.pushConstantRangeCount = 0;
	VkResult result = vkCreatePipelineLayout(Sample.logicalDevice, &layout_create_info, nullptr, &Sample.pipelineLayout);
	CSVK_Assert(result == VK_SUCCESS,"Create Pipeline Layout Failed.");

	VkGraphicsPipelineCreateInfo  graphics_pipeline_info = {};
	graphics_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphics_pipeline_info.stageCount = 2;
	graphics_pipeline_info.pStages = shader_stage_info;
	graphics_pipeline_info.pVertexInputState = &vertex_input_info;
	graphics_pipeline_info.pInputAssemblyState = &input_assembly_info;
	graphics_pipeline_info.pViewportState = &viewport_create_info;
	graphics_pipeline_info.pRasterizationState = &rasterization_create_info;
	graphics_pipeline_info.pMultisampleState = &multisample_create_info;
	graphics_pipeline_info.pDepthStencilState = &depthstencil_create_info;
	graphics_pipeline_info.pColorBlendState = &colorblend_create_info;
	graphics_pipeline_info.layout = Sample.pipelineLayout;
	graphics_pipeline_info.renderPass = Sample.renderPass;
	graphics_pipeline_info.subpass = 0;
	graphics_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	result = vkCreateGraphicsPipelines(Sample.logicalDevice, VK_NULL_HANDLE, 1, &graphics_pipeline_info, nullptr, &Sample.graphicsPipeline);
	CSVK_Assert(result == VK_SUCCESS,"Create Graphics Pipeline Failed.");

	vkDestroyShaderModule(Sample.logicalDevice, shader_vert_module, nullptr);
	vkDestroyShaderModule(Sample.logicalDevice, shader_frag_module, nullptr);
}
/*
  *创建帧缓冲区对象
 */
void  init_framebuffer(CSVKSample  &Sample)
{
	VkImageView  *imageViews = Sample.swapChainImageViews.data();
	Sample.swapChainFramebuffers.resize(Sample.swapChainImageCount);

	for (int k = 0; k < Sample.swapChainImageCount; ++ k)
	{
		VkImageView  image_views[2] = {imageViews[k],Sample.depthImageView};
		VkFramebufferCreateInfo  framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = Sample.renderPass;
		framebuffer_info.attachmentCount = 2;
		framebuffer_info.pAttachments = image_views;
		framebuffer_info.width = Sample.swapChainExtent.width;
		framebuffer_info.height = Sample.swapChainExtent.height;
		framebuffer_info.layers = 1;
		VkResult result = vkCreateFramebuffer(Sample.logicalDevice, &framebuffer_info, nullptr, Sample.swapChainFramebuffers.data() + k);
		CSVK_Assert(result == VK_SUCCESS,"Create Framebuffer Failed.");
	}
}
/*
  *创建命令池
 */
void  init_command_pool(CSVKSample  &Sample)
{
	VkCommandPoolCreateInfo  cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.queueFamilyIndex = Sample.graphicsFamilyIndex;
	
	VkResult result = vkCreateCommandPool(Sample.logicalDevice, &cmd_pool_info, nullptr, &Sample.commandPool);
	CSVK_Assert(result == VK_SUCCESS,"Create Command Pool Failed.");
}
/*
  *检索内存的类型
 */
uint32_t   check_memory_type_index(CSVKSample  &Sample,uint32_t  target_bit, VkMemoryPropertyFlags  require_bit)
{
	for (int k = 0; k < Sample.physicalDeviceMemoryProperty.memoryTypeCount; ++k)
	{
		/*检索是否支持目标索引所代表的内存类型,以及该内存类型是否支持目标特性*/
		int   bit_index = 1 << k;
		if ((target_bit & bit_index) && (Sample.physicalDeviceMemoryProperty.memoryTypes[k].propertyFlags & require_bit) == require_bit)
		{
			return k;
		}
	}
	return -1;
}
/*
  *复制缓冲区对象
 */
void  copy_vertex_buffer(CSVKSample  &Sample,VkBuffer  src_buffer,VkBuffer  dst_buffer,uint32_t  buffer_size)
{
	/*使用命令缓冲区对象,并提交到图形队列中*/
	VkCommandBufferAllocateInfo  cmd_buffer_alloc = {};
	cmd_buffer_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_buffer_alloc.commandPool = Sample.commandPool;
	cmd_buffer_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd_buffer_alloc.commandBufferCount = 1;

	VkCommandBuffer  stag_cmd_buffer=nullptr;
	VkResult  result = vkAllocateCommandBuffers(Sample.logicalDevice, &cmd_buffer_alloc, &stag_cmd_buffer);
	CSVK_Assert(result==VK_SUCCESS,"Create Staging Command Buffer Failed.");

	VkCommandBufferBeginInfo  cmd_buffer_begin = {};
	cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buffer_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(stag_cmd_buffer,&cmd_buffer_begin);

	VkBufferCopy  buffer_copy = {};
	buffer_copy.srcOffset = 0;
	buffer_copy.dstOffset = 0;
	buffer_copy.size = buffer_size;
	vkCmdCopyBuffer(stag_cmd_buffer, src_buffer, dst_buffer, 1, &buffer_copy);

	vkEndCommandBuffer(stag_cmd_buffer);

	VkSubmitInfo  submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &stag_cmd_buffer;
	vkQueueSubmit(Sample.graphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(Sample.graphicsQueue);

	vkFreeCommandBuffers(Sample.logicalDevice, Sample.commandPool, 1, &stag_cmd_buffer);
}
/*
  *创建缓冲区对象
 */
void  create_target_vertex_buffer(CSVKSample  &Sample,uint32_t  buffer_size,VkBufferUsageFlags  buffer_usage, VkMemoryPropertyFlagBits properties,VkBuffer &buffer,VkDeviceMemory &memory)
{
	VkBufferCreateInfo  buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = buffer_size;
	buffer_create_info.usage = buffer_usage;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkResult result = vkCreateBuffer(Sample.logicalDevice, &buffer_create_info, nullptr, &buffer);
	CSVK_Assert(result == VK_SUCCESS,"Create target buffer failed.");

	VkMemoryRequirements  memory_require;
	vkGetBufferMemoryRequirements(Sample.logicalDevice, buffer, &memory_require);

	VkMemoryAllocateInfo  memory_alloc_info = {};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memory_require.size;
	memory_alloc_info.memoryTypeIndex = check_memory_type_index(Sample, memory_require.memoryTypeBits, properties);
	result = vkAllocateMemory(Sample.logicalDevice, &memory_alloc_info, nullptr, &memory);
	CSVK_Assert(result == VK_SUCCESS,"Alloc Memory For vertex buffer creation failed.");

	vkBindBufferMemory(Sample.logicalDevice, buffer, memory, 0);
}
/*
  *创建顶点缓冲区对象
 */
void  init_vertex_buffer(CSVKSample  &Sample)
{
	float  vertex_data[] = {
		-0.5f,-0.5f,0,  1.0f,1.0f,0.0f,0,1,  //0
		0.5f,-0.5f,0,  1.0f,0.0f,0.0f, 1,1, //1
		0.5f,0.5f,0,  0.0f,1.0f,0.0f,  1,0,//2
		-0.5f,0.5f,0,  0.0f,0.0f,1.0f,0,0,//3

		- 1.0f,-1.0f,1,  0.0f,1.0f,0.0f,0,1,  //4
		0.0f,-1.f,1,  0.0f,0.0f,1.0f, 1,1, //5
		0.0f,0.0f,1,  0.0f,1.0f,1.0f,  1,0,//6
		-1.f,0.0f,1,  1.0f,0.0f,1.0f,0,0//7
	};

	VkBuffer   stag_buffer = VK_NULL_HANDLE;
	VkDeviceMemory    stag_memory;
	create_target_vertex_buffer(Sample, sizeof(vertex_data),VK_BUFFER_USAGE_TRANSFER_SRC_BIT,static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),stag_buffer,stag_memory);

	void  *memory_data = nullptr;
	VkResult  result = vkMapMemory(Sample.logicalDevice, stag_memory, 0, sizeof(vertex_data), 0, &memory_data);
	CSVK_Assert(result == VK_SUCCESS,"Map Memory Buffer Failed.");
	memcpy(memory_data,vertex_data,sizeof(vertex_data));
	vkUnmapMemory(Sample.logicalDevice, stag_memory);

	create_target_vertex_buffer(Sample, sizeof(vertex_data),VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,Sample.vertexBuffer,Sample.vertexMemory);

	copy_vertex_buffer(Sample, stag_buffer, Sample.vertexBuffer, sizeof(vertex_data));

	vkDestroyBuffer(Sample.logicalDevice, stag_buffer, nullptr);
	vkFreeMemory(Sample.logicalDevice, stag_memory, nullptr);
}
/*
  *创建索引缓冲区对象
 */
void  init_index_buffer(CSVKSample  &Sample)
{
	uint16_t    index_data[12] = {
		0,1,3,3,1,2,
		4,5,7,7,5,6,
	};
	VkBuffer  stage_buffer = VK_NULL_HANDLE;
	VkDeviceMemory   stage_memory = VK_NULL_HANDLE;
	create_target_vertex_buffer(Sample, sizeof(index_data),VK_BUFFER_USAGE_TRANSFER_SRC_BIT,static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),stage_buffer,stage_memory);

	void  *memory_data = nullptr;
	VkResult result = vkMapMemory(Sample.logicalDevice, stage_memory, 0, sizeof(index_data), 0, &memory_data);
	CSVK_Assert(result == VK_SUCCESS, "VKMapMemory For index buffer failed.");
	memcpy(memory_data, index_data, sizeof(index_data));
	vkUnmapMemory(Sample.logicalDevice, stage_memory);

	create_target_vertex_buffer(Sample, sizeof(index_data), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,Sample.indexBuffer,Sample.indexMemory);

	copy_vertex_buffer(Sample, stage_buffer, Sample.indexBuffer, sizeof(index_data));

	vkDestroyBuffer(Sample.logicalDevice, stage_buffer,nullptr);
	vkFreeMemory(Sample.logicalDevice, stage_memory, nullptr);
}
/*
  *创建uniform buffer
 */
void  init_uniform_buffer(CSVKSample  &Sample)
{
	create_target_vertex_buffer(Sample, sizeof(float) * 16 * 3,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),Sample.uniformBuffer,Sample.uniformMemory);
}
/*
  *描述符集合布局
 */
void  init_descriptor_set_layout(CSVKSample &Sample)
{
	/*描述符布局绑定*/
	VkDescriptorSetLayoutBinding  desc_set_layout_bind[2] = {};
	desc_set_layout_bind[0].binding = 0;
	desc_set_layout_bind[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	desc_set_layout_bind[0].descriptorCount = 1;
	desc_set_layout_bind[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	desc_set_layout_bind[1].binding = 1;
	desc_set_layout_bind[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	desc_set_layout_bind[1].descriptorCount = 1;
	desc_set_layout_bind[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo  desc_set_layout_info = {};
	desc_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	desc_set_layout_info.bindingCount = 2;
	desc_set_layout_info.pBindings = desc_set_layout_bind;

	VkResult result = vkCreateDescriptorSetLayout(Sample.logicalDevice, &desc_set_layout_info, nullptr, &Sample.descriptorSetLayout);
	CSVK_Assert(result == VK_SUCCESS,"Create Descriptor Set Layout Failed.");
}
/*
  *创建纹理
 */
void  create_texture(CSVKSample &Sample,VkImage &image,VkDeviceMemory &memory,VkMemoryPropertyFlags  propertys,VkFormat  image_format,uint32_t  width,uint32_t height,VkImageTiling  image_tiling,VkImageUsageFlags  image_usage)
{
	VkImageCreateInfo  image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = image_format;
	image_create_info.extent.width = width;
	image_create_info.extent.height = height;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = image_tiling;
	image_create_info.usage = image_usage;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.queueFamilyIndexCount = 0;
	image_create_info.pQueueFamilyIndices = nullptr;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkResult result = vkCreateImage(Sample.logicalDevice, &image_create_info, nullptr, &image);
	CSVK_Assert(result == VK_SUCCESS,"Create Image Failed.");

	VkMemoryRequirements  memory_require;
	vkGetImageMemoryRequirements(Sample.logicalDevice, image, &memory_require);

	VkMemoryAllocateInfo  memory_alloc_info = {};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memory_require.size;
	memory_alloc_info.memoryTypeIndex = check_memory_type_index(Sample, memory_require.memoryTypeBits,propertys);

	result = vkAllocateMemory(Sample.logicalDevice, &memory_alloc_info, nullptr, &memory);
	CSVK_Assert(result == VK_SUCCESS,"Allocate Memory Failed.");

	vkBindImageMemory(Sample.logicalDevice, image, memory, 0);
}
/*
  *变换纹理布局
 */
void  transform_texture_layout(CSVKSample &Sample,VkImage image,VkImageLayout old_layout,VkImageLayout new_layout)
{
	/*create command buffer*/
	VkCommandBufferAllocateInfo  cmd_buffer_alloc_info = {};
	cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_buffer_alloc_info.commandPool = Sample.commandPool;
	cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd_buffer_alloc_info.commandBufferCount = 1;

	VkCommandBuffer  cmd_buffer = nullptr;
	VkResult result = vkAllocateCommandBuffers(Sample.logicalDevice, &cmd_buffer_alloc_info, &cmd_buffer);
	CSVK_Assert(result == VK_SUCCESS,"Create Command Buffer Failed.");

	VkCommandBufferBeginInfo  cmd_buffer_begin = {};
	cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buffer_begin.pInheritanceInfo = nullptr;
	cmd_buffer_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin);

	/*内存屏障*/
	VkImageMemoryBarrier   image_memory_barrier = {};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.oldLayout = old_layout;
	image_memory_barrier.newLayout = new_layout;
	image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_memory_barrier.image = image;
	image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_memory_barrier.subresourceRange.baseArrayLayer = 0;
	image_memory_barrier.subresourceRange.baseMipLevel = 0;
	image_memory_barrier.subresourceRange.layerCount = 1;
	image_memory_barrier.subresourceRange.levelCount = 1;

	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		image_memory_barrier.subresourceRange.aspectMask |= (Sample.depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || Sample.depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)?VK_IMAGE_ASPECT_STENCIL_BIT : 0;
	}

	VkPipelineStageFlags   source_stage;
	VkPipelineStageFlags   dst_stage;
	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		image_memory_barrier.srcAccessMask = 0;
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		image_memory_barrier.srcAccessMask = 0;
		image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
		CSVK_Assert(false,"Unsupported Operation.");
	vkCmdPipelineBarrier(cmd_buffer, source_stage, dst_stage, 0, 0, nullptr, 0,0,1, &image_memory_barrier);

	vkEndCommandBuffer(cmd_buffer);
	VkSubmitInfo  submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buffer;
	vkQueueSubmit(Sample.graphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(Sample.graphicsQueue);

	vkFreeCommandBuffers(Sample.logicalDevice, Sample.commandPool, 1, &cmd_buffer);
}
/*
  *从缓冲区到image复制数据
 */
void  copy_buffer_to_image(CSVKSample  &Sample,VkBuffer buffer,VkImage image,uint32_t width,uint32_t height)
{
	/*命令缓冲区对象*/
	VkCommandBufferAllocateInfo  cmd_buffer_alloc = {};
	cmd_buffer_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_buffer_alloc.commandBufferCount = 1;
	cmd_buffer_alloc.commandPool = Sample.commandPool;
	cmd_buffer_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer  cmd_buffer = nullptr;
	VkResult result = vkAllocateCommandBuffers(Sample.logicalDevice, &cmd_buffer_alloc, &cmd_buffer);

	VkCommandBufferBeginInfo cmd_buffer_begin = {};
	cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buffer_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	result = vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin);
	CSVK_Assert(result == VK_SUCCESS,"Begin Command Buffer Failed.");

	VkBufferImageCopy  buffer_image_copy = {};
	buffer_image_copy.bufferOffset = 0;
	buffer_image_copy.bufferRowLength = 0;
	buffer_image_copy.bufferImageHeight = 0;
	buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	buffer_image_copy.imageSubresource.mipLevel = 0;
	buffer_image_copy.imageSubresource.baseArrayLayer = 0;
	buffer_image_copy.imageSubresource.layerCount = 1;
	buffer_image_copy.imageOffset.x = 0;
	buffer_image_copy.imageOffset.y = 0;
	buffer_image_copy.imageOffset.z = 0;
	buffer_image_copy.imageExtent.width = width;
	buffer_image_copy.imageExtent.height = height;
	buffer_image_copy.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(cmd_buffer,buffer,image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&buffer_image_copy);

	vkEndCommandBuffer(cmd_buffer);
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buffer;
	vkQueueSubmit(Sample.graphicsQueue, 1, &submit_info,VK_NULL_HANDLE);
	vkQueueWaitIdle(Sample.graphicsQueue);

	vkFreeCommandBuffers(Sample.logicalDevice, Sample.commandPool, 1, &cmd_buffer);
}
/*
  *创建纹理图像
 */
void  init_texture_image(CSVKSample  &Sample)
{
	int  texel_width = 0, texel_height = 0, texel_channels=0;
	stbi_uc  *pixels = stbi_load("textures/texture.jpg", &texel_width, &texel_height, &texel_channels,STBI_rgb_alpha);
	uint32_t  pixel_size = texel_width * texel_height * 4;
	CSVK_Assert(pixels != nullptr,"Load textures failed.");

	VkBuffer  stage_buffer = VK_NULL_HANDLE;
	VkDeviceMemory  stage_memory = VK_NULL_HANDLE;
	create_target_vertex_buffer(Sample, pixel_size,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),stage_buffer,stage_memory);

	void  *memory_data = nullptr;
	VkResult result = vkMapMemory(Sample.logicalDevice, stage_memory,0,pixel_size,0,&memory_data);
	CSVK_Assert(result==VK_SUCCESS,"Map Memory Buffer Failed.");
	memcpy(memory_data,pixels,pixel_size);
	vkUnmapMemory(Sample.logicalDevice, stage_memory);
	stbi_image_free(pixels);

	create_texture(Sample,Sample.texture,Sample.textureMemory,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,VK_FORMAT_R8G8B8A8_UNORM,texel_width,texel_height,VK_IMAGE_TILING_OPTIMAL,VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);
	transform_texture_layout(Sample, Sample.texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copy_buffer_to_image(Sample,stage_buffer,Sample.texture,texel_width,texel_height);
	transform_texture_layout(Sample, Sample.texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(Sample.logicalDevice,stage_buffer,nullptr);
	vkFreeMemory(Sample.logicalDevice, stage_memory, nullptr);
}
/*
  *创建image-view
 */
void  init_texture_image_view(CSVKSample &Sample)
{
	VkImageViewCreateInfo  image_view_create = {};
	image_view_create.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create.image = Sample.texture;
	image_view_create.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create.format = VK_FORMAT_R8G8B8A8_UNORM;
	image_view_create.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_create.subresourceRange.baseMipLevel = 0;
	image_view_create.subresourceRange.levelCount = 1;
	image_view_create.subresourceRange.baseArrayLayer = 0;
	image_view_create.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(Sample.logicalDevice, &image_view_create, nullptr, &Sample.textureView);
	CSVK_Assert(result == VK_SUCCESS,"Create Image View Failed.");
}
/*
  *创建纹理采样器
 */
void  init_texture_view_sampler(CSVKSample &Sample)
{
	VkSamplerCreateInfo  sampler_create = {};
	sampler_create.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create.magFilter = VK_FILTER_LINEAR;
	sampler_create.minFilter = VK_FILTER_LINEAR;
	sampler_create.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_create.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_create.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_create.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_create.mipLodBias = 0;
	sampler_create.anisotropyEnable = VK_FALSE;
	sampler_create.maxAnisotropy = 16;
	sampler_create.compareEnable = VK_FALSE;
	sampler_create.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_create.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_create.unnormalizedCoordinates = VK_FALSE;

	VkResult result = vkCreateSampler(Sample.logicalDevice, &sampler_create, nullptr, &Sample.sampler);
	CSVK_Assert(result == VK_SUCCESS,"Create Sampler Failed.");
}
/*
  *检索深度纹理的格式
 */
void  check_depth_image_format(CSVKSample &Sample)
{
	VkFormat  depth_formats[3] = {VK_FORMAT_D32_SFLOAT,VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D24_UNORM_S8_UINT};
	Sample.depthFormat = VK_FORMAT_UNDEFINED;
	for (int k = 0; k < 3; ++k)
	{
		VkFormatProperties  format_property;
		vkGetPhysicalDeviceFormatProperties(Sample.physicalDevice, depth_formats[k], &format_property);
		/*检索Optimal Tiling*/
		if (format_property.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			Sample.depthFormat = depth_formats[k];
			break;
		}
	}
	CSVK_Assert(Sample.depthFormat != VK_FORMAT_UNDEFINED,"Could not check fitable depth format.");
}
/*
  *创建深度纹理
 */
void  init_depth_image_resource(CSVKSample &Sample)
{
	check_depth_image_format(Sample);

	VkImageCreateInfo  image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = Sample.depthFormat;
	image_create_info.extent.width = Sample.swapChainExtent.width;
	image_create_info.extent.height = Sample.swapChainExtent.height;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult result = vkCreateImage(Sample.logicalDevice, &image_create_info, nullptr, &Sample.depthImage);
	CSVK_Assert(result == VK_SUCCESS,"Could not create depth image.");

	VkMemoryRequirements  memory_require;
	vkGetImageMemoryRequirements(Sample.logicalDevice, Sample.depthImage, &memory_require);

	VkMemoryAllocateInfo  memory_alloc = {};
	memory_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc.allocationSize = memory_require.size;
	memory_alloc.memoryTypeIndex = check_memory_type_index(Sample, memory_require.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	result = vkAllocateMemory(Sample.logicalDevice, &memory_alloc, nullptr, &Sample.depthMemory);
	CSVK_Assert(result == VK_SUCCESS,"Allocate depth memory failed.");

	result = vkBindImageMemory(Sample.logicalDevice, Sample.depthImage, Sample.depthMemory, 0);
	CSVK_Assert(result == VK_SUCCESS,"Bind depth image memory failed.");

	VkImageViewCreateInfo  image_view_create = {};
	image_view_create.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create.image = Sample.depthImage;
	image_view_create.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create.format = Sample.depthFormat;
	image_view_create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	image_view_create.subresourceRange.baseArrayLayer = 0;
	image_view_create.subresourceRange.layerCount = 1;
	image_view_create.subresourceRange.baseMipLevel = 0;
	image_view_create.subresourceRange.levelCount = 1;

	result = vkCreateImageView(Sample.logicalDevice, &image_view_create, nullptr, &Sample.depthImageView);
	CSVK_Assert(result == VK_SUCCESS,"Could not create depth image view.");

	/*变换布局*/
	transform_texture_layout(Sample, Sample.depthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}
/*
*创建描述符对象池
*/
void  init_descriptor_pool(CSVKSample  &Sample)
{
	VkDescriptorPoolSize   pool_size[2] = {};
	pool_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size[0].descriptorCount = 1;

	pool_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_size[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo  desc_pool_info = {};
	desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	desc_pool_info.poolSizeCount = 2;
	desc_pool_info.pPoolSizes = pool_size;
	desc_pool_info.maxSets = 1;

	VkResult  result = vkCreateDescriptorPool(Sample.logicalDevice, &desc_pool_info, nullptr, &Sample.descriptorPool);
	CSVK_Assert(result == VK_SUCCESS, "Create Descriptor Pool Failed.");
}
/*
  *创建描述符集合
 */
void  init_descriptor_set(CSVKSample &Sample)
{
	VkDescriptorSetAllocateInfo  desc_set_alloc_info = {};
	desc_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	desc_set_alloc_info.descriptorPool = Sample.descriptorPool;
	desc_set_alloc_info.descriptorSetCount = 1;
	desc_set_alloc_info.pSetLayouts = &Sample.descriptorSetLayout;
	VkResult  result = vkAllocateDescriptorSets(Sample.logicalDevice, &desc_set_alloc_info, &Sample.descriptorSet);
	CSVK_Assert(result == VK_SUCCESS,"Alloc Descriptor Sets Failed.");

	VkDescriptorBufferInfo  desc_buffer_info = {};
	desc_buffer_info.buffer = Sample.uniformBuffer;
	desc_buffer_info.offset = 0;
	desc_buffer_info.range = sizeof(float)*16*3;

	VkDescriptorImageInfo  desc_image_info = {};
	desc_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	desc_image_info.imageView = Sample.textureView;
	desc_image_info.sampler = Sample.sampler;

	VkWriteDescriptorSet	write_desc_set[2] = {};
	write_desc_set[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_desc_set[0].dstSet = Sample.descriptorSet;
	write_desc_set[0].dstBinding = 0;
	write_desc_set[0].dstArrayElement = 0;
	write_desc_set[0].descriptorCount = 1;
	write_desc_set[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_desc_set[0].pBufferInfo = &desc_buffer_info;

	write_desc_set[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_desc_set[1].dstSet = Sample.descriptorSet;
	write_desc_set[1].dstBinding = 1;
	write_desc_set[1].dstArrayElement = 0;
	write_desc_set[1].descriptorCount = 1;
	write_desc_set[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_desc_set[1].pImageInfo = &desc_image_info;

	vkUpdateDescriptorSets(Sample.logicalDevice, 2, write_desc_set, 0, nullptr);
}
/*
  *创建命令缓冲区对象
 */
void  init_command_buffer(CSVKSample  &Sample)
{
	Sample.commandBuffers.resize(Sample.swapChainImageCount);
	VkCommandBufferAllocateInfo  cmd_buffer_alloc_info = {};
	cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_buffer_alloc_info.commandPool = Sample.commandPool;
	cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd_buffer_alloc_info.commandBufferCount = Sample.swapChainImageCount;
	VkResult result = vkAllocateCommandBuffers(Sample.logicalDevice, &cmd_buffer_alloc_info, Sample.commandBuffers.data());
	CSVK_Assert(result == VK_SUCCESS,"Alloc Command Buffer Failed.");

	VkCommandBuffer  *cmd_buffers = Sample.commandBuffers.data();
	for (int k = 0; k < Sample.swapChainImageCount; ++k)
	{
		VkCommandBufferBeginInfo  cmd_buffer_begin = {};
		cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buffer_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		vkBeginCommandBuffer(cmd_buffers[k], &cmd_buffer_begin);

		VkClearValue  clear_value[2];
		clear_value[0].color = { 0.0f,0.0f,0.0f,1.0f };
		clear_value[1].depthStencil = {1.0,0};

		VkRenderPassBeginInfo  renderpass_begin = {};
		renderpass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_begin.renderPass = Sample.renderPass;
		renderpass_begin.framebuffer = Sample.swapChainFramebuffers[k];
		renderpass_begin.renderArea.offset.x = 0;
		renderpass_begin.renderArea.offset.y = 0;
		renderpass_begin.renderArea.extent = Sample.swapChainExtent;
		renderpass_begin.clearValueCount = 2;
		renderpass_begin.pClearValues = clear_value;

		vkCmdBeginRenderPass(cmd_buffers[k], &renderpass_begin, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(cmd_buffers[k], VK_PIPELINE_BIND_POINT_GRAPHICS, Sample.graphicsPipeline);

		VkDeviceSize  offsets = 0;
		vkCmdBindVertexBuffers(cmd_buffers[k], 0, 1, &Sample.vertexBuffer, &offsets);
		vkCmdBindIndexBuffer(cmd_buffers[k], Sample.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(cmd_buffers[k], VK_PIPELINE_BIND_POINT_GRAPHICS, Sample.pipelineLayout,0,1,&Sample.descriptorSet,0,nullptr);

		//vkCmdDraw(cmd_buffers[k], 3, 1, 0, 0);
		vkCmdDrawIndexed(cmd_buffers[k],12,1,0,0,0);

		vkCmdEndRenderPass(cmd_buffers[k]);
		result = vkEndCommandBuffer(cmd_buffers[k]);
		CSVK_Assert(result == VK_SUCCESS,"vkEndCommandBuffer Failed.");
	}
}
/*
  *创建信号量
 */
void  init_create_semaphore(CSVKSample  &Sample)
{
	VkSemaphoreCreateInfo     semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult  result = vkCreateSemaphore(Sample.logicalDevice, &semaphore_info, nullptr, &Sample.imageSemaphore);
	CSVK_Assert(result == VK_SUCCESS ,"Create Semaphore_1 Failed.");

	result = vkCreateSemaphore(Sample.logicalDevice, &semaphore_info, nullptr, &Sample.renderSemaphore);
	CSVK_Assert(result == VK_SUCCESS,"Create Semaphore_2 Failed.");
}
/*
  *每帧调用函数
 */
void  on_callback_perframe(CSVKSample  &Sample)
{
	uint32_t  image_index = -1;
	vkAcquireNextImageKHR(Sample.logicalDevice, Sample.swapChainKHR, -1, Sample.imageSemaphore, VK_NULL_HANDLE, &image_index);

	/*提交信息*/
	VkPipelineStageFlags  pipeline_state= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo  submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &Sample.imageSemaphore;
	submit_info.pWaitDstStageMask = &pipeline_state;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = Sample.commandBuffers.data() + image_index;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &Sample.renderSemaphore;

	VkResult result = vkQueueSubmit(Sample.graphicsQueue, 1, &submit_info, VK_NULL_HANDLE);

	VkPresentInfoKHR  present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &Sample.renderSemaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &Sample.swapChainKHR;
	present_info.pImageIndices = &image_index;

	vkQueuePresentKHR(Sample.presentQueue, &present_info);
	vkQueueWaitIdle(Sample.presentQueue);
}
/*
  *更新uniform缓冲区对象
 */
void  update_uniform_buffer(CSVKSample &Sample)
{
	void  *memory_data = nullptr;
	float   matrix_array[16 * 3] = {
		1,0,0,0,   0,1,0,0,   0,0,1,0, 0,0,0,1,
		1,0,0,0,   0,1,0,0,   0,0,1,0, 0,0,0,1,
		1,0,0,0,   0,1,0,0,   0,0,1,0, 0,0,0,1,
	};
	mat4x4  proj;
	mat4x4::createOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, proj);
	memcpy(matrix_array + 16*2,proj.pointer(),sizeof(float)*16);

	VkResult result = vkMapMemory(Sample.logicalDevice, Sample.uniformMemory, 0, sizeof(matrix_array), 0, &memory_data);
	CSVK_Assert(result == VK_SUCCESS,"Map Uniform Memory Failed.");
	memcpy(memory_data, matrix_array,sizeof(matrix_array));
	vkUnmapMemory(Sample.logicalDevice, Sample.uniformMemory);
}
/*
  *销毁Vulkan实例
 */
void  destroy_vulkan_sample(CSVKSample  &Sample)
{
	vkDestroySemaphore(Sample.logicalDevice, Sample.renderSemaphore, nullptr);
	vkDestroySemaphore(Sample.logicalDevice, Sample.imageSemaphore, nullptr);

	vkDestroyCommandPool(Sample.logicalDevice, Sample.commandPool, nullptr);

	for (int k = 0; k < Sample.swapChainImageCount; ++k)
		vkDestroyFramebuffer(Sample.logicalDevice, Sample.swapChainFramebuffers[k], nullptr);
	vkDestroyPipeline(Sample.logicalDevice, Sample.graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(Sample.logicalDevice, Sample.pipelineLayout, nullptr);
	vkDestroyRenderPass(Sample.logicalDevice, Sample.renderPass, nullptr);

	for (int k = 0; k < Sample.swapChainImageCount; ++k)
		vkDestroyImageView(Sample.logicalDevice, Sample.swapChainImageViews[k], nullptr);
	vkDestroySwapchainKHR(Sample.logicalDevice, Sample.swapChainKHR, nullptr);

	vkDestroyBuffer(Sample.logicalDevice, Sample.vertexBuffer, nullptr);
	vkFreeMemory(Sample.logicalDevice, Sample.vertexMemory, nullptr);

	vkDestroyBuffer(Sample.logicalDevice, Sample.indexBuffer, nullptr);
	vkFreeMemory(Sample.logicalDevice, Sample.indexMemory, nullptr);

	vkDestroyBuffer(Sample.logicalDevice,Sample.uniformBuffer,nullptr);
	vkFreeMemory(Sample.logicalDevice, Sample.uniformMemory, nullptr);

	vkDestroyImage(Sample.logicalDevice, Sample.texture, nullptr);
	vkDestroyImageView(Sample.logicalDevice, Sample.textureView, nullptr);
	vkFreeMemory(Sample.logicalDevice, Sample.textureMemory, nullptr);
	vkDestroySampler(Sample.logicalDevice, Sample.sampler, nullptr);

	vkDestroyImage(Sample.logicalDevice, Sample.depthImage, nullptr);
	vkDestroyImageView(Sample.logicalDevice, Sample.depthImageView, nullptr);
	vkFreeMemory(Sample.logicalDevice, Sample.depthMemory, nullptr);

	vkDestroyDescriptorPool(Sample.logicalDevice, Sample.descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(Sample.logicalDevice, Sample.descriptorSetLayout, nullptr);

	vkDestroyDevice(Sample.logicalDevice, nullptr);
	/*销毁错误处理回调函数*/
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(Sample.csvkInstance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) 
		func(Sample.csvkInstance, Sample.debugCallback, nullptr);
	vkDestroySurfaceKHR(Sample.csvkInstance, Sample.csvkSurfaceKHR, nullptr);
	vkDestroyInstance(Sample.csvkInstance, nullptr);
}

int  main(int argc, char *argv[])
{
	CSVKSample    CSSample = {};

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	CSSample.window = glfwCreateWindow(s_window_width, s_window_height, "Vulkan_Triangle", nullptr, nullptr);

	/*检测是否有相关的验证层支持*/
	uint32_t    layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties>   layer_property(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, layer_property.data());
	//check
	bool   found_layer = false;
	for (auto it = layer_property.begin(); it != layer_property.end(); ++it)
	{
		if (!strcmp(s_validate_layer_name, it->layerName))
		{
			found_layer = true;
			break;
		}
	}
	CSVK_Assert(found_layer,"do not find validate layer.");
	/*create Vulkan-Instance*/
	init_vulkan_instance(CSSample);

	/*register debebug-callback*/
	init_debug_callback(CSSample);

	/*create surface*/
	VkResult result = glfwCreateWindowSurface(CSSample.csvkInstance, CSSample.window, nullptr, &CSSample.csvkSurfaceKHR);
	CSVK_Assert(result == VK_SUCCESS,"glfwCreateWindowSurface error.");

	/*选择合适的物理设备*/
	init_select_physical_device(CSSample);

	/*创建逻辑设备*/
	init_logic_device(CSSample);

	/*创建交换链*/
	init_create_swapchain(CSSample);

	/*创建交换链中image-view*/
	init_swapchain_imageview(CSSample);

	/*创建命令缓冲池*/
	init_command_pool(CSSample);

	/*创建深度缓冲区对象*/
	init_depth_image_resource(CSSample);

	/*创建渲染通道*/
	init_renderpass(CSSample);

	/*创建描述符集合布局*/
	init_descriptor_set_layout(CSSample);

	/*创建图形管线*/
	init_create_graphics_pipeline(CSSample);

	/*创建帧缓冲区对象*/
	init_framebuffer(CSSample);

	/*创建顶点缓冲区对象*/
	init_vertex_buffer(CSSample);

	/*创建索引缓冲区对象*/
	init_index_buffer(CSSample);

	/*创建统一变量缓冲区对象*/
	init_uniform_buffer(CSSample);

	/*创建image*/
	init_texture_image(CSSample);

	/*image-view*/
	init_texture_image_view(CSSample);

	/*sampler*/
	init_texture_view_sampler(CSSample);

	/*创建描述符对象池*/
	init_descriptor_pool(CSSample);

	/*创建描述符集合*/
	init_descriptor_set(CSSample);

	/*创建命令缓冲区对象*/
	init_command_buffer(CSSample);

	/*创建信号灯*/
	init_create_semaphore(CSSample);

	while (!glfwWindowShouldClose(CSSample.window))
	{
		glfwPollEvents();
		update_uniform_buffer(CSSample);
		on_callback_perframe(CSSample);
	}

	vkDeviceWaitIdle(CSSample.logicalDevice);
	destroy_vulkan_sample(CSSample);
	glfwTerminate();
	return 0;
}