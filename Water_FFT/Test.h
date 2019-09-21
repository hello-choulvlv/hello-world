//此文件是test.cpp对应的头文件，test.cpp/h对应一个完整的应用程序，它里面存储了单个程序特有的东西，如硬件特性、渲染物体数等

//总体控制
#define SAMPLE_BUFFER_SIZE  3		//每次得到的DirectInput缓冲区大小，一般不要改
#define MODULENUM 5					//渲染物体总数目
#define USE_MAXMRTNUM	1			// 使用多少个MRT，1表示不使用


//系统控制
//#define USE_DEBUG					//使用DEBUG模式，即使遇到错误也不会退出，弹出一个MESSAGEBOX
#define USE_AVERAGEFPS				//使用平均帧数，并非即时帧数

#define USE_IMMEDIATE_KEYBOARD_INPUT		//使用即时数据（DirectInput），两个都没有定义的时候，就使用AsyncKey，只支持键盘
#define USE_BUFFERED_KEYBOARD_INPUT			//使用缓冲数据（DirectInput），两个可以同时定义表示混合输入，即同时读取即时和缓冲数据，避免放掉任何一个输入

#define USE_IMMEDIATE_MOUSE_INPUT			//说明同上，用于鼠标
#define USE_BUFFERED_MOUSE_INPUT
#define BASE_MOUSEMOVE_DISTANCE 30	// 鼠标移动的基准距离（像素），每移动这些像素，摄像机就按标准的速度移动，这个距离越大，摄像机的速度就越慢，当这个值特别大或为0时，就相当于忽略鼠标控制了。


//用下面的宏来控制对应不同特效的检测和屏蔽
#define USE_VERTEXSHADER
#define USE_PIXELSHADER

#define USE_VSVERSION "vs_3_0"		//最低必须的VS版本
#define USE_PSVERSION "ps_3_0"		//最低必须的PS版本，2.x必须写成ps_2_a或ps_2_b，2_a的优先级高于2_b，如果需要2_b，那么2_a也可以通过检测，反之不行

#define VS2x_USE_MAXNUM_TEMP 10				// SM2.0x独有的，如果用到了SM2.0x，就要给出最大用到的临时寄存器数，和是否使用了分支。如果这几个条件硬件无法不满足，即使硬件支持2.0x也要提示错误
#define PS2x_USE_MAXNUM_TEMP 32				// 只有在USE_VS/PSVERSION为2_x/a/b时才生效！这两项表示程序所用到的临时寄存器数，即程序的最小需求

//#define PS2x_USE_FLOWCONTROL				// 考虑到并非所有的PS2.x硬件都会使用分支，所以该项只在用到的时候打开


#define USE_FP16					
//#define USE_FP32	//考虑到CPU Lock，海浪模拟必须用FP32！

#define USE_VTF					// 顶点纹理取样，Displacement Mapping

#define USE_RENDERTOTEXTURE			//渲染到纹理
#define USE_CUBEMAP				// 只要使用到CUBEMAP的都要打开
//#define USE_CUBEMAPENVIRONMENT	// 用固定管线渲染环境到CUBEMAP的，必须打开，否则渲染结果出错

//#define USE_EMBM
//#define USE_DOTPRODUCT3			//像素光照，即COLOR_OP = DOTPRODUCT3 , WITHOUT SHADER

//#define USE_DYNAMICTEXTURE		//动态类型的纹理，一般很少用到