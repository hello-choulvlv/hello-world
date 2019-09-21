#include <time.h>
#include "myd3d.h"
#include "base.h"

char convstr[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/************************************************************************/
/*						String Operate Functions                        */
/************************************************************************/
void inttostr(unsigned long n)  /*比较小的数用unsigned,大数用unsigned long，不能搞混*/
{char temp[11]={48,48,48};
unsigned long cfs;
int num,bit=0,a,b;
float bitemp=1;            /*get value of "bit"*/
if(n==0) {convstr[0]='0';convstr[1]='\0';return;}
for(a=0;a<12&&bitemp>=1;a++){cfs=cf(a);bitemp=(float)n/cfs;bit++;}
bit--;
for(b=bit-1;b>=0;b--)
{cfs=cf(b);
temp[b]=(int)(n/cfs)+48;
num=(int)(n/cfs);
n=n-num*cfs;}
for(b=bit-1;b>=0;b--) convstr[b]=temp[bit-b-1];
convstr[bit]='\0';
}


void floattostr(int bitnum,double data)    //bitnum代表显示的小数位数，最多9位整数和9位小数
{double d=data,ddouble; unsigned long n=(unsigned long)data;

char temp[11]={48,48,48};
unsigned long cfs;
int num,bit=0,bit1=0,a,b;
float bitemp=1;            /*get value of "bit"*/

if(n==0) {convstr[0]='0';convstr[1]='\0';bit++;}    //如果整数部分为0则先只输出一个0，且当前位数指向第2个'\0'
else {
	for(a=0;a<12&&bitemp>=1;a++){cfs=cf(a);bitemp=(float)n/cfs;bit++;}
	bit--;
	for(b=bit-1;b>=0;b--)
	{cfs=cf(b);
	temp[b]=(int)(n/cfs)+48;
	num=(int)(n/cfs);
	n=n-num*cfs;}
	for(b=bit-1;b>=0;b--) convstr[b]=temp[bit-b-1];
}     //得到整数部分

convstr[bit]='.';        //先在当前位数位置（整数部分为0则位置为2，反之位置为整数部分的后一个）输出一个小数点
if(bitnum==0) {convstr[bit]='\0';return;}      //若不要求小数部分，则结束
ddouble=d-(unsigned long)data;      //得到小数部分
cfs=cf(bitnum);
ddouble*=cfs;
n=(unsigned long)ddouble;

if(n==0) convstr[bit]='\0';   //若小数部分为0，则字符串结束
else {bit++;
temp[0]=48;temp[1]=48;temp[2]=48;
bitemp=1;           /*get value of "bit"*/
for(a=0;a<12&&bitemp>=1;a++){cfs=cf(a);bitemp=(float)n/cfs;bit1++;}
bit1--;
for(b=bit1-1;b>=0;b--)
{cfs=cf(b);
temp[b]=(int)(n/cfs)+48;
num=(int)(n/cfs);
n=n-num*cfs;}
for(b=bit1-1;b>=0;b--) convstr[b+bit]=temp[bit1-b-1];
convstr[bit1+bit]='\0'; 
}     //得到小数部分
}


unsigned long strtoint(char *p)
{char *pp=p;unsigned long cfdata,result=0;
int i,j;
for(i=0;*pp!='\0';i++,pp++)if(*pp>57||*pp<48) return(result);  //i为位数
if(!i) return(0);
pp=p;
for(j=i;j>0;j--){cfdata=cf(j-1);result+=(unsigned long)(*pp-48)*cfdata;pp++;}
return(result);
}



double strtofloat(char *phead)
{unsigned long zsu,xsu,cfs;
int zsbit=0,xsbit=0;
char zs[10]={0,0,0,0,0,0,0,0,0,0},xs[10]={0,0,0,0,0,0,0,0,0,0};
char *p=phead;
double result=0;
for(;*p!='.'&&*p!='\0';p++,zsbit++) zs[zsbit]=*p;zs[zsbit]='\0';p++;
for(;*p!='\0';p++,xsbit++) xs[xsbit]=*p;xs[xsbit]='\0';
zsu=strtoint(zs);xsu=strtoint(xs);cfs=cf(xsbit);
result=zsu+(double)xsu/cfs;
return(result);
}





int getintbits(unsigned long aa)
{int j;
if(aa<1) return(1);
for(j=0;;j++){if(aa<1) break;aa/=10;}
return (j);
}



void getstrconsist(unsigned int *hznum,unsigned int *charnum,char *p)
{char sign=0;*hznum=0;*charnum=0;
do{
	if(!(*p)) break;
	if(*p<128) (*charnum)++;
	else {if(!sign) sign=1;
	else {(*hznum)++;sign=0;}
	}
	p++;
}while(1);
}


unsigned int getcharnum(char *p)
{char sign=0;
unsigned int hznum=0,charnum=0;
do{
	if(!(*p)) break;
	if(*p<128) charnum++;
	else {if(!sign) sign=1;
	else {hznum++;sign=0;}
	}
	p++;
}while(1);
return(hznum+charnum);
}



void delstr(char *p,int from,int num)
{int i;char *temp;
if(!p||num<=0||from<1||!(*p)) return;
temp=p+from-1;
for(i=0;i<num;i++,temp++) if(!(*(temp))) break;//此句是防止num大于字符串剩下的长度
num=i;temp=p+from-1;

for(;*temp;temp++) {*temp=*(temp+num);if(!(*(temp+num))) break;}
}


void insstr(char *dest,int from,char *source)
{int num,curno;char *tmpdst,*tmpsrc=source;
if(!dest||!source||from<0||!(*source)) return;
tmpdst=dest+from;
for(num=0;*(tmpsrc+num);num++);    //找到源字串长度
if(!num) return;
for(curno=1;*tmpdst;tmpdst++,curno++);   //移动到目的字串的结束标志
for(;curno>=0;tmpdst--,curno--) *(tmpdst+num)=*tmpdst;   //逐个向后移动
tmpdst=dest+from;
for(;*tmpsrc;tmpdst++,tmpsrc++) *tmpdst=*tmpsrc;   //将源字串拷贝过去
}


char *copystr(char *source,int from,int num)
{int i;char *tmpdst,*tmpsrc;
if(!source||from<1||num<1||!(*source)) return(0);
tmpsrc=source+from-1;
for(i=0;i<num&&*(tmpsrc+i);i++);  num=i;    //确保from+num超出source边界的特殊情况
tmpdst=(char *)malloc(i+1);
for(i=0;i<num;i++) *(tmpdst+i)=*(tmpsrc+i);
*(tmpdst+i)=0;
return(tmpdst);
}



void wordtostr(char *word,char *str)      //给字符数组str赋一个字符串word
{char *pp=str;
do{
	*pp=*word;
	if(*pp=='\0') break;
	pp++;word++;

}while(1);
}




UINT GetFloatFromString(char *pString, float *pRetrieveData, UINT iFloatDataNum/* = 0*/)
{
	if (pString == NULL)
		return 0;

	char pBuf[50] = "", pSrcDataHead[1000] = "", *pSrcData = pSrcDataHead;
	strcpy(pSrcData, pString);

	char *pSubSectionHead = pSrcData, *pSubSectionEnd = NULL;
	float *pCurrDataWrite = pRetrieveData;
	UINT  i = 0;

	while( (pSubSectionEnd=strchr(pSrcData, ',')) && (i<iFloatDataNum || !iFloatDataNum) )
	{
		// 结束当前Head-End的一段（段尾置空），pSrcData跳跃到下一段
		*pSubSectionEnd = 0;
		pSrcData = pSubSectionEnd + 1;

		strcpy(pBuf, pSubSectionHead);
		*pCurrDataWrite++ = (float)atof(pBuf);

		i++;
		pSubSectionHead = pSrcData;
	}
	// 跳出循环以后，说明没有,了，但剩下最后一个float数还要继续处理
	strcpy(pBuf, pSubSectionHead);
	*pCurrDataWrite++ = (float)atof(pBuf);
	i++;


	return i;
}

/************************************************************************/
/*								Math                                    */
/************************************************************************/
float absf(float fNum)
{
	if(fNum < 0)
		return -fNum;
	else
		return fNum;
}
double absf(double fNum)
{
	if(fNum < 0)
		return -fNum;
	else
		return fNum;
}

unsigned long cf(int n)
{unsigned long result=1;
int a;
for(a=1;a<=n;a++) {if(n==0) break;result=result*10;}
return(result);
}

UINT CheckPowerOf2(UINT iValue)
{
	if(!iValue)
		return 0;

	UINT iMod = iValue % 2;
	UINT iPower = 0;
	// 如果可以被2整除，就继续，如果是2的幂，最后必定除得只剩下1
	while(iMod == 0)
	{
		iValue /= 2;
		iMod = iValue % 2;
		iPower++;
	}
	if(iValue == 1)
		return iPower;
	else
		return 0;
}

UINT ReverseBitOrder(UINT iValue, UINT iBitNum)
{
	if(iBitNum > 32)
		return 0xffffffff;

	UINT iResult = 0;
	UINT iBitValue = 0;
	for(UINT i = 0; i < iBitNum; i++)
	{
		// 先得到当前处理位的原始数据，并移到最低位备用，为了便于后面“或”，其他位都是0
		iBitValue = (iValue >> i) & 0x1;
		// 将原始位数据移到指定地点（位反序）
		iResult |= iBitValue << (iBitNum-1 - i);
	}
	// 高位保持不变
	for(int i = iBitNum; i < 32; i++)
	{
		// 先得到当前处理位的原始数据，并移到最低位备用，为了便于后面“或”，其他位都是0
		iBitValue = (iValue >> i) & 0x1;
		// 将原始位数据移到指定地点（位反序）
		iResult |= iBitValue << i;
	}


	return iResult;
}



// clamp，最后两个参数表示clamp的范围
float clampf(float fValue, float fStandardmin /* = 0.0f */, float fStandardmax /* = 0.0f */)
{
	// 非法
	if(fStandardmax < fStandardmin)
		return fValue;

	if(fValue < fStandardmin)
		fValue = fStandardmin;

	if(fValue > fStandardmax)
		fValue = fStandardmax;

	// 防止fValue跟fStandard很相近的时候，上面的比较会失败，产生浮点数运算误差
	if(absf(fValue - fStandardmin) < 0.00001f)
		fValue = fStandardmin;
	if(absf(fValue - fStandardmax) < 0.00001f)
		fValue = fStandardmax;

	return fValue;
}

// 四舍五入，最后一个参数表示四舍五入的分界，必须在0～1之间
int roundf(float fValue, float fStandard /* = 0.5f */)
{
	if(fStandard >= 1.0f || fStandard <= 0.0f)
		return 0;

	int iValue = (int)fValue;
	float fMantissa = (float)fValue - iValue;
	if(fMantissa >= fStandard)
		iValue++;
	return iValue;
}

// 计算阶乘
int CalcFactorial(int iData)
{
	if(iData == 0)
		return 1;
	int iResult = 1;
	for(; iData > 0; iData--)
		iResult *= iData;
	return iResult;
}
// 双阶乘
int CalcDoubleFactorial(int iData)
{
	if(iData == 0)
		return 1;
	int iResult = 1;
	for(; iData > 0; iData--)
		if(iData % 2)
			iResult *= iData;
	return iResult;
}


// 只选前3x3的部分求Det
float GetDeterminant3X3(D3DXMATRIX Mat)
{
	float fDet = 0.0f;
	fDet += Mat._11 * Mat._22 * Mat._33;
	fDet += Mat._12 * Mat._23 * Mat._31;
	fDet += Mat._13 * Mat._21 * Mat._32;
	fDet -= Mat._11 * Mat._23 * Mat._32;
	fDet -= Mat._12 * Mat._21 * Mat._33;
	fDet -= Mat._13 * Mat._22 * Mat._31;
	return fDet;
}





/***************************Randomize************************/
/*static unsigned int RAND_SEED;
unsigned int random(int range)
{RAND_SEED=(RAND_SEED*123+59)%range;
return(RAND_SEED);
}
void randomize(void)
{int temp[2];
movedata(0x0040,0x006c,FP_SEG(temp),FP_OFF(temp),4);
RAND_SEED=temp[0];}
*/

void randomize(void)
{ srand( (unsigned)time( NULL ) ); }

int random(int number)
{
	return (int)(number/(float)RAND_MAX * rand());
}

// 浮点数的随机数（括号后是精度范围，表示小数点后多少位，不能太高，否则随机数反而不精确）
// 在整数随机数的函数中，如果传入的为负数，那么返回的也全部是负数（或0）
// 返回小于fRandomValue的数，如果它为负，返回的也全为负数或0
float randomf(float fRandomValue, UINT iBitNum/* = 4 */)
{
	// 基本原理是将浮点数扩大（乘10的方）后再得随机数，然后再缩小回原范围
	float fInt = powf(10.0f, (float)iBitNum);
	int iRandomValue = (int)(fRandomValue * fInt);

	float fValue = (float)random(iRandomValue);
	return fValue / fInt;
}



double gaussrandom()
{
	static double V1, V2, S;
	static int phase = 0;
	double X;

	if ( phase == 0 ) {
		do {
			double U1 = (double)rand() / RAND_MAX;
			double U2 = (double)rand() / RAND_MAX;

			V1 = 2 * U1 - 1;
			V2 = 2 * U2 - 1;
			S = V1 * V1 + V2 * V2;
		} while(S >= 1 || S == 0);

		X = V1 * sqrt(-2 * log(S) / S);
	} else
		X = V2 * sqrt(-2 * log(S) / S);

	phase = 1 - phase;

	return X;
}










/************************************************************************/
/*							Color Convertor                             */
/************************************************************************/
DWORD VectoRGBSigned(float x, float y, float z, float w)
{
	D3DXVECTOR3 vec=D3DXVECTOR3(x,y,z);
	//if(D3DXVec3Length(&vec)>1)
	D3DXVec3Normalize(&vec, &vec);
	DWORD r,g,b,a;
	r=(DWORD)(vec.x*127+128);
	g=(DWORD)(vec.y*127+128);
	b=(DWORD)(vec.z*127+128);
	a=(DWORD)(w*255);
	return a<<24|r<<16|g<<8|b;
}


DWORD VectoRGBUnsigned(float x, float y, float z, float w)
{
	D3DXVECTOR3 vec=D3DXVECTOR3(x,y,z);
	D3DXVec3Normalize(&vec, &vec);
	DWORD r,g,b,a;
	r=(DWORD)(vec.x*255);
	g=(DWORD)(vec.y*255);
	b=(DWORD)(vec.z*255);
	a=(DWORD)(w*255);
	return a<<24|r<<16|g<<8|b;
}






DWORD ColortoRGBSigned(float x, float y, float z, float w)
{
	D3DXVECTOR3 vec=D3DXVECTOR3(x,y,z);
	DWORD r,g,b,a;
	r=(DWORD)(vec.x*127+128);
	g=(DWORD)(vec.y*127+128);
	b=(DWORD)(vec.z*127+128);
	a=(DWORD)(w*255);
	return a<<24|r<<16|g<<8|b;
}


DWORD ColortoRGBUnsigned(float x, float y, float z, float w)
{
	D3DXVECTOR3 vec=D3DXVECTOR3(x,y,z);
	DWORD r,g,b,a;
	r=(DWORD)(x*255);
	g=(DWORD)(y*255);
	b=(DWORD)(z*255);
	a=(DWORD)(w*255);
	return a<<24|r<<16|g<<8|b;
}






float FP16Tofloat(float16 fValue)
{
	float fRetValue = 0.0f;
	UINT iRetValue = *((UINT *)&fRetValue);

	UINT iTemp = 0;
	unsigned char iExpValue = 0;
	float16 fTemp = 0;

	// 先检查是不是0
	if(fValue == 0)
		return 0.0f;


	// 拷贝符号位
	iRetValue = fValue >> 15;
	iRetValue = iRetValue << 31;

	// 转换指数位
	iExpValue = (unsigned char)(fValue >> 10) & 0x1f;
	iExpValue -= 15;	// 得到真正的指数
	iTemp = iExpValue + 127;	// 换算成float的指数存储
	iTemp &= 0xff;
	iTemp = iTemp << 23;
	iRetValue |= iTemp;

	// 截断尾数位
	iTemp = fValue & 0x3ff;
	iTemp = iTemp << 13;
	iRetValue |= iTemp;

	fRetValue = *((float *)&iRetValue);
	return fRetValue;
}


float16 floatToFP16(float fValue)
{
	UINT *pValue = (UINT *)&fValue;
	UINT iValue = *pValue;

	float16 fRetValue = 0;
	unsigned char iExpValue = 0;
	UINT iTempLong = 0;
	float16 fTemp = 0;

	// 先检查是不是0
	if(iValue == 0)
		return 0;

	// 拷贝符号位
	fRetValue = iValue >> 31;
	fRetValue = fRetValue << 15;
	if(fRetValue)
		OutputDebugString("警告：FP16数据为负数！\n");

	// 转换指数位（小心越界！）
	iExpValue = (iValue >> 23) & 0xff;
	iExpValue -= 127;	// 得到真正的指数
	signed char fTestExp = (signed)iExpValue;
	if(fTestExp < -15)
	{
		OutputDebugString("警告：转换FP16数据越界！原float数太小！强制置0！\n");
		return 0;
	}
	if(fTestExp > 15)
	{
		OutputDebugString("错误：转换FP16数据越界！原float数太大！\n");
		return 0;
	}

	fTemp = iExpValue + 15;	// 换算成FP16的指数
	fTemp &= 0x1f;
	fTemp = fTemp << 10;
	fRetValue |= fTemp;

	// 截断尾数位
	iTempLong = iValue & 0x7fffff;
	fTemp = iTempLong >> 13;	// fp16和float差13位
	fRetValue |= fTemp;

	return fRetValue;
}









/************************************************************************/
/*				Space Coordinates Matrix Points and Vectors             */
/************************************************************************/

// 根据任意旋转矩阵来得到欧拉矩阵和欧拉角
D3DXMATRIX XZXEulerRotation(LPD3DXMATRIX pMat, D3DXVECTOR3 VecAxis, float fAngle, LPD3DXVECTOR3 pVecEulerAngle/* = NULL*/)
{
	D3DXMATRIX MatRotationCombine;
	D3DXMATRIX MatZ1, MatY1, MatZ2;
	D3DXVECTOR3 X(1,0,0), Y(0,1,0), Z(0,0,1);
	D3DXVECTOR3 x(1,0,0), y(0,1,0), z(0,0,1);
	D3DXVECTOR3 x1, x2, x3, y1, y2, y3, z1, z2, z3;

	// 根据参数构造旋转坐标轴的矩阵
	D3DXMatrixRotationAxis(&MatRotationCombine, &VecAxis, fAngle);

	// 旋转坐标轴
	D3DXVec3TransformCoord(&x, &X, &MatRotationCombine);
	D3DXVec3TransformCoord(&y, &Y, &MatRotationCombine);
	D3DXVec3TransformCoord(&z, &Z, &MatRotationCombine);

	D3DXVec3Normalize(&x, &x);
	D3DXVec3Normalize(&y, &y);
	D3DXVec3Normalize(&z, &z);


	D3DXVECTOR3 VecTemp(0, 0, 0);
	D3DXVECTOR3 VecCross(0, 0, 0);

	// 得到Alpha
	// 先得到x在YZ面的投影，很简单哦，取YZ分量就好
	D3DXVec3Normalize(&VecTemp, &D3DXVECTOR3(0, x.y, x.z));
	// 得到投影向量和Y的夹角，同样的，要判断旋转方向
	float fAlpha = acosf( D3DXVec3Dot(&VecTemp, &Y) );
	D3DXVec3Cross(&VecCross, &Y, &VecTemp);	// Y向投影向量旋转，叉乘前后顺序要注意
	if( D3DXVec3Dot(&VecCross, &X) < 0 )	// 和旋转轴的cos小于0说明是反方向
		fAlpha = -fAlpha;

	// 构造旋转矩阵
	D3DXMatrixRotationAxis(&MatZ1, &X, fAlpha);
	D3DXVec3TransformCoord(&x1, &X, &MatZ1);
	D3DXVec3TransformCoord(&y1, &Y, &MatZ1);
	D3DXVec3TransformCoord(&z1, &Z, &MatZ1);
	D3DXVec3Normalize(&x1, &x1);
	D3DXVec3Normalize(&y1, &y1);
	D3DXVec3Normalize(&z1, &z1);



	// 得到Beta，在左手坐标系中，为了判断旋转方向，需要做一个叉积
	float fBeta = acosf( D3DXVec3Dot(&x, &X) );
	D3DXVec3Cross(&VecCross, &X, &x);	// Z向z旋转，叉乘前后顺序要注意
	if( D3DXVec3Dot(&VecCross, &z1) < 0 )	// 和旋转轴的cos小于0说明是反方向
		fBeta = -fBeta;

	// 构造旋转矩阵
	D3DXMatrixRotationAxis(&MatY1, &z1, fBeta);
	D3DXVec3TransformCoord(&x2, &x1, &MatY1);
	D3DXVec3TransformCoord(&y2, &y1, &MatY1);
	D3DXVec3TransformCoord(&z2, &z1, &MatY1);
	D3DXVec3Normalize(&x2, &x2);
	D3DXVec3Normalize(&y2, &y2);
	D3DXVec3Normalize(&z2, &z2);


	// 得到Gamma
	// 先得到黑线，即Y绕Z转动Alpha
	float fGamma = acosf( D3DXVec3Dot(&z1, &z) );
	D3DXVec3Cross(&VecCross, &z1, &z);		// 黑线向y旋转，叉乘前后顺序要注意
	if( D3DXVec3Dot(&VecCross, &x2) < 0 )	// 和旋转轴的cos小于0说明是反方向
		fGamma = -fGamma;

	D3DXMatrixRotationAxis(&MatZ2, &x2, fGamma);

	// 组合矩阵
	MatRotationCombine = MatZ1 * MatY1 * MatZ2;

	if(pMat)
	{
		*pMat = MatRotationCombine;
	}
	if(pVecEulerAngle)
	{
		*pVecEulerAngle = D3DXVECTOR3(fAlpha, fBeta, fGamma);
	}
	return MatRotationCombine;
}


D3DXMATRIX ZYZEulerRotation(LPD3DXMATRIX pMat, D3DXVECTOR3 VecAxis, float fAngle, LPD3DXVECTOR3 pVecEulerAngle/* = NULL*/)
{
	D3DXMATRIX MatRotationCombine;
	D3DXMATRIX MatZ1, MatY1, MatZ2;
	D3DXVECTOR3 X(1,0,0), Y(0,1,0), Z(0,0,1);
	D3DXVECTOR3 x(1,0,0), y(0,1,0), z(0,0,1);
	D3DXVECTOR3 x1, x2, x3, y1, y2, y3, z1, z2, z3;

	// 根据参数构造旋转坐标轴的矩阵
	D3DXMatrixRotationAxis(&MatRotationCombine, &VecAxis, fAngle);

	// 旋转坐标轴
	D3DXVec3TransformCoord(&x, &X, &MatRotationCombine);
	D3DXVec3TransformCoord(&y, &Y, &MatRotationCombine);
	D3DXVec3TransformCoord(&z, &Z, &MatRotationCombine);

	D3DXVec3Normalize(&x, &x);
	D3DXVec3Normalize(&y, &y);
	D3DXVec3Normalize(&z, &z);


	D3DXVECTOR3 VecTemp(0, 0, 0);
	D3DXVECTOR3 VecCross(0, 0, 0);

	// 得到Alpha
	// 先得到z在XY面的投影，很简单哦，取XY分量就好
	D3DXVec3Normalize(&VecTemp, &D3DXVECTOR3(z.x, z.y, 0));
	// 得到投影向量和X的夹角，同样的，要判断旋转方向
	float fAlpha = acosf( D3DXVec3Dot(&VecTemp, &X) );
	D3DXVec3Cross(&VecCross, &X, &VecTemp);	// X向投影向量旋转，叉乘前后顺序要注意
	if( D3DXVec3Dot(&VecCross, &Z) < 0 )	// 和旋转轴的cos小于0说明是反方向
		fAlpha = -fAlpha;

	// 构造旋转矩阵
	D3DXMatrixRotationAxis(&MatZ1, &Z, fAlpha);
	D3DXVec3TransformCoord(&x1, &X, &MatZ1);
	D3DXVec3TransformCoord(&y1, &Y, &MatZ1);
	D3DXVec3TransformCoord(&z1, &Z, &MatZ1);
	D3DXVec3Normalize(&x1, &x1);
	D3DXVec3Normalize(&y1, &y1);
	D3DXVec3Normalize(&z1, &z1);



	// 得到Beta，在左手坐标系中，为了判断旋转方向，需要做一个叉积
	float fBeta = acosf( D3DXVec3Dot(&z, &Z) );
	D3DXVec3Cross(&VecCross, &Z, &z);	// Z向z旋转，叉乘前后顺序要注意
	if( D3DXVec3Dot(&VecCross, &y1) < 0 )	// 和旋转轴的cos小于0说明是反方向
		fBeta = -fBeta;

	// 构造旋转矩阵
	D3DXMatrixRotationAxis(&MatY1, &y1, fBeta);
	D3DXVec3TransformCoord(&x2, &x1, &MatY1);
	D3DXVec3TransformCoord(&y2, &y1, &MatY1);
	D3DXVec3TransformCoord(&z2, &z1, &MatY1);
	D3DXVec3Normalize(&x2, &x2);
	D3DXVec3Normalize(&y2, &y2);
	D3DXVec3Normalize(&z2, &z2);


	// 得到Gamma
	// 先得到黑线，即Y绕Z转动Alpha
	float fGamma = acosf( D3DXVec3Dot(&y1, &y) );
	D3DXVec3Cross(&VecCross, &y1, &y);		// 黑线向y旋转，叉乘前后顺序要注意
	if( D3DXVec3Dot(&VecCross, &z2) < 0 )	// 和旋转轴的cos小于0说明是反方向
		fGamma = -fGamma;

	D3DXMatrixRotationAxis(&MatZ2, &z2, fGamma);

	// 组合矩阵
	MatRotationCombine = MatZ1 * MatY1 * MatZ2;

	if(pMat)
	{
		*pMat = MatRotationCombine;
	}
	if(pVecEulerAngle)
	{
		*pVecEulerAngle = D3DXVECTOR3(fAlpha, fBeta, fGamma);
	}
	return MatRotationCombine;
}


// 重载，根据四元组来得到欧拉矩阵和欧拉角
D3DXMATRIX XZXEulerRotation(LPD3DXMATRIX pMat, D3DXQUATERNION Quater, LPD3DXVECTOR3 pVecEulerAngle/* = NULL*/)
{
	float fAngle = 0.0f;
	D3DXVECTOR3 VecAxis(0, 0, 0);
	D3DXQuaternionToAxisAngle(&Quater, &VecAxis, &fAngle);
	return XZXEulerRotation(pMat, VecAxis, fAngle, pVecEulerAngle);
}

D3DXMATRIX ZYZEulerRotation(LPD3DXMATRIX pMat, D3DXQUATERNION Quater, LPD3DXVECTOR3 pVecEulerAngle/* = NULL*/)
{
	float fAngle = 0.0f;
	D3DXVECTOR3 VecAxis(0, 0, 0);
	D3DXQuaternionToAxisAngle(&Quater, &VecAxis, &fAngle);
	return ZYZEulerRotation(pMat, VecAxis, fAngle, pVecEulerAngle);
}



// 根据欧拉角得到欧拉矩阵
D3DXMATRIX GetZYZEulerMatrix(LPD3DXMATRIX pMat, D3DXVECTOR3 VecEulerAngle)
{
	D3DXMATRIX MatRotationCombine;
	D3DXMATRIX MatZ1, MatY1, MatZ2;
	D3DXVECTOR3 X(1,0,0), Y(0,1,0), Z(0,0,1);
	D3DXVECTOR3 x1, x2, x3, y1, y2, y3, z1, z2, z3;

	D3DXMatrixIdentity(&MatRotationCombine);


	// 根据Alpha构造旋转矩阵
	D3DXMatrixRotationAxis(&MatZ1, &Z, VecEulerAngle.x);
	D3DXVec3TransformCoord(&x1, &X, &MatZ1);
	D3DXVec3TransformCoord(&y1, &Y, &MatZ1);
	D3DXVec3TransformCoord(&z1, &Z, &MatZ1);
	D3DXVec3Normalize(&x1, &x1);
	D3DXVec3Normalize(&y1, &y1);
	D3DXVec3Normalize(&z1, &z1);



	// 根据Beta构造旋转矩阵
	D3DXMatrixRotationAxis(&MatY1, &y1, VecEulerAngle.y);
	D3DXVec3TransformCoord(&x2, &x1, &MatY1);
	D3DXVec3TransformCoord(&y2, &y1, &MatY1);
	D3DXVec3TransformCoord(&z2, &z1, &MatY1);
	D3DXVec3Normalize(&x2, &x2);
	D3DXVec3Normalize(&y2, &y2);
	D3DXVec3Normalize(&z2, &z2);


	// 根据Gamma得到旋转矩阵
	D3DXMatrixRotationAxis(&MatZ2, &z2, VecEulerAngle.z);

	// 组合矩阵
	MatRotationCombine = MatZ1 * MatY1 * MatZ2;

	if(pMat)
	{
		*pMat = MatRotationCombine;
	}
	return MatRotationCombine;
}


D3DXMATRIX GetXZXEulerMatrix(LPD3DXMATRIX pMat, D3DXVECTOR3 VecEulerAngle)
{
	D3DXMATRIX MatRotationCombine;
	D3DXMATRIX MatZ1, MatY1, MatZ2;
	D3DXVECTOR3 X(1,0,0), Y(0,1,0), Z(0,0,1);
	D3DXVECTOR3 x1, x2, x3, y1, y2, y3, z1, z2, z3;

	D3DXMatrixIdentity(&MatRotationCombine);


	// 根据Alpha构造旋转矩阵
	D3DXMatrixRotationAxis(&MatZ1, &X, VecEulerAngle.x);
	D3DXVec3TransformCoord(&x1, &X, &MatZ1);
	D3DXVec3TransformCoord(&y1, &Y, &MatZ1);
	D3DXVec3TransformCoord(&z1, &Z, &MatZ1);
	D3DXVec3Normalize(&x1, &x1);
	D3DXVec3Normalize(&y1, &y1);
	D3DXVec3Normalize(&z1, &z1);



	// 根据Beta构造旋转矩阵
	D3DXMatrixRotationAxis(&MatY1, &z1, VecEulerAngle.y);
	D3DXVec3TransformCoord(&x2, &x1, &MatY1);
	D3DXVec3TransformCoord(&y2, &y1, &MatY1);
	D3DXVec3TransformCoord(&z2, &z1, &MatY1);
	D3DXVec3Normalize(&x2, &x2);
	D3DXVec3Normalize(&y2, &y2);
	D3DXVec3Normalize(&z2, &z2);


	// 根据Gamma得到旋转矩阵
	D3DXMatrixRotationAxis(&MatZ2, &x2, VecEulerAngle.z);

	// 组合矩阵
	MatRotationCombine = MatZ1 * MatY1 * MatZ2;

	if(pMat)
	{
		*pMat = MatRotationCombine;
	}
	return MatRotationCombine;
}



// 笛卡尔到球面坐标的互相转换，球面坐标是VECTOR2(Theta, Phi)，第一个是和垂直轴（Z）的夹角，第二个是在底面（XZ）的投影和横轴（X）的夹角
D3DXVECTOR2 CartesianToSpherical(D3DXVECTOR3 VecCarte, LPD3DXVECTOR2 pVecSphere)
{
	D3DXVECTOR2 VecSphere;
	D3DXVec3Normalize(&VecCarte, &VecCarte);

	// 算和垂直轴的夹角
	VecSphere.x = acosf(VecCarte.z);

	// 算投影和横轴的夹角
	// atan函数在y正负轴会断掉，这里强制给值
	if(absf(VecCarte.x) < 0.00001f)
	{
		if(VecCarte.y > 0.0f)
			VecSphere.y = D3DX_PI / 2.0f;
		else
			VecSphere.y = D3DX_PI * 3.0f / 2.0f;
	}
	else
	{
		VecSphere.y = atanf(VecCarte.y / VecCarte.x);

		// 第二、三象限修正
		if(VecCarte.x < 0.0f)
		{		
			VecSphere.y = D3DX_PI + VecSphere.y;
		}
		// 第四象限修正
		if(VecCarte.y < 0.0f && VecCarte.x >= 0.0f)
		{
			VecSphere.y = D3DX_PI * 2 + VecSphere.y;
		}
	}

	*pVecSphere = VecSphere;
	return VecSphere;
}

// 球面坐标是VECTOR2(Theta, Phi)，第一个是和垂直轴（Z）的夹角，第二个是在底面（XZ）的投影和横轴（X）的夹角
D3DXVECTOR3 SphericalToCartesian(D3DXVECTOR2 VecSphere, LPD3DXVECTOR3 pVecCarte)
{
	D3DXVECTOR3 VecCarte;

	VecCarte.x = cosf(VecSphere.y) * sinf(VecSphere.x);
	VecCarte.y = sinf(VecSphere.y) * sinf(VecSphere.x);
	VecCarte.z = cosf(VecSphere.x);

	D3DXVec3Normalize(&VecCarte, &VecCarte);
	*pVecCarte = VecCarte;
	return VecCarte;
}


// 生成正交向量组
HRESULT GenerateViewSpace(D3DXVECTOR3 *pVecLookAt_Z, D3DXVECTOR3 *pVecHead_Y, D3DXVECTOR3 *pVecRight_X)
{
	if(!pVecLookAt_Z || !pVecHead_Y || !pVecRight_X)
		return D3DERR_INVALIDCALL;
	if(D3DXVec3Length(pVecLookAt_Z) == 0 || D3DXVec3Length(pVecHead_Y) == 0 || *pVecHead_Y == *pVecLookAt_Z)
		return D3DERR_NOTAVAILABLE;

	D3DXVec3Normalize(pVecLookAt_Z, pVecLookAt_Z);
	D3DXVec3Normalize(pVecHead_Y, pVecHead_Y);

	// 以ZY面为基础，求面法线（即X轴）
	D3DXVec3Cross(pVecRight_X, pVecHead_Y, pVecLookAt_Z);
	D3DXVec3Normalize(pVecRight_X, pVecRight_X);

	// Y不一定和Z正交，但X一定和Z正交，因为Z是视线向量不能变，所以重新求Y的正交，即头顶向量
	D3DXVec3Cross(pVecHead_Y, pVecLookAt_Z, pVecRight_X);
	D3DXVec3Normalize(pVecHead_Y, pVecHead_Y);

	return S_OK;
}








/************************************************************************/
/*							Get Intersect                               */
/************************************************************************/

bool GetIntersect2D(D3DXVECTOR3 *P, D3DXVECTOR3 P1, D3DXVECTOR3 P2, D3DXVECTOR3 V1, D3DXVECTOR3 V2)
{
	// 先检查两条直线是否平行，平行就返回错误值，表示无交点
	D3DXVECTOR3 VecP, VecV;
	VecP = P2 - P1;
	VecV = V2 - V1;
	D3DXVec3Normalize(&VecP, &VecP);
	D3DXVec3Normalize(&VecV, &VecV);
	float fFov = absf(D3DXVec3Dot(&VecP, &VecV));	// 注意不要忘了abs一下，因为dot可能会出现负值
	if(absf(1.0f - fFov) < 0.00001f)
		return false;


	// 检查是否有垂直的，因为垂线的方程并非y = kx + b形式
	bool bPVertical = false, bVVertical = false;
	if(absf(P2.x - P1.x) < 0.0001f)
		bPVertical = true;
	if(absf(V2.x - V1.x) < 0.0001f)
		bVVertical = true;


	// 计算两条直线的方程
	float k1, k2, b1, b2;
	if(!bPVertical)
	{
		k1 = (P2.y - P1.y) / (P2.x - P1.x);
		b1 = P1.y - k1 * P1.x;
	}
	if(!bVVertical)
	{
		k2 = (V2.y - V1.y) / (V2.x - V1.x);
		b2 = V1.y - k2 * V1.x;
	}


	// 计算交点
	D3DXVECTOR3 PtIntersect;
	if(bPVertical && !bVVertical)
	{
		PtIntersect.x = P1.x;
		PtIntersect.y = P1.x * k2 + b2;
	}
	else if(!bPVertical && bVVertical)
	{
		PtIntersect.x = V1.x;
		PtIntersect.y = V1.x * k1 + b1;
	}
	// 不可能两条直线都垂直，如果都垂直，就会早早的返回（Dot = 1）
	else
	{
		PtIntersect.x = (b2 - b1) / (k1 - k2);
		PtIntersect.y = PtIntersect.x * k1 + b1;
	}

	*P =  PtIntersect;
	return true;
}


bool IsPointInTriangle(D3DXVECTOR3 P, D3DXVECTOR3 PtV1, D3DXVECTOR3 PtV2, D3DXVECTOR3 PtV3)
{
/*	// 也可以根据重心坐标系来确定，根据A1 + f * (A2-A1) + g * (A3-A1) = P； h = 1 - f - g
	float f, g, h;	// 系数
	D3DXVECTOR3 temp = (P - A1) / ( (P-A1)/(P-A2) * (A2-A1) + A3-A1 );
	return true;
*/
	// 求三条边的规格化向量
	D3DXVECTOR3 A1 = PtV2 - PtV1;	// V1->V2
	D3DXVECTOR3 A2 = PtV3 - PtV1;	// V1->V3
	D3DXVECTOR3 A3 = PtV2 - PtV3;	// V3->V2
	D3DXVec3Normalize(&A1, &A1);
	D3DXVec3Normalize(&A2, &A2);
	D3DXVec3Normalize(&A3, &A3);

	// 三角形的三个夹角
	float fFov[3], fTemp[2];
	fFov[0] = acosf(D3DXVec3Dot(&A1, &A2));	// V1V2和V1V3的夹角
	fFov[1] = acosf(D3DXVec3Dot(&(-A1), &(-A3)));	// V1V2和V2V3的夹角
	fFov[2] = acosf(D3DXVec3Dot(&(-A2), &A3));	// V1V3和V2V3的夹角
	
	// 检测V1到P
	D3DXVECTOR3 VecP = P - PtV1;
	D3DXVec3Normalize(&VecP, &VecP);
	fTemp[0] = acosf(D3DXVec3Dot(&VecP, &A1));
	fTemp[1] = acosf(D3DXVec3Dot(&VecP, &A2));
	if(fTemp[0] >= fFov[0] || fTemp[1] >= fFov[0])
		return false;

	// 检测V2到P
	VecP = P - PtV2;
	D3DXVec3Normalize(&VecP, &VecP);
	fTemp[0] = acosf(D3DXVec3Dot(&VecP, &(-A1)));
	fTemp[1] = acosf(D3DXVec3Dot(&VecP, &(-A3)));
	if(fTemp[0] >= fFov[1] || fTemp[1] >= fFov[1])
		return false;

	// 检测V3到P
	VecP = P - PtV3;
	D3DXVec3Normalize(&VecP, &VecP);
	fTemp[0] = acosf(D3DXVec3Dot(&VecP, &(-A2)));
	fTemp[1] = acosf(D3DXVec3Dot(&VecP, &A3));
	if(fTemp[0] >= fFov[2] || fTemp[1] >= fFov[2])
		return false;

	return true;
}





// 射线跟踪求交，得到和平面的交点
BOOL GetIntersectPlane(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXPLANE Plane, LPD3DXVECTOR3 pPtIntersect)
{
	D3DXVECTOR3 VecNormal = D3DXVECTOR3(Plane.a, Plane.b, Plane.c);
	D3DXVec3Normalize(&VecNormal, &VecNormal);

	// 和平面接近平行或背离，不相交
	if(D3DXVec3Dot(&VecRay, &(-1*VecNormal)) < 0.00001f)
		return FALSE;
	// 平面参数不正确，返回失败
	if(D3DXVec3Length(&VecNormal) == 0.0f)
		return FALSE;


	// 过射线点做平行于求交平面的第二个平面，即Normal dot Point + d = 0
	float fPlane2_D = -1.0f * D3DXVec3Dot(&VecNormal, &PtStart);

	// 求点到平面的距离和点到平面的射线
	float fDistance = absf(Plane.d - fPlane2_D);
	D3DXVECTOR3 PtOnPlane = PtStart - VecNormal*fDistance;
	D3DXVECTOR3 VecPerpendicular = PtOnPlane - PtStart;
	D3DXVec3Normalize(&VecPerpendicular, &VecPerpendicular);

	// 假定相交点到起点的距离为D，那么fDistance / d = cos(垂线和射线夹角)，垂线和法线是一样滴
	float fD = absf( fDistance / D3DXVec3Dot(&VecPerpendicular, &VecRay) );

	if(pPtIntersect)
	{
		*pPtIntersect = PtStart + fD * VecRay;
	}

	return TRUE;
}




BOOL GetIntersectBox3D(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXVECTOR3 PtMin, D3DXVECTOR3 PtMax)
{
	BOOL bIntersect = FALSE;

	D3DXVec3Normalize(&VecRay, &VecRay);
	if( absf(D3DXVec3Length(&VecRay) - 1.0f) > 0.000001f )
		return FALSE;

	// 先根据顶点位置和射线方向构造矩阵
	D3DXVECTOR3 VecRotationAxis(0, 0, 0);
	float fDot = 0.0f;
	D3DXMATRIX Mat, MatTranslation;

	// 先构造平移矩阵
	D3DXMatrixTranslation(&MatTranslation, -PtStart.x, -PtStart.y, -PtStart.z);
	// 再找到Ray和Z轴的夹角，如果重合（cos = 1）就直接跳过旋转
	fDot = D3DXVec3Dot(&VecRay, &D3DXVECTOR3(0, 0, 1));
	if(fDot > 0.9999999f)
		Mat = MatTranslation;
	else
	{
		// 再根据Z轴与射线方向，找到旋转轴，注意叉乘方向，RotationAxis都是绕轴逆时针旋转的
		D3DXVec3Cross(&VecRotationAxis, &VecRay, &D3DXVECTOR3(0, 0, 1));
		D3DXMatrixRotationAxis(&Mat, &VecRotationAxis, acosf(fDot));

		// 先平移，再旋转
		Mat = MatTranslation * Mat;
	}

	// 变换BOX两点
	D3DXVECTOR3 PtMaxTrans(0, 0, 0), PtMinTrans(0, 0, 0);
	D3DXVec3TransformCoord(&PtMaxTrans, &PtMax, &Mat);
	D3DXVec3TransformCoord(&PtMinTrans, &PtMin, &Mat);

	// 判断求交，X或Y同号说明在一侧，不相交，等于0说明擦边而过，也算相交
	if( (PtMaxTrans.x * PtMinTrans.x) > 0.0f || (PtMaxTrans.y * PtMinTrans.y) > 0.0f )
		bIntersect = FALSE;
	else
		bIntersect = TRUE;

	// 这里可以用三角面的相交函数得到相交点坐标和相交长度

	return bIntersect;
}



// 射线跟踪求交，三角形A->B->C，绘制顺序必须一致，返回是否相交，同时可以输出交点重心坐标、交点到原始点的长度和笛卡尔坐标（可选）
// 返回的重心坐标x, y, z各自分别代表h, f, g，即分别到A1 A2 A3的长度比例
// 最后一个参数是相交状态，如果在得知是否相交后还想得知具体的相交状态（如是不相交还是背离？相交还是共面？）就可以加上该参数
// 注意这是一个调用频率及其高的函数，为了提速，里面不能调用任何函数包括构造函数，必须在内部手动计算，另外所有的变量都必须是static
BOOL GetIntersectTriangle3D(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXVECTOR3 PtA, D3DXVECTOR3 PtB, D3DXVECTOR3 PtC, LPD3DXVECTOR3 pPtBaryCoord, float *pLength, LPD3DXVECTOR3 pPtIntersect/* = NULL */, MYRAYTRACING_INTERSECT *pIntersectAttribute/* = NULL */)
{

	if(!pPtBaryCoord || !pLength)
	{
		OutputDebugString("必须输入长度和重心坐标的指针，射线相交判定失败！！\n");
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_INVALIDARGUMENTS;
		return FALSE;
	}
	/*为了提速，这里跳过判断，所以必须保证Ray已经规格化而且不能是零向量！
	D3DXVec3Normalize(&VecRay, &VecRay);

	if(D3DXVec3Length(&VecRay) < 0.00001f)
	{
	if(pIntersectAttribute)
	*pIntersectAttribute = INTERSECT_INVALIDARGUMENTS;
	return FALSE;
	}
	*/
	static D3DXVECTOR3 VecLineAB, VecLineAC, VecLineAStart;

	VecLineAB.x = PtB.x - PtA.x;
	VecLineAB.y = PtB.y - PtA.y;
	VecLineAB.z = PtB.z - PtA.z;

	VecLineAC.x = PtC.x - PtA.x;
	VecLineAC.y = PtC.y - PtA.y;
	VecLineAC.z = PtC.z - PtA.z;

	VecLineAStart.x = PtStart.x - PtA.x;
	VecLineAStart.y = PtStart.y - PtA.y;
	VecLineAStart.z = PtStart.z - PtA.z;

	// Direction Pre-Judgement, when ray and face normal have the opposite direction, shows no intersect at all
	// Pre-Judgement could improve performance, but may cause errors, so do not judge here
	//	D3DXVECTOR3 VecNormal(0, 0, 0), VecLine1, VecLine2;
	//	D3DXVec3Normalize(&VecLine1, &VecLineAB);
	//	D3DXVec3Normalize(&VecLine2, &VecLineAC);
	//	D3DXVec3Cross(&VecNormal, &VecLine1, &VecLine2);
	//	D3DXVec3Normalize(&VecNormal, &VecNormal);
	//	float fDot = D3DXVec3Dot(&VecNormal, &VecRay);
	//	if(fDot < 0.0f)
	//	{
	//		if(pIntersectAttribute)
	//			*pIntersectAttribute = INTERSECT_OPPOSITEDIR;
	//		return FALSE;
	//	}
	//	if(absf(fDot) < 0.001f)		// 接近平行
	//	{
	//		if(pIntersectAttribute)
	//			*pIntersectAttribute = INTERSECT_PARALLEL;
	//		return FALSE;
	//	}


	/*为了提速这里去掉判断，所以必须确保输入的面数据是有效的
	// Sideline's Length is too small, demonstrate that the triangle face is invalid
	if(D3DXVec3Length(&VecLineAB) < 0.00001f || D3DXVec3Length(&VecLineAC) < 0.00001f)
	{
	if(pIntersectAttribute)
	*pIntersectAttribute = INTERSECT_INVALIDARGUMENTS;
	return FALSE;
	}
	// 同样为了提速，去掉。这里必须确保射线点不能和面的三个顶点重合！
	if(absf(D3DXVec3Length(&VecLineAStart)) < 0.00001f)
	{
	if(pIntersectAttribute)
	*pIntersectAttribute = INTERSECT_SAMEPOINT;
	return FALSE;
	}
	*/

	// Equation: -Ray*Length + (V2-V1) * f + (V3-V1) * g = Start - V1
	static D3DXVECTOR3 PtBaryCentric(0, 0, 0);	// f, g, h，h = 1 - f - g
	static D3DXMATRIX DetCoef;
	DetCoef._11 = VecRay.x;	DetCoef._12 = VecLineAB.x;	DetCoef._13 = VecLineAC.x;
	DetCoef._21 = VecRay.y;	DetCoef._22 = VecLineAB.y;	DetCoef._23 = VecLineAC.y;
	DetCoef._31 = VecRay.z;	DetCoef._32 = VecLineAB.z;	DetCoef._33 = VecLineAC.z;

	// Get Determinant
	static float fDet = 0.0f;//GetDeterminant3X3(DetCoef);
	fDet = 0.0f;
	fDet += DetCoef._11 * DetCoef._22 * DetCoef._33;
	fDet += DetCoef._12 * DetCoef._23 * DetCoef._31;
	fDet += DetCoef._13 * DetCoef._21 * DetCoef._32;
	fDet -= DetCoef._11 * DetCoef._23 * DetCoef._32;
	fDet -= DetCoef._12 * DetCoef._21 * DetCoef._33;
	fDet -= DetCoef._13 * DetCoef._22 * DetCoef._31;

	// No Intersect
	if((fDet<0.0f ? -fDet : fDet) < 0.000001f)
	{
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_NO;
		return FALSE;
	}


	static D3DXMATRIX D1, D2, D3;
	D1._11 = VecLineAStart.x;	D1._12 = VecLineAB.x;	D1._13 = VecLineAC.x;
	D1._21 = VecLineAStart.y;	D1._22 = VecLineAB.y;	D1._23 = VecLineAC.y;
	D1._31 = VecLineAStart.z;	D1._32 = VecLineAB.z;	D1._33 = VecLineAC.z;

	D2._11 = VecRay.x;	D2._12 = VecLineAStart.x;	D2._13 = VecLineAC.x;
	D2._21 = VecRay.y;	D2._22 = VecLineAStart.y;	D2._23 = VecLineAC.y;
	D2._31 = VecRay.z;	D2._32 = VecLineAStart.z;	D2._33 = VecLineAC.z;

	D3._11 = VecRay.x;	D3._12 = VecLineAB.x;	D3._13 = VecLineAStart.x;
	D3._21 = VecRay.y;	D3._22 = VecLineAB.y;	D3._23 = VecLineAStart.y;
	D3._31 = VecRay.z;	D3._32 = VecLineAB.z;	D3._33 = VecLineAStart.z;

	static float fD1 = 0.0f, fD2 = 0.0f, fD3 = 0.0f;//fD1 = GetDeterminant3X3(D1), fD2 = GetDeterminant3X3(D2), fD3 = GetDeterminant3X3(D3);
	fD1 = 0.0f;
	fD1 += D1._11 * D1._22 * D1._33;
	fD1 += D1._12 * D1._23 * D1._31;
	fD1 += D1._13 * D1._21 * D1._32;
	fD1 -= D1._11 * D1._23 * D1._32;
	fD1 -= D1._12 * D1._21 * D1._33;
	fD1 -= D1._13 * D1._22 * D1._31;

	fD2 = 0.0f;
	fD2 += D2._11 * D2._22 * D2._33;
	fD2 += D2._12 * D2._23 * D2._31;
	fD2 += D2._13 * D2._21 * D2._32;
	fD2 -= D2._11 * D2._23 * D2._32;
	fD2 -= D2._12 * D2._21 * D2._33;
	fD2 -= D2._13 * D2._22 * D2._31;

	fD3 = 0.0f;
	fD3 += D3._11 * D3._22 * D3._33;
	fD3 += D3._12 * D3._23 * D3._31;
	fD3 += D3._13 * D3._21 * D3._32;
	fD3 -= D3._11 * D3._23 * D3._32;
	fD3 -= D3._12 * D3._21 * D3._33;
	fD3 -= D3._13 * D3._22 * D3._31;

	static float fLength = 0.0f;
	fLength = -1.0f * fD1 / fDet;

	if(fLength < 0.0f)
	{
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_OPPOSITEDIR;
		return FALSE;
	}
	if(fLength < 0.000001f)
	{
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_SAMEFACE;
		return FALSE;
	}

	PtBaryCentric.y = fD2 / fDet;
	PtBaryCentric.z = fD3 / fDet;
	PtBaryCentric.x = 1 - PtBaryCentric.y - PtBaryCentric.z;

	// Invalid, must be internal fatal error
	if(PtBaryCentric.x > 1.0f || PtBaryCentric.x < 0.0f)
	{
		// 射线和面接近平行的话就会出这样的错
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_NO;
		return FALSE;
	}
	if(PtBaryCentric.y > 1.0f || PtBaryCentric.y < 0.0f)
	{
		// 射线和面接近平行的话就会出这样的错
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_NO;
		return FALSE;
	}
	if(PtBaryCentric.z > 1.0f || PtBaryCentric.z < 0.0f)
	{
		// 射线和面接近平行的话就会出这样的错
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_NO;
		return FALSE;
	}

	if(PtBaryCentric.x == 1.0f || PtBaryCentric.y == 1.0f || PtBaryCentric.z == 1.0f)
	{
		// Ray Start Point is the same of one of the face's three points
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_SAMEPOINT;
		return FALSE;
	}


	// Output
	pPtBaryCoord->x = PtBaryCentric.x;
	pPtBaryCoord->y = PtBaryCentric.y;
	pPtBaryCoord->z = PtBaryCentric.z;
	*pLength = fLength;

	if(pPtIntersect)
	{
		pPtIntersect->x = PtStart.x + VecRay.x * fLength;
		pPtIntersect->y = PtStart.y + VecRay.y * fLength;
		pPtIntersect->z = PtStart.z + VecRay.z * fLength;
	}



	if(pIntersectAttribute)
		*pIntersectAttribute = INTERSECT_OK;
	return TRUE;
}


/* 上面函数的原始版
// 射线跟踪求交，三角形A->B->C，绘制顺序必须一致，返回是否相交，同时可以输出交点重心坐标、交点到原始点的长度和笛卡尔坐标（可选）
// 返回的重心坐标x, y, z各自分别代表h, f, g，即分别到A1 A2 A3的长度比例
// 最后一个参数是相交状态，如果在得知是否相交后还想得知具体的相交状态（如是不相交还是背离？相交还是共面？）就可以加上该参数
BOOL GetIntersectTriangle3D(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXVECTOR3 PtA, D3DXVECTOR3 PtB, D3DXVECTOR3 PtC, LPD3DXVECTOR3 pPtBaryCoord, float *pLength, LPD3DXVECTOR3 pPtIntersect, MYRAYTRACING_INTERSECT *pIntersectAttribute)
{

if(!pPtBaryCoord || !pLength)
{
OutputDebugString("必须输入长度和重心坐标的指针，射线相交判定失败！！\n");
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_INVALIDARGUMENTS;
return FALSE;
}

D3DXVec3Normalize(&VecRay, &VecRay);

if(D3DXVec3Length(&VecRay) < 0.00001f)
{
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_INVALIDARGUMENTS;
return FALSE;
}

D3DXVECTOR3 VecLineAB, VecLineAC, VecLineAStart;
VecLineAB = PtB - PtA;
VecLineAC = PtC - PtA;
VecLineAStart = PtStart - PtA;

// Direction Pre-Judgement, when ray and face normal have the opposite direction, shows no intersect at all
// Pre-Judgement could improve performance, but may cause errors
//	D3DXVECTOR3 VecNormal(0, 0, 0), VecLine1, VecLine2;
//	D3DXVec3Normalize(&VecLine1, &VecLineAB);
//	D3DXVec3Normalize(&VecLine2, &VecLineAC);
//	D3DXVec3Cross(&VecNormal, &VecLine1, &VecLine2);
//	D3DXVec3Normalize(&VecNormal, &VecNormal);
//	float fDot = D3DXVec3Dot(&VecNormal, &VecRay);
//	if(fDot < 0.0f)
//	{
//		if(pIntersectAttribute)
//			*pIntersectAttribute = INTERSECT_OPPOSITEDIR;
//		return FALSE;
//	}
//	if(absf(fDot) < 0.001f)		// 接近平行
//	{
//		if(pIntersectAttribute)
//			*pIntersectAttribute = INTERSECT_PARALLEL;
//		return FALSE;
//	}

// Sideline's Length is too small, demonstrate that the triangle face is invalid
if(D3DXVec3Length(&VecLineAB) < 0.00001f || D3DXVec3Length(&VecLineAC) < 0.00001f)
{
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_INVALIDARGUMENTS;
return FALSE;
}
if(absf(D3DXVec3Length(&VecLineAStart)) < 0.00001f)
{
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_SAMEFACE;
return FALSE;
}


// Equation: -Ray*Length + (V2-V1) * f + (V3-V1) * g = Start - V1
D3DXVECTOR3 PtBaryCentric(0, 0, 0);	// f, g, h，h = 1 - f - g
D3DXMATRIX DetCoef
(VecRay.x, VecLineAB.x, VecLineAC.x, 1.0f,
VecRay.y, VecLineAB.y, VecLineAC.y, 1.0f,
VecRay.z, VecLineAB.z, VecLineAC.z, 1.0f,
1.0f, 1.0f, 1.0f, 1.0f);

// Get Determinant
float fDet = GetDeterminant3X3(DetCoef);

// No Intersect
if(absf(fDet) < 0.000001f)
{
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_NO;
return FALSE;
}


if(pPtBaryCoord)
{
D3DXMATRIX D1
(VecLineAStart.x, VecLineAB.x, VecLineAC.x, 1.0f,
VecLineAStart.y, VecLineAB.y, VecLineAC.y, 1.0f,
VecLineAStart.z, VecLineAB.z, VecLineAC.z, 1.0f,
1.0f, 1.0f, 1.0f, 1.0f);
D3DXMATRIX D2
(VecRay.x, VecLineAStart.x, VecLineAC.x, 1.0f,
VecRay.y, VecLineAStart.y, VecLineAC.y, 1.0f,
VecRay.z, VecLineAStart.z, VecLineAC.z, 1.0f,
1.0f, 1.0f, 1.0f, 1.0f);
D3DXMATRIX D3
(VecRay.x, VecLineAB.x, VecLineAStart.x, 1.0f,
VecRay.y, VecLineAB.y, VecLineAStart.y, 1.0f,
VecRay.z, VecLineAB.z, VecLineAStart.z, 1.0f,
1.0f, 1.0f, 1.0f, 1.0f);

float fD1 = GetDeterminant3X3(D1), fD2 = GetDeterminant3X3(D2), fD3 = GetDeterminant3X3(D3);
float fLength = -1.0f * fD1 / fDet;

if(fLength < 0.0f)
{
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_OPPOSITEDIR;
return FALSE;
}
if(fLength < 0.000001f)
{
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_SAMEFACE;
return FALSE;
}

PtBaryCentric.y = fD2 / fDet;
PtBaryCentric.z = fD3 / fDet;
PtBaryCentric.x = 1 - PtBaryCentric.y - PtBaryCentric.z;

// Invalid, must be internal fatal error
if(PtBaryCentric.x > 1.0f || PtBaryCentric.x < 0.0f)
{
// 射线和面接近平行的话就会出这样的错
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_NO;
return FALSE;
}
if(PtBaryCentric.y > 1.0f || PtBaryCentric.y < 0.0f)
{
// 射线和面接近平行的话就会出这样的错
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_NO;
return FALSE;
}
if(PtBaryCentric.z > 1.0f || PtBaryCentric.z < 0.0f)
{
// 射线和面接近平行的话就会出这样的错
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_NO;
return FALSE;
}

if(PtBaryCentric.x == 1.0f || PtBaryCentric.y == 1.0f || PtBaryCentric.z == 1.0f)
{
// Ray Start Point is the same of one of the face's three points
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_SAMEPOINT;
return FALSE;
}


// Output
*pPtBaryCoord = PtBaryCentric;

if(pPtIntersect)
*pPtIntersect = PtStart + VecRay * fLength;

if(pLength)
*pLength = fLength;
}

if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_OK;
return TRUE;
}
*/











/************************************************************************/
/*								Other                                   */
/************************************************************************/
UINT ConvertShaderVersion(const char *pszVersion, bool bPixelShader)
{
	if(pszVersion == NULL)
		return 0;
	// 不符合Shader Version字串规范，返回，暂时支持到SM4.x
	if( pszVersion[1] !='s' || pszVersion[3] > '4' || pszVersion[3] < '1' )
		return 0;

	char pVersion[7] = "";
	
	// 判断VS版本
	if(!bPixelShader)
	{
		// 必须是vs_x_y，且y只能是数字
		if(pszVersion[0] != 'v' || pszVersion[5] > '9' || pszVersion[5] < '0')
			return 0;
		return (pszVersion[3] - 48) * 10 + (pszVersion[5] - 48);
	}
	// 判断PS版本
	else
	{
		if(pszVersion[0] != 'p')
			return 0;

		// 只有ps2.x才能有字母作为副版本号
		UINT iSubVersion = pszVersion[5] - 48;
		if(pszVersion[3] == '2')
		{
			// ps2.x和ps2.0的值一样，所以设备检测较为宽松，以便专门进行PS20Caps检测
			if(pszVersion[5] == 'x')
				iSubVersion = 0;
			else if(pszVersion[5] == 'a')
				iSubVersion = 2;
			else if(pszVersion[5] == 'b')
				iSubVersion = 1;
		}
		else if(pszVersion[5] > '9' || pszVersion[5] < '0')
			return 0;

		return (pszVersion[3] - 48) * 10 + iSubVersion;
	}
}



BOOL CheckPS2xSupport(UINT iNumTemp, BOOL bFlowControl, BOOL bNoLimitDependentRead/* = FALSE */)
{
	char pszVersion[7] = "";
	// 得到当前PS版本
	memcpy(pszVersion, D3DXGetPixelShaderProfile(d3ddevice), 7);

	// 不符合Pixel Shader 2.0 Version字串规范
	if( pszVersion[0] !='p' || pszVersion[1] !='s' || pszVersion[3] < '2' )
		return FALSE;
	
	// 设备支持的主版本高于SM2
	if(pszVersion[3] > '2')
		return TRUE;

	// 设备不支持无限制次数的纹理寻址，不属于ps2.x
	if(!(d3dcaps.PS20Caps.Caps & D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT))
		return FALSE;

	// 设备支持的最大指令数不够（ps2x硬件必须支持512条指令），不属于ps2.x
	if(d3dcaps.PS20Caps.NumInstructionSlots < 512)
		return FALSE;

	// 设备不支持任意交叉混合，但这里不做判断，尽量不要在Shader中使用任意交叉混合，用单条指令代替，硬件会自动优化，不会影响性能的
//	if(!(d3dcaps.PS20Caps.Caps & D3DPS20CAPS_ARBITRARYSWIZZLE))
//		return FALSE;

	// 设备支持的临时寄存器数不够要求的
	if(d3dcaps.PS20Caps.NumTemps < (int)iNumTemp)
		return FALSE;

	if(bFlowControl)
	{
		// 设备不支持分支
		if(!d3dcaps.PS20Caps.StaticFlowControlDepth || !d3dcaps.PS20Caps.DynamicFlowControlDepth)
			return FALSE;
	}

	if(bNoLimitDependentRead)
	{
		// 设备不支持无限制的Dependent-Level寻址
		if(!(d3dcaps.PS20Caps.Caps & D3DPS20CAPS_NODEPENDENTREADLIMIT))
			return FALSE;
	}

	return TRUE;
}




HRESULT SetTexturedRenderTarget(DWORD iMRTNo, LPDIRECT3DTEXTURE9 pRTTex, LPDIRECT3DSURFACE9 pDepthStencil)
{
	if(iMRTNo > 3 || !pRTTex)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	// 先检查要设置的RT是否还处于作为贴图的状态，如果是，就设空
	LPDIRECT3DBASETEXTURE9 pTexInUse = NULL;
	for(UINT i = 0; i < 8; i++)
	{
		HR_RETURN(d3ddevice->GetTexture(i, &pTexInUse));
		if(pTexInUse == (LPDIRECT3DBASETEXTURE9)pRTTex)
			d3ddevice->SetTexture(i, NULL);
		SAFE_RELEASE(pTexInUse);
	}
	for(int i = D3DVERTEXTEXTURESAMPLER0; i < D3DVERTEXTEXTURESAMPLER3; i++)
	{
		HR_RETURN(d3ddevice->GetTexture(i, &pTexInUse));
		if(pTexInUse == (LPDIRECT3DBASETEXTURE9)pRTTex)
			d3ddevice->SetTexture(i, NULL);
		SAFE_RELEASE(pTexInUse);
	}

	// 设置RT
	LPDIRECT3DSURFACE9 pRTSurf = NULL;
	pRTTex->GetSurfaceLevel(0, &pRTSurf);
	HR_RETURN(d3ddevice->SetRenderTarget(iMRTNo, pRTSurf));
	if(pDepthStencil)
		HR_RETURN(d3ddevice->SetDepthStencilSurface(pDepthStencil));
	
	// 结束
	SAFE_RELEASE(pRTSurf);
	return S_OK;
}



HRESULT SetTexturedRenderTarget(DWORD iMRTNo, DWORD iFaceNo, LPDIRECT3DCUBETEXTURE9 pRTTex, LPDIRECT3DSURFACE9 pDepthStencil)
{
	if(iMRTNo > 3 || iFaceNo > 5 || !pRTTex)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	// 先检查要设置的RT是否还处于作为贴图的状态，如果是，就设空
	LPDIRECT3DBASETEXTURE9 pTexInUse = NULL;
	for(UINT i = 0; i < 8; i++)
	{
		HR_RETURN(d3ddevice->GetTexture(i, &pTexInUse));
		if(pTexInUse == (LPDIRECT3DBASETEXTURE9)pRTTex)
			d3ddevice->SetTexture(i, NULL);
		SAFE_RELEASE(pTexInUse);
	}
	for(int i = D3DVERTEXTEXTURESAMPLER0; i < D3DVERTEXTEXTURESAMPLER3; i++)
	{
		HR_RETURN(d3ddevice->GetTexture(i, &pTexInUse));
		if(pTexInUse == (LPDIRECT3DBASETEXTURE9)pRTTex)
			d3ddevice->SetTexture(i, NULL);
		SAFE_RELEASE(pTexInUse);
	}

	// 设置RT
	LPDIRECT3DSURFACE9 pRTSurf = NULL;
	pRTTex->GetCubeMapSurface((_D3DCUBEMAP_FACES)iFaceNo, 0, &pRTSurf);
	HR_RETURN(d3ddevice->SetRenderTarget(iMRTNo, pRTSurf));
	if(pDepthStencil)
		HR_RETURN(d3ddevice->SetDepthStencilSurface(pDepthStencil));

	// 结束
	SAFE_RELEASE(pRTSurf);
	return S_OK;
}





float GetFresnelCoef(float fni, float fno, float fcos)
{
	float fFresnel, g, c, n, fPrev, fNext;
	// cos要强制Clamp到0
	if(fcos < 0.0f)
		fcos = 0.0f;
	n = fni / fno;
	c = fni * fcos / fno;
	g = sqrtf(1 + c * c - c);
	fPrev = (g - c) / (g + c);
	fNext = (c * (g+c) - n*n) / (c * (g-c) + n*n);
	fPrev *= fPrev;
	fNext *= fNext;
	fFresnel = 0.5f * fPrev * (1 + fNext);
	return fFresnel;
}