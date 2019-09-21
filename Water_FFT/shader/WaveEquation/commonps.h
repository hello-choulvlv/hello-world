;//����TexelCoordInteger��������������=��ͨ��������*Ҫ��������ķֱ��ʣ��������Color�����������ɫ����������õ�r1��r8�ļĴ���������a,b��������8���У�������÷ŵ�����ʼִ�У����Ӱ����������
;//����OneTap = 1 / Ҫ������2D��ͼ�ֱ��ʣ�TexSampler�ǲ����Ĵ���s#
#define BILINEAR_TEXLD(TexelCoordInteger, Color, TexSampler)\
					 	frc Color, TexelCoordInteger\
						sub r1, TexelCoordInteger, Color\
						\
						sub Color.z, ONE, Color.x\
						sub Color.w, ONE, Color.y\
						\
						add r4, r1, ONE\
						\
						mov r2.x, r4.x\
						mov r2.y, r1.y\
						mov r2.zw, ONE\
						\
						mov r3.x, r1.x\
						mov r3.y, r4.y\
						mov r3.zw, ONE\
						\
						mul r5, Color.z, Color.w\
						mul r6, Color.x, Color.w\
						mul r7, Color.z, Color.y\
						mul r8, Color.x, Color.y\
						\
						add r1, r1, HALF\
						mul r1, r1, cOneTap\
						texld r1, r1, TexSampler\
						\
						add r2, r2, HALF\
						mul r2, r2, cOneTap\
						texld r2, r2, TexSampler\
						\
						add r3, r3, HALF\
						mul r3, r3, cOneTap\
						texld r3, r3, TexSampler\
						\
						add r4, r4, HALF\
						mul r4, r4, cOneTap\
						texld r4, r4, TexSampler\
						\
						mul Color, r1, r5\
						mad Color, r2, r6, Color\
						mad Color, r3, r7, Color\
						mad Color, r4, r8, Color\
						
						
						
						
						
						
						
						
						
						
						
						
						
;�ϳ������ӳ��������ú�Զ�������������ӳ���
;����FaceIndex��TexelCoordOn2D�ϳ�Cube���꣬����TexelCoordOnCubeMap��
;����Ҫ���Խ�磬�����PCF�Ļ����ܱ�Ե���ػ���ת����һ����ȥ�����ﲻ���жϣ���ǿ���޶��߽磬����ģ��
;����ǰ���Ѿ�ƫ�ƹ������㣬�ȼ���Ƿ���磬��С��0�����ڵ���1�������Clamp
;��ӳ��CoordOn2D���������꣬��0��1��-0.5��0.5
#define COMBINE_ADDRESSVECTOR(FaceIndex, TexelCoordOn2D, TexelCoordOnCubeMap) 	\
	max TexelCoordOn2D.xy, TexelCoordOn2D.xy, ZERO\
	min TexelCoordOn2D.xy, TexelCoordOn2D.xy, ONE\
	sub TexelCoordOn2D.xy, TexelCoordOn2D.xy, HALF\
	if_eq FaceIndex.r, ZERO\
		mov TexelCoordOnCubeMap.x, HALF\
		mov TexelCoordOnCubeMap.y, -TexelCoordOn2D.y\
		mov TexelCoordOnCubeMap.z, -TexelCoordOn2D.x\
	endif\
		\
	if_eq FaceIndex.r, ONE\
		mov TexelCoordOnCubeMap.x, OPPO_HALF\
		mov TexelCoordOnCubeMap.y, -TexelCoordOn2D.y\
		mov TexelCoordOnCubeMap.z, TexelCoordOn2D.x\
	endif\
	\
	if_eq FaceIndex.r, TWO\
		mov TexelCoordOnCubeMap.x, TexelCoordOn2D.x\
		mov TexelCoordOnCubeMap.y, HALF\
		mov TexelCoordOnCubeMap.z, TexelCoordOn2D.y\
	endif\
	\
	if_eq FaceIndex.r, THREE\
		mov TexelCoordOnCubeMap.x, TexelCoordOn2D.x\
		mov TexelCoordOnCubeMap.y, OPPO_HALF\
		mov TexelCoordOnCubeMap.z, -TexelCoordOn2D.y\
	endif\
	\
	if_eq FaceIndex.r, FOUR\
		mov TexelCoordOnCubeMap.x, TexelCoordOn2D.x\
		mov TexelCoordOnCubeMap.y, -TexelCoordOn2D.y\
		mov TexelCoordOnCubeMap.z, HALF\
	endif\
	\
	if_eq FaceIndex.r, FIVE\
		mov TexelCoordOnCubeMap.x, -TexelCoordOn2D.x\
		mov TexelCoordOnCubeMap.y, -TexelCoordOn2D.y\
		mov TexelCoordOnCubeMap.z, OPPO_HALF\
	endif\
	
	
	
	
	
	
	
	
	
	
	
	
;// �Ѵ������ж�ȡ�������ٶȳ��ָ���������ʽ��ֻ��xy��Ч��zw���䣩
;// ����Velocity�����Velocity���м���ʱʹ�õļĴ���ΪTempUse
#define RESTORE_VELOCITY(Velocity, TempUse) \
						sub TempUse.x, Velocity.z, ONE\
						sub TempUse.y, Velocity.w, ONE\
						mul Velocity.x, Velocity.x, TempUse.x\
						mul Velocity.y, Velocity.y, TempUse.y\


;// �ѵ�ǰ����õ��ٶȳ�����xy������Ч������ɾ���ֵ+���ŵ���ʽ���Ա��������
;// ����Velocity�����Velocity���м���ʱʹ�õļĴ���ΪTempUse
#define COMBINE_VELOCITY(Velocity, TempUse) \
						cmp TempUse.z, Velocity.x, TWO, ZERO\
						cmp TempUse.w, Velocity.y, TWO, ZERO\
						abs TempUse.x, Velocity.x\
						abs TempUse.y, Velocity.y\
						mov Velocity, TempUse \
						
						
						
						
						
						
;//******************��ѧ���㺯��********************/
						
;//������Ϊ�˷�ֹrsq��rcp�����ε�������ר���ṩ���ӿ�
;//Src����ָ���������Ҳ���Ϊ������Dest��Ҫ�󣬽����д��Destָ����ÿ������
#define SQRT(Dest, Src, Temp)\
						rsq Temp.x, Src\
						rcp Dest, Temp.x\
						
;//���������ȣ������д��Lengthָ����ÿ������
#define GETVECTOR_LENGTH(Length, Vector, Temp1, Temp2)\
						dp3 Temp1, Vector, Vector\
						SQRT(Length, Temp1.x, Temp2)\
						

						
						
;//******************����SM2.0�еĶ�̬�Ƚϸ�ֵ���൱��SM2.a/3.0�е�if_�ټ�һ����ֵ���********************/

;//����BeJudged������ָ��r#�ĵ�������������÷���ifָ����ͬ��
;//�жϣ����BeJudged����0����ô����Value����Constant���൱��if(BeJudged == 0) Value = Constant;
;//�����Constantֻ�Ǳ�ʾ��ֵ��Դ���ѣ�������˵�ǵó����Ĵ���
;//Epsilon��һ����С������������0.00000001����TempUse����ʱʹ�õļĴ���

;// mul TempUse.x, TempUse.x, TempUse.y   ���Ϊ0���Ǿͱ�ʾBeJudged�ֱ���ͼ�Epsilon���������෴��
;// cmp Value, TempUse.x, Value, Constant  ��������෴���ʹ���BeJudgedΪ0����Value=Constant�����������ͬ����˵��BeJudged��Ϊ0������Value��ԭֵ����
#define IF_EQUALZERO(BeJudged, Value, Constant, Epsilon, TempUse)\
					 	sub TempUse.x, BeJudged, Epsilon\
					 	add TempUse.y, BeJudged, Epsilon\
					 	mul TempUse.x, TempUse.x, TempUse.y\
					 	cmp Value, TempUse.x, Value, Constant\

;//if(BeJuged != 0)	Value = Constant;	 	
#define IF_NOTEQUALZERO(BeJudged, Value, Constant, Epsilon, TempUse)\
					 	sub TempUse.x, BeJudged, Epsilon\
					 	add TempUse.y, BeJudged, Epsilon\
					 	mul TempUse.x, TempUse.x, TempUse.y\
					 	cmp Value, TempUse.x, Constant, Value\

;//if(BeJuged = JudgeValue)	Value = Constant������Ľ��ͺ��÷�ͬ��
;//�����BeJudged/JudgeValue����ָ��r#�ĵ�������
#define IF_EQ(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse1, TempUse2)\
					 	sub TempUse1.x, BeJudged, JudgeValue\
					 	IF_EQUALZERO(TempUse1.x, Value, Constant, Epsilon, TempUse2)\

;//if(BeJuged != JudgeValue)	Value = Constant;
;//�����BeJudged/JudgeValue����ָ��r#�ĵ�������
#define IF_NE(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse1, TempUse2)\
					 	sub TempUse1.x, BeJudged, JudgeValue\
					 	IF_NOTEQUALZERO(TempUse1.x, Value, Constant, Epsilon, TempUse2)\


					 	
;//if(BeJuged >= JudgeValue)	Value = Constant������Ľ��ͺ��÷�ͬ��
;//�����BeJudged/JudgeValue����ָ��r#�ĵ�������
;// if(BeJudged - JudgeValue >= 0 ) Value = Constant
#define IF_GE(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse)\
					 	sub TempUse.x, BeJudged, JudgeValue\
					 	cmp Value, TempUse.x, Constant, Value\
					 	
				 	
;//if(BeJuged > JudgeValue)	Value = Constant������Ľ��ͺ��÷�ͬ��
;//�����BeJudged/JudgeValue����ָ��r#�ĵ�������
;// if(JudgeValue - BeJuged < 0 ) Value = Constant
#define IF_GT(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse)\
					 	sub TempUse.x, JudgeValue, BeJudged\
					 	cmp Value, TempUse.x, Value, Constant\


;//if(BeJuged <= JudgeValue)	Value = Constant������Ľ��ͺ��÷�ͬ��
;//�����BeJudged/JudgeValue����ָ��r#�ĵ�������
;// if(JudgeValue - BeJudged >= 0 ) Value = Constant
#define IF_LE(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse)\
					 	sub TempUse.x, JudgeValue, BeJudged\
					 	cmp Value, TempUse.x, Constant, Value\


;//if(BeJuged < JudgeValue)	Value = Constant������Ľ��ͺ��÷�ͬ��
;//�����BeJudged/JudgeValue����ָ��r#�ĵ�������
;// if(BeJudged - JudgeValue < 0 ) Value = Constant
#define IF_LT(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse)\
					 	sub TempUse.x, BeJudged, JudgeValue\
					 	cmp Value, TempUse.x, Value, Constant\

					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 							
;// c0		Half Tapƫ��������ʽ��0.5/Width, 0.5/Height
;// c1		One Tapƫ��������ʽ��1/Width, 1/Height
;// c2		Texture�ֱ��� for Bilinear Filtering����ʽ��Width, Height

;// c3		����
;// c4		Delta Time��ÿ��������ͬ����λ��

						
#define cHalfTap c0
#define cOneTap c1
#define cTextureDimension c2

#define cDeltaTime c4

#define ZERO c3.x
#define HALF c3.y
#define ONE c3.z
#define TWO c3.w