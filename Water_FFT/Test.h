//���ļ���test.cpp��Ӧ��ͷ�ļ���test.cpp/h��Ӧһ��������Ӧ�ó���������洢�˵����������еĶ�������Ӳ�����ԡ���Ⱦ��������

//�������
#define SAMPLE_BUFFER_SIZE  3		//ÿ�εõ���DirectInput��������С��һ�㲻Ҫ��
#define MODULENUM 5					//��Ⱦ��������Ŀ
#define USE_MAXMRTNUM	1			// ʹ�ö��ٸ�MRT��1��ʾ��ʹ��


//ϵͳ����
//#define USE_DEBUG					//ʹ��DEBUGģʽ����ʹ��������Ҳ�����˳�������һ��MESSAGEBOX
#define USE_AVERAGEFPS				//ʹ��ƽ��֡�������Ǽ�ʱ֡��

#define USE_IMMEDIATE_KEYBOARD_INPUT		//ʹ�ü�ʱ���ݣ�DirectInput����������û�ж����ʱ�򣬾�ʹ��AsyncKey��ֻ֧�ּ���
#define USE_BUFFERED_KEYBOARD_INPUT			//ʹ�û������ݣ�DirectInput������������ͬʱ�����ʾ������룬��ͬʱ��ȡ��ʱ�ͻ������ݣ�����ŵ��κ�һ������

#define USE_IMMEDIATE_MOUSE_INPUT			//˵��ͬ�ϣ��������
#define USE_BUFFERED_MOUSE_INPUT
#define BASE_MOUSEMOVE_DISTANCE 30	// ����ƶ��Ļ�׼���루���أ���ÿ�ƶ���Щ���أ�������Ͱ���׼���ٶ��ƶ����������Խ����������ٶȾ�Խ���������ֵ�ر���Ϊ0ʱ�����൱�ں����������ˡ�


//������ĺ������ƶ�Ӧ��ͬ��Ч�ļ�������
#define USE_VERTEXSHADER
#define USE_PIXELSHADER

#define USE_VSVERSION "vs_3_0"		//��ͱ����VS�汾
#define USE_PSVERSION "ps_3_0"		//��ͱ����PS�汾��2.x����д��ps_2_a��ps_2_b��2_a�����ȼ�����2_b�������Ҫ2_b����ô2_aҲ����ͨ����⣬��֮����

#define VS2x_USE_MAXNUM_TEMP 10				// SM2.0x���еģ�����õ���SM2.0x����Ҫ��������õ�����ʱ�Ĵ����������Ƿ�ʹ���˷�֧������⼸������Ӳ���޷������㣬��ʹӲ��֧��2.0xҲҪ��ʾ����
#define PS2x_USE_MAXNUM_TEMP 32				// ֻ����USE_VS/PSVERSIONΪ2_x/a/bʱ����Ч���������ʾ�������õ�����ʱ�Ĵ����������������С����

//#define PS2x_USE_FLOWCONTROL				// ���ǵ��������е�PS2.xӲ������ʹ�÷�֧�����Ը���ֻ���õ���ʱ���


#define USE_FP16					
//#define USE_FP32	//���ǵ�CPU Lock������ģ�������FP32��

#define USE_VTF					// ��������ȡ����Displacement Mapping

#define USE_RENDERTOTEXTURE			//��Ⱦ������
#define USE_CUBEMAP				// ֻҪʹ�õ�CUBEMAP�Ķ�Ҫ��
//#define USE_CUBEMAPENVIRONMENT	// �ù̶�������Ⱦ������CUBEMAP�ģ�����򿪣�������Ⱦ�������

//#define USE_EMBM
//#define USE_DOTPRODUCT3			//���ع��գ���COLOR_OP = DOTPRODUCT3 , WITHOUT SHADER

//#define USE_DYNAMICTEXTURE		//��̬���͵�����һ������õ�