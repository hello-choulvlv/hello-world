;//输入TexelCoordInteger（整数纹理坐标=普通纹理坐标*要采样纹理的分辨率），输出到Color（采样后的颜色），里面会用到r1～r8的寄存器，所以a,b不能在这8个中，而且最好放到程序开始执行，免得影响其他程序
;//其中OneTap = 1 / 要采样的2D贴图分辨率，TexSampler是采样寄存器s#
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
						
						
						
						
						
						
						
						
						
						
						
						
						
;合成向量子程序，这里用宏远快于真正调用子程序
;根据FaceIndex和TexelCoordOn2D合成Cube坐标，存在TexelCoordOnCubeMap中
;这里要检测越界，如果是PCF的话可能边缘像素会跳转到另一个面去，这里不好判断，就强制限定边界，即不模糊
;由于前面已经偏移过采样点，先检查是否出界，即小于0，大于等于1，出界就Clamp
;再映射CoordOn2D的纹理坐标，从0～1到-0.5～0.5
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
	
	
	
	
	
	
	
	
	
	
	
	
;// 把从纹理中读取出来的速度场恢复成正常形式（只有xy有效，zw不变）
;// 输入Velocity，输出Velocity，中间临时使用的寄存器为TempUse
#define RESTORE_VELOCITY(Velocity, TempUse) \
						sub TempUse.x, Velocity.z, ONE\
						sub TempUse.y, Velocity.w, ONE\
						mul Velocity.x, Velocity.x, TempUse.x\
						mul Velocity.y, Velocity.y, TempUse.y\


;// 把当前计算好的速度场（仅xy分量有效）打包成绝对值+符号的形式，以便存入纹理
;// 输入Velocity，输出Velocity，中间临时使用的寄存器为TempUse
#define COMBINE_VELOCITY(Velocity, TempUse) \
						cmp TempUse.z, Velocity.x, TWO, ZERO\
						cmp TempUse.w, Velocity.y, TWO, ZERO\
						abs TempUse.x, Velocity.x\
						abs TempUse.y, Velocity.y\
						mov Velocity, TempUse \
						
						
						
						
						
						
;//******************数学运算函数********************/
						
;//开方，为了防止rsq和rcp用至晕倒，这里专门提供个接口
;//Src必须指定单分量且不可为负数，Dest不要求，结果会写入Dest指定的每个分量
#define SQRT(Dest, Src, Temp)\
						rsq Temp.x, Src\
						rcp Dest, Temp.x\
						
;//求向量长度，结果会写入Length指定的每个分量
#define GETVECTOR_LENGTH(Length, Vector, Temp1, Temp2)\
						dp3 Temp1, Vector, Vector\
						SQRT(Length, Temp1.x, Temp2)\
						

						
						
;//******************用于SM2.0中的动态比较赋值，相当于SM2.a/3.0中的if_再加一条赋值语句********************/

;//输入BeJudged（必须指定r#的单个分量，这个用法和if指令相同）
;//判断，如果BeJudged等于0，那么就让Value等于Constant，相当于if(BeJudged == 0) Value = Constant;
;//这里的Constant只是表示赋值来源而已，并不是说非得常量寄存器
;//Epsilon是一个很小的正常数（如0.00000001），TempUse是临时使用的寄存器

;// mul TempUse.x, TempUse.x, TempUse.y   如果为0，那就表示BeJudged分别减和加Epsilon，符号是相反的
;// cmp Value, TempUse.x, Value, Constant  如果符号相反，就代表BeJudged为0，让Value=Constant；如果符号相同，就说明BeJudged不为0，保持Value的原值即可
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

;//if(BeJuged = JudgeValue)	Value = Constant，具体的解释和用法同上
;//这里的BeJudged/JudgeValue必须指定r#的单个分量
#define IF_EQ(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse1, TempUse2)\
					 	sub TempUse1.x, BeJudged, JudgeValue\
					 	IF_EQUALZERO(TempUse1.x, Value, Constant, Epsilon, TempUse2)\

;//if(BeJuged != JudgeValue)	Value = Constant;
;//这里的BeJudged/JudgeValue必须指定r#的单个分量
#define IF_NE(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse1, TempUse2)\
					 	sub TempUse1.x, BeJudged, JudgeValue\
					 	IF_NOTEQUALZERO(TempUse1.x, Value, Constant, Epsilon, TempUse2)\


					 	
;//if(BeJuged >= JudgeValue)	Value = Constant，具体的解释和用法同上
;//这里的BeJudged/JudgeValue必须指定r#的单个分量
;// if(BeJudged - JudgeValue >= 0 ) Value = Constant
#define IF_GE(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse)\
					 	sub TempUse.x, BeJudged, JudgeValue\
					 	cmp Value, TempUse.x, Constant, Value\
					 	
				 	
;//if(BeJuged > JudgeValue)	Value = Constant，具体的解释和用法同上
;//这里的BeJudged/JudgeValue必须指定r#的单个分量
;// if(JudgeValue - BeJuged < 0 ) Value = Constant
#define IF_GT(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse)\
					 	sub TempUse.x, JudgeValue, BeJudged\
					 	cmp Value, TempUse.x, Value, Constant\


;//if(BeJuged <= JudgeValue)	Value = Constant，具体的解释和用法同上
;//这里的BeJudged/JudgeValue必须指定r#的单个分量
;// if(JudgeValue - BeJudged >= 0 ) Value = Constant
#define IF_LE(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse)\
					 	sub TempUse.x, JudgeValue, BeJudged\
					 	cmp Value, TempUse.x, Constant, Value\


;//if(BeJuged < JudgeValue)	Value = Constant，具体的解释和用法同上
;//这里的BeJudged/JudgeValue必须指定r#的单个分量
;// if(BeJudged - JudgeValue < 0 ) Value = Constant
#define IF_LT(BeJudged, JudgeValue, Value, Constant, Epsilon, TempUse)\
					 	sub TempUse.x, BeJudged, JudgeValue\
					 	cmp Value, TempUse.x, Value, Constant\

					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 	
					 							
;// c0		Half Tap偏移量，格式：0.5/Width, 0.5/Height
;// c1		One Tap偏移量，格式：1/Width, 1/Height
;// c2		Texture分辨率 for Bilinear Filtering，格式：Width, Height

;// c3		常数
;// c4		Delta Time，每分量都相同，单位秒

						
#define cHalfTap c0
#define cOneTap c1
#define cTextureDimension c2

#define cDeltaTime c4

#define ZERO c3.x
#define HALF c3.y
#define ONE c3.z
#define TWO c3.w