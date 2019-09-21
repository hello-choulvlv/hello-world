#include <time.h>
#include "myd3d.h"
#include "base.h"

char convstr[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/************************************************************************/
/*						String Operate Functions                        */
/************************************************************************/
void inttostr(unsigned long n)  /*�Ƚ�С������unsigned,������unsigned long�����ܸ��*/
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


void floattostr(int bitnum,double data)    //bitnum������ʾ��С��λ�������9λ������9λС��
{double d=data,ddouble; unsigned long n=(unsigned long)data;

char temp[11]={48,48,48};
unsigned long cfs;
int num,bit=0,bit1=0,a,b;
float bitemp=1;            /*get value of "bit"*/

if(n==0) {convstr[0]='0';convstr[1]='\0';bit++;}    //�����������Ϊ0����ֻ���һ��0���ҵ�ǰλ��ָ���2��'\0'
else {
	for(a=0;a<12&&bitemp>=1;a++){cfs=cf(a);bitemp=(float)n/cfs;bit++;}
	bit--;
	for(b=bit-1;b>=0;b--)
	{cfs=cf(b);
	temp[b]=(int)(n/cfs)+48;
	num=(int)(n/cfs);
	n=n-num*cfs;}
	for(b=bit-1;b>=0;b--) convstr[b]=temp[bit-b-1];
}     //�õ���������

convstr[bit]='.';        //���ڵ�ǰλ��λ�ã���������Ϊ0��λ��Ϊ2����֮λ��Ϊ�������ֵĺ�һ�������һ��С����
if(bitnum==0) {convstr[bit]='\0';return;}      //����Ҫ��С�����֣������
ddouble=d-(unsigned long)data;      //�õ�С������
cfs=cf(bitnum);
ddouble*=cfs;
n=(unsigned long)ddouble;

if(n==0) convstr[bit]='\0';   //��С������Ϊ0�����ַ�������
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
}     //�õ�С������
}


unsigned long strtoint(char *p)
{char *pp=p;unsigned long cfdata,result=0;
int i,j;
for(i=0;*pp!='\0';i++,pp++)if(*pp>57||*pp<48) return(result);  //iΪλ��
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
for(i=0;i<num;i++,temp++) if(!(*(temp))) break;//�˾��Ƿ�ֹnum�����ַ���ʣ�µĳ���
num=i;temp=p+from-1;

for(;*temp;temp++) {*temp=*(temp+num);if(!(*(temp+num))) break;}
}


void insstr(char *dest,int from,char *source)
{int num,curno;char *tmpdst,*tmpsrc=source;
if(!dest||!source||from<0||!(*source)) return;
tmpdst=dest+from;
for(num=0;*(tmpsrc+num);num++);    //�ҵ�Դ�ִ�����
if(!num) return;
for(curno=1;*tmpdst;tmpdst++,curno++);   //�ƶ���Ŀ���ִ��Ľ�����־
for(;curno>=0;tmpdst--,curno--) *(tmpdst+num)=*tmpdst;   //�������ƶ�
tmpdst=dest+from;
for(;*tmpsrc;tmpdst++,tmpsrc++) *tmpdst=*tmpsrc;   //��Դ�ִ�������ȥ
}


char *copystr(char *source,int from,int num)
{int i;char *tmpdst,*tmpsrc;
if(!source||from<1||num<1||!(*source)) return(0);
tmpsrc=source+from-1;
for(i=0;i<num&&*(tmpsrc+i);i++);  num=i;    //ȷ��from+num����source�߽���������
tmpdst=(char *)malloc(i+1);
for(i=0;i<num;i++) *(tmpdst+i)=*(tmpsrc+i);
*(tmpdst+i)=0;
return(tmpdst);
}



void wordtostr(char *word,char *str)      //���ַ�����str��һ���ַ���word
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
		// ������ǰHead-End��һ�Σ���β�ÿգ���pSrcData��Ծ����һ��
		*pSubSectionEnd = 0;
		pSrcData = pSubSectionEnd + 1;

		strcpy(pBuf, pSubSectionHead);
		*pCurrDataWrite++ = (float)atof(pBuf);

		i++;
		pSubSectionHead = pSrcData;
	}
	// ����ѭ���Ժ�˵��û��,�ˣ���ʣ�����һ��float����Ҫ��������
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
	// ������Ա�2�������ͼ����������2���ݣ����ض�����ֻʣ��1
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
		// �ȵõ���ǰ����λ��ԭʼ���ݣ����Ƶ����λ���ã�Ϊ�˱��ں��桰�򡱣�����λ����0
		iBitValue = (iValue >> i) & 0x1;
		// ��ԭʼλ�����Ƶ�ָ���ص㣨λ����
		iResult |= iBitValue << (iBitNum-1 - i);
	}
	// ��λ���ֲ���
	for(int i = iBitNum; i < 32; i++)
	{
		// �ȵõ���ǰ����λ��ԭʼ���ݣ����Ƶ����λ���ã�Ϊ�˱��ں��桰�򡱣�����λ����0
		iBitValue = (iValue >> i) & 0x1;
		// ��ԭʼλ�����Ƶ�ָ���ص㣨λ����
		iResult |= iBitValue << i;
	}


	return iResult;
}



// clamp���������������ʾclamp�ķ�Χ
float clampf(float fValue, float fStandardmin /* = 0.0f */, float fStandardmax /* = 0.0f */)
{
	// �Ƿ�
	if(fStandardmax < fStandardmin)
		return fValue;

	if(fValue < fStandardmin)
		fValue = fStandardmin;

	if(fValue > fStandardmax)
		fValue = fStandardmax;

	// ��ֹfValue��fStandard�������ʱ������ıȽϻ�ʧ�ܣ������������������
	if(absf(fValue - fStandardmin) < 0.00001f)
		fValue = fStandardmin;
	if(absf(fValue - fStandardmax) < 0.00001f)
		fValue = fStandardmax;

	return fValue;
}

// �������룬���һ��������ʾ��������ķֽ磬������0��1֮��
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

// ����׳�
int CalcFactorial(int iData)
{
	if(iData == 0)
		return 1;
	int iResult = 1;
	for(; iData > 0; iData--)
		iResult *= iData;
	return iResult;
}
// ˫�׳�
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


// ֻѡǰ3x3�Ĳ�����Det
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

// ������������������ź��Ǿ��ȷ�Χ����ʾС��������λ������̫�ߣ������������������ȷ��
// ������������ĺ����У���������Ϊ��������ô���ص�Ҳȫ���Ǹ�������0��
// ����С��fRandomValue�����������Ϊ�������ص�ҲȫΪ������0
float randomf(float fRandomValue, UINT iBitNum/* = 4 */)
{
	// ����ԭ���ǽ����������󣨳�10�ķ������ٵ��������Ȼ������С��ԭ��Χ
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

	// �ȼ���ǲ���0
	if(fValue == 0)
		return 0.0f;


	// ��������λ
	iRetValue = fValue >> 15;
	iRetValue = iRetValue << 31;

	// ת��ָ��λ
	iExpValue = (unsigned char)(fValue >> 10) & 0x1f;
	iExpValue -= 15;	// �õ�������ָ��
	iTemp = iExpValue + 127;	// �����float��ָ���洢
	iTemp &= 0xff;
	iTemp = iTemp << 23;
	iRetValue |= iTemp;

	// �ض�β��λ
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

	// �ȼ���ǲ���0
	if(iValue == 0)
		return 0;

	// ��������λ
	fRetValue = iValue >> 31;
	fRetValue = fRetValue << 15;
	if(fRetValue)
		OutputDebugString("���棺FP16����Ϊ������\n");

	// ת��ָ��λ��С��Խ�磡��
	iExpValue = (iValue >> 23) & 0xff;
	iExpValue -= 127;	// �õ�������ָ��
	signed char fTestExp = (signed)iExpValue;
	if(fTestExp < -15)
	{
		OutputDebugString("���棺ת��FP16����Խ�磡ԭfloat��̫С��ǿ����0��\n");
		return 0;
	}
	if(fTestExp > 15)
	{
		OutputDebugString("����ת��FP16����Խ�磡ԭfloat��̫��\n");
		return 0;
	}

	fTemp = iExpValue + 15;	// �����FP16��ָ��
	fTemp &= 0x1f;
	fTemp = fTemp << 10;
	fRetValue |= fTemp;

	// �ض�β��λ
	iTempLong = iValue & 0x7fffff;
	fTemp = iTempLong >> 13;	// fp16��float��13λ
	fRetValue |= fTemp;

	return fRetValue;
}









/************************************************************************/
/*				Space Coordinates Matrix Points and Vectors             */
/************************************************************************/

// ����������ת�������õ�ŷ�������ŷ����
D3DXMATRIX XZXEulerRotation(LPD3DXMATRIX pMat, D3DXVECTOR3 VecAxis, float fAngle, LPD3DXVECTOR3 pVecEulerAngle/* = NULL*/)
{
	D3DXMATRIX MatRotationCombine;
	D3DXMATRIX MatZ1, MatY1, MatZ2;
	D3DXVECTOR3 X(1,0,0), Y(0,1,0), Z(0,0,1);
	D3DXVECTOR3 x(1,0,0), y(0,1,0), z(0,0,1);
	D3DXVECTOR3 x1, x2, x3, y1, y2, y3, z1, z2, z3;

	// ���ݲ���������ת������ľ���
	D3DXMatrixRotationAxis(&MatRotationCombine, &VecAxis, fAngle);

	// ��ת������
	D3DXVec3TransformCoord(&x, &X, &MatRotationCombine);
	D3DXVec3TransformCoord(&y, &Y, &MatRotationCombine);
	D3DXVec3TransformCoord(&z, &Z, &MatRotationCombine);

	D3DXVec3Normalize(&x, &x);
	D3DXVec3Normalize(&y, &y);
	D3DXVec3Normalize(&z, &z);


	D3DXVECTOR3 VecTemp(0, 0, 0);
	D3DXVECTOR3 VecCross(0, 0, 0);

	// �õ�Alpha
	// �ȵõ�x��YZ���ͶӰ���ܼ�Ŷ��ȡYZ�����ͺ�
	D3DXVec3Normalize(&VecTemp, &D3DXVECTOR3(0, x.y, x.z));
	// �õ�ͶӰ������Y�ļнǣ�ͬ���ģ�Ҫ�ж���ת����
	float fAlpha = acosf( D3DXVec3Dot(&VecTemp, &Y) );
	D3DXVec3Cross(&VecCross, &Y, &VecTemp);	// Y��ͶӰ������ת�����ǰ��˳��Ҫע��
	if( D3DXVec3Dot(&VecCross, &X) < 0 )	// ����ת���cosС��0˵���Ƿ�����
		fAlpha = -fAlpha;

	// ������ת����
	D3DXMatrixRotationAxis(&MatZ1, &X, fAlpha);
	D3DXVec3TransformCoord(&x1, &X, &MatZ1);
	D3DXVec3TransformCoord(&y1, &Y, &MatZ1);
	D3DXVec3TransformCoord(&z1, &Z, &MatZ1);
	D3DXVec3Normalize(&x1, &x1);
	D3DXVec3Normalize(&y1, &y1);
	D3DXVec3Normalize(&z1, &z1);



	// �õ�Beta������������ϵ�У�Ϊ���ж���ת������Ҫ��һ�����
	float fBeta = acosf( D3DXVec3Dot(&x, &X) );
	D3DXVec3Cross(&VecCross, &X, &x);	// Z��z��ת�����ǰ��˳��Ҫע��
	if( D3DXVec3Dot(&VecCross, &z1) < 0 )	// ����ת���cosС��0˵���Ƿ�����
		fBeta = -fBeta;

	// ������ת����
	D3DXMatrixRotationAxis(&MatY1, &z1, fBeta);
	D3DXVec3TransformCoord(&x2, &x1, &MatY1);
	D3DXVec3TransformCoord(&y2, &y1, &MatY1);
	D3DXVec3TransformCoord(&z2, &z1, &MatY1);
	D3DXVec3Normalize(&x2, &x2);
	D3DXVec3Normalize(&y2, &y2);
	D3DXVec3Normalize(&z2, &z2);


	// �õ�Gamma
	// �ȵõ����ߣ���Y��Zת��Alpha
	float fGamma = acosf( D3DXVec3Dot(&z1, &z) );
	D3DXVec3Cross(&VecCross, &z1, &z);		// ������y��ת�����ǰ��˳��Ҫע��
	if( D3DXVec3Dot(&VecCross, &x2) < 0 )	// ����ת���cosС��0˵���Ƿ�����
		fGamma = -fGamma;

	D3DXMatrixRotationAxis(&MatZ2, &x2, fGamma);

	// ��Ͼ���
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

	// ���ݲ���������ת������ľ���
	D3DXMatrixRotationAxis(&MatRotationCombine, &VecAxis, fAngle);

	// ��ת������
	D3DXVec3TransformCoord(&x, &X, &MatRotationCombine);
	D3DXVec3TransformCoord(&y, &Y, &MatRotationCombine);
	D3DXVec3TransformCoord(&z, &Z, &MatRotationCombine);

	D3DXVec3Normalize(&x, &x);
	D3DXVec3Normalize(&y, &y);
	D3DXVec3Normalize(&z, &z);


	D3DXVECTOR3 VecTemp(0, 0, 0);
	D3DXVECTOR3 VecCross(0, 0, 0);

	// �õ�Alpha
	// �ȵõ�z��XY���ͶӰ���ܼ�Ŷ��ȡXY�����ͺ�
	D3DXVec3Normalize(&VecTemp, &D3DXVECTOR3(z.x, z.y, 0));
	// �õ�ͶӰ������X�ļнǣ�ͬ���ģ�Ҫ�ж���ת����
	float fAlpha = acosf( D3DXVec3Dot(&VecTemp, &X) );
	D3DXVec3Cross(&VecCross, &X, &VecTemp);	// X��ͶӰ������ת�����ǰ��˳��Ҫע��
	if( D3DXVec3Dot(&VecCross, &Z) < 0 )	// ����ת���cosС��0˵���Ƿ�����
		fAlpha = -fAlpha;

	// ������ת����
	D3DXMatrixRotationAxis(&MatZ1, &Z, fAlpha);
	D3DXVec3TransformCoord(&x1, &X, &MatZ1);
	D3DXVec3TransformCoord(&y1, &Y, &MatZ1);
	D3DXVec3TransformCoord(&z1, &Z, &MatZ1);
	D3DXVec3Normalize(&x1, &x1);
	D3DXVec3Normalize(&y1, &y1);
	D3DXVec3Normalize(&z1, &z1);



	// �õ�Beta������������ϵ�У�Ϊ���ж���ת������Ҫ��һ�����
	float fBeta = acosf( D3DXVec3Dot(&z, &Z) );
	D3DXVec3Cross(&VecCross, &Z, &z);	// Z��z��ת�����ǰ��˳��Ҫע��
	if( D3DXVec3Dot(&VecCross, &y1) < 0 )	// ����ת���cosС��0˵���Ƿ�����
		fBeta = -fBeta;

	// ������ת����
	D3DXMatrixRotationAxis(&MatY1, &y1, fBeta);
	D3DXVec3TransformCoord(&x2, &x1, &MatY1);
	D3DXVec3TransformCoord(&y2, &y1, &MatY1);
	D3DXVec3TransformCoord(&z2, &z1, &MatY1);
	D3DXVec3Normalize(&x2, &x2);
	D3DXVec3Normalize(&y2, &y2);
	D3DXVec3Normalize(&z2, &z2);


	// �õ�Gamma
	// �ȵõ����ߣ���Y��Zת��Alpha
	float fGamma = acosf( D3DXVec3Dot(&y1, &y) );
	D3DXVec3Cross(&VecCross, &y1, &y);		// ������y��ת�����ǰ��˳��Ҫע��
	if( D3DXVec3Dot(&VecCross, &z2) < 0 )	// ����ת���cosС��0˵���Ƿ�����
		fGamma = -fGamma;

	D3DXMatrixRotationAxis(&MatZ2, &z2, fGamma);

	// ��Ͼ���
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


// ���أ�������Ԫ�����õ�ŷ�������ŷ����
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



// ����ŷ���ǵõ�ŷ������
D3DXMATRIX GetZYZEulerMatrix(LPD3DXMATRIX pMat, D3DXVECTOR3 VecEulerAngle)
{
	D3DXMATRIX MatRotationCombine;
	D3DXMATRIX MatZ1, MatY1, MatZ2;
	D3DXVECTOR3 X(1,0,0), Y(0,1,0), Z(0,0,1);
	D3DXVECTOR3 x1, x2, x3, y1, y2, y3, z1, z2, z3;

	D3DXMatrixIdentity(&MatRotationCombine);


	// ����Alpha������ת����
	D3DXMatrixRotationAxis(&MatZ1, &Z, VecEulerAngle.x);
	D3DXVec3TransformCoord(&x1, &X, &MatZ1);
	D3DXVec3TransformCoord(&y1, &Y, &MatZ1);
	D3DXVec3TransformCoord(&z1, &Z, &MatZ1);
	D3DXVec3Normalize(&x1, &x1);
	D3DXVec3Normalize(&y1, &y1);
	D3DXVec3Normalize(&z1, &z1);



	// ����Beta������ת����
	D3DXMatrixRotationAxis(&MatY1, &y1, VecEulerAngle.y);
	D3DXVec3TransformCoord(&x2, &x1, &MatY1);
	D3DXVec3TransformCoord(&y2, &y1, &MatY1);
	D3DXVec3TransformCoord(&z2, &z1, &MatY1);
	D3DXVec3Normalize(&x2, &x2);
	D3DXVec3Normalize(&y2, &y2);
	D3DXVec3Normalize(&z2, &z2);


	// ����Gamma�õ���ת����
	D3DXMatrixRotationAxis(&MatZ2, &z2, VecEulerAngle.z);

	// ��Ͼ���
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


	// ����Alpha������ת����
	D3DXMatrixRotationAxis(&MatZ1, &X, VecEulerAngle.x);
	D3DXVec3TransformCoord(&x1, &X, &MatZ1);
	D3DXVec3TransformCoord(&y1, &Y, &MatZ1);
	D3DXVec3TransformCoord(&z1, &Z, &MatZ1);
	D3DXVec3Normalize(&x1, &x1);
	D3DXVec3Normalize(&y1, &y1);
	D3DXVec3Normalize(&z1, &z1);



	// ����Beta������ת����
	D3DXMatrixRotationAxis(&MatY1, &z1, VecEulerAngle.y);
	D3DXVec3TransformCoord(&x2, &x1, &MatY1);
	D3DXVec3TransformCoord(&y2, &y1, &MatY1);
	D3DXVec3TransformCoord(&z2, &z1, &MatY1);
	D3DXVec3Normalize(&x2, &x2);
	D3DXVec3Normalize(&y2, &y2);
	D3DXVec3Normalize(&z2, &z2);


	// ����Gamma�õ���ת����
	D3DXMatrixRotationAxis(&MatZ2, &x2, VecEulerAngle.z);

	// ��Ͼ���
	MatRotationCombine = MatZ1 * MatY1 * MatZ2;

	if(pMat)
	{
		*pMat = MatRotationCombine;
	}
	return MatRotationCombine;
}



// �ѿ�������������Ļ���ת��������������VECTOR2(Theta, Phi)����һ���Ǻʹ�ֱ�ᣨZ���ļнǣ��ڶ������ڵ��棨XZ����ͶӰ�ͺ��ᣨX���ļн�
D3DXVECTOR2 CartesianToSpherical(D3DXVECTOR3 VecCarte, LPD3DXVECTOR2 pVecSphere)
{
	D3DXVECTOR2 VecSphere;
	D3DXVec3Normalize(&VecCarte, &VecCarte);

	// ��ʹ�ֱ��ļн�
	VecSphere.x = acosf(VecCarte.z);

	// ��ͶӰ�ͺ���ļн�
	// atan������y�������ϵ�������ǿ�Ƹ�ֵ
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

		// �ڶ�������������
		if(VecCarte.x < 0.0f)
		{		
			VecSphere.y = D3DX_PI + VecSphere.y;
		}
		// ������������
		if(VecCarte.y < 0.0f && VecCarte.x >= 0.0f)
		{
			VecSphere.y = D3DX_PI * 2 + VecSphere.y;
		}
	}

	*pVecSphere = VecSphere;
	return VecSphere;
}

// ����������VECTOR2(Theta, Phi)����һ���Ǻʹ�ֱ�ᣨZ���ļнǣ��ڶ������ڵ��棨XZ����ͶӰ�ͺ��ᣨX���ļн�
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


// ��������������
HRESULT GenerateViewSpace(D3DXVECTOR3 *pVecLookAt_Z, D3DXVECTOR3 *pVecHead_Y, D3DXVECTOR3 *pVecRight_X)
{
	if(!pVecLookAt_Z || !pVecHead_Y || !pVecRight_X)
		return D3DERR_INVALIDCALL;
	if(D3DXVec3Length(pVecLookAt_Z) == 0 || D3DXVec3Length(pVecHead_Y) == 0 || *pVecHead_Y == *pVecLookAt_Z)
		return D3DERR_NOTAVAILABLE;

	D3DXVec3Normalize(pVecLookAt_Z, pVecLookAt_Z);
	D3DXVec3Normalize(pVecHead_Y, pVecHead_Y);

	// ��ZY��Ϊ���������淨�ߣ���X�ᣩ
	D3DXVec3Cross(pVecRight_X, pVecHead_Y, pVecLookAt_Z);
	D3DXVec3Normalize(pVecRight_X, pVecRight_X);

	// Y��һ����Z��������Xһ����Z��������ΪZ�������������ܱ䣬����������Y����������ͷ������
	D3DXVec3Cross(pVecHead_Y, pVecLookAt_Z, pVecRight_X);
	D3DXVec3Normalize(pVecHead_Y, pVecHead_Y);

	return S_OK;
}








/************************************************************************/
/*							Get Intersect                               */
/************************************************************************/

bool GetIntersect2D(D3DXVECTOR3 *P, D3DXVECTOR3 P1, D3DXVECTOR3 P2, D3DXVECTOR3 V1, D3DXVECTOR3 V2)
{
	// �ȼ������ֱ���Ƿ�ƽ�У�ƽ�оͷ��ش���ֵ����ʾ�޽���
	D3DXVECTOR3 VecP, VecV;
	VecP = P2 - P1;
	VecV = V2 - V1;
	D3DXVec3Normalize(&VecP, &VecP);
	D3DXVec3Normalize(&VecV, &VecV);
	float fFov = absf(D3DXVec3Dot(&VecP, &VecV));	// ע�ⲻҪ����absһ�£���Ϊdot���ܻ���ָ�ֵ
	if(absf(1.0f - fFov) < 0.00001f)
		return false;


	// ����Ƿ��д�ֱ�ģ���Ϊ���ߵķ��̲���y = kx + b��ʽ
	bool bPVertical = false, bVVertical = false;
	if(absf(P2.x - P1.x) < 0.0001f)
		bPVertical = true;
	if(absf(V2.x - V1.x) < 0.0001f)
		bVVertical = true;


	// ��������ֱ�ߵķ���
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


	// ���㽻��
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
	// ����������ֱ�߶���ֱ���������ֱ���ͻ�����ķ��أ�Dot = 1��
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
/*	// Ҳ���Ը�����������ϵ��ȷ��������A1 + f * (A2-A1) + g * (A3-A1) = P�� h = 1 - f - g
	float f, g, h;	// ϵ��
	D3DXVECTOR3 temp = (P - A1) / ( (P-A1)/(P-A2) * (A2-A1) + A3-A1 );
	return true;
*/
	// �������ߵĹ������
	D3DXVECTOR3 A1 = PtV2 - PtV1;	// V1->V2
	D3DXVECTOR3 A2 = PtV3 - PtV1;	// V1->V3
	D3DXVECTOR3 A3 = PtV2 - PtV3;	// V3->V2
	D3DXVec3Normalize(&A1, &A1);
	D3DXVec3Normalize(&A2, &A2);
	D3DXVec3Normalize(&A3, &A3);

	// �����ε������н�
	float fFov[3], fTemp[2];
	fFov[0] = acosf(D3DXVec3Dot(&A1, &A2));	// V1V2��V1V3�ļн�
	fFov[1] = acosf(D3DXVec3Dot(&(-A1), &(-A3)));	// V1V2��V2V3�ļн�
	fFov[2] = acosf(D3DXVec3Dot(&(-A2), &A3));	// V1V3��V2V3�ļн�
	
	// ���V1��P
	D3DXVECTOR3 VecP = P - PtV1;
	D3DXVec3Normalize(&VecP, &VecP);
	fTemp[0] = acosf(D3DXVec3Dot(&VecP, &A1));
	fTemp[1] = acosf(D3DXVec3Dot(&VecP, &A2));
	if(fTemp[0] >= fFov[0] || fTemp[1] >= fFov[0])
		return false;

	// ���V2��P
	VecP = P - PtV2;
	D3DXVec3Normalize(&VecP, &VecP);
	fTemp[0] = acosf(D3DXVec3Dot(&VecP, &(-A1)));
	fTemp[1] = acosf(D3DXVec3Dot(&VecP, &(-A3)));
	if(fTemp[0] >= fFov[1] || fTemp[1] >= fFov[1])
		return false;

	// ���V3��P
	VecP = P - PtV3;
	D3DXVec3Normalize(&VecP, &VecP);
	fTemp[0] = acosf(D3DXVec3Dot(&VecP, &(-A2)));
	fTemp[1] = acosf(D3DXVec3Dot(&VecP, &A3));
	if(fTemp[0] >= fFov[2] || fTemp[1] >= fFov[2])
		return false;

	return true;
}





// ���߸����󽻣��õ���ƽ��Ľ���
BOOL GetIntersectPlane(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXPLANE Plane, LPD3DXVECTOR3 pPtIntersect)
{
	D3DXVECTOR3 VecNormal = D3DXVECTOR3(Plane.a, Plane.b, Plane.c);
	D3DXVec3Normalize(&VecNormal, &VecNormal);

	// ��ƽ��ӽ�ƽ�л��룬���ཻ
	if(D3DXVec3Dot(&VecRay, &(-1*VecNormal)) < 0.00001f)
		return FALSE;
	// ƽ���������ȷ������ʧ��
	if(D3DXVec3Length(&VecNormal) == 0.0f)
		return FALSE;


	// �����ߵ���ƽ������ƽ��ĵڶ���ƽ�棬��Normal dot Point + d = 0
	float fPlane2_D = -1.0f * D3DXVec3Dot(&VecNormal, &PtStart);

	// ��㵽ƽ��ľ���͵㵽ƽ�������
	float fDistance = absf(Plane.d - fPlane2_D);
	D3DXVECTOR3 PtOnPlane = PtStart - VecNormal*fDistance;
	D3DXVECTOR3 VecPerpendicular = PtOnPlane - PtStart;
	D3DXVec3Normalize(&VecPerpendicular, &VecPerpendicular);

	// �ٶ��ཻ�㵽���ľ���ΪD����ôfDistance / d = cos(���ߺ����߼н�)�����ߺͷ�����һ����
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

	// �ȸ��ݶ���λ�ú����߷��������
	D3DXVECTOR3 VecRotationAxis(0, 0, 0);
	float fDot = 0.0f;
	D3DXMATRIX Mat, MatTranslation;

	// �ȹ���ƽ�ƾ���
	D3DXMatrixTranslation(&MatTranslation, -PtStart.x, -PtStart.y, -PtStart.z);
	// ���ҵ�Ray��Z��ļнǣ�����غϣ�cos = 1����ֱ��������ת
	fDot = D3DXVec3Dot(&VecRay, &D3DXVECTOR3(0, 0, 1));
	if(fDot > 0.9999999f)
		Mat = MatTranslation;
	else
	{
		// �ٸ���Z�������߷����ҵ���ת�ᣬע���˷���RotationAxis����������ʱ����ת��
		D3DXVec3Cross(&VecRotationAxis, &VecRay, &D3DXVECTOR3(0, 0, 1));
		D3DXMatrixRotationAxis(&Mat, &VecRotationAxis, acosf(fDot));

		// ��ƽ�ƣ�����ת
		Mat = MatTranslation * Mat;
	}

	// �任BOX����
	D3DXVECTOR3 PtMaxTrans(0, 0, 0), PtMinTrans(0, 0, 0);
	D3DXVec3TransformCoord(&PtMaxTrans, &PtMax, &Mat);
	D3DXVec3TransformCoord(&PtMinTrans, &PtMin, &Mat);

	// �ж��󽻣�X��Yͬ��˵����һ�࣬���ཻ������0˵�����߶�����Ҳ���ཻ
	if( (PtMaxTrans.x * PtMinTrans.x) > 0.0f || (PtMaxTrans.y * PtMinTrans.y) > 0.0f )
		bIntersect = FALSE;
	else
		bIntersect = TRUE;

	// �����������������ཻ�����õ��ཻ��������ཻ����

	return bIntersect;
}



// ���߸����󽻣�������A->B->C������˳�����һ�£������Ƿ��ཻ��ͬʱ������������������ꡢ���㵽ԭʼ��ĳ��Ⱥ͵ѿ������꣨��ѡ��
// ���ص���������x, y, z���Էֱ����h, f, g�����ֱ�A1 A2 A3�ĳ��ȱ���
// ���һ���������ཻ״̬������ڵ�֪�Ƿ��ཻ�����֪������ཻ״̬�����ǲ��ཻ���Ǳ��룿�ཻ���ǹ��棿���Ϳ��Լ��ϸò���
// ע������һ������Ƶ�ʼ���ߵĺ�����Ϊ�����٣����治�ܵ����κκ����������캯�����������ڲ��ֶ����㣬�������еı�����������static
BOOL GetIntersectTriangle3D(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXVECTOR3 PtA, D3DXVECTOR3 PtB, D3DXVECTOR3 PtC, LPD3DXVECTOR3 pPtBaryCoord, float *pLength, LPD3DXVECTOR3 pPtIntersect/* = NULL */, MYRAYTRACING_INTERSECT *pIntersectAttribute/* = NULL */)
{

	if(!pPtBaryCoord || !pLength)
	{
		OutputDebugString("�������볤�Ⱥ����������ָ�룬�����ཻ�ж�ʧ�ܣ���\n");
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_INVALIDARGUMENTS;
		return FALSE;
	}
	/*Ϊ�����٣����������жϣ����Ա��뱣֤Ray�Ѿ���񻯶��Ҳ�������������
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
	//	if(absf(fDot) < 0.001f)		// �ӽ�ƽ��
	//	{
	//		if(pIntersectAttribute)
	//			*pIntersectAttribute = INTERSECT_PARALLEL;
	//		return FALSE;
	//	}


	/*Ϊ����������ȥ���жϣ����Ա���ȷ�����������������Ч��
	// Sideline's Length is too small, demonstrate that the triangle face is invalid
	if(D3DXVec3Length(&VecLineAB) < 0.00001f || D3DXVec3Length(&VecLineAC) < 0.00001f)
	{
	if(pIntersectAttribute)
	*pIntersectAttribute = INTERSECT_INVALIDARGUMENTS;
	return FALSE;
	}
	// ͬ��Ϊ�����٣�ȥ�����������ȷ�����ߵ㲻�ܺ�������������غϣ�
	if(absf(D3DXVec3Length(&VecLineAStart)) < 0.00001f)
	{
	if(pIntersectAttribute)
	*pIntersectAttribute = INTERSECT_SAMEPOINT;
	return FALSE;
	}
	*/

	// Equation: -Ray*Length + (V2-V1) * f + (V3-V1) * g = Start - V1
	static D3DXVECTOR3 PtBaryCentric(0, 0, 0);	// f, g, h��h = 1 - f - g
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
		// ���ߺ���ӽ�ƽ�еĻ��ͻ�������Ĵ�
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_NO;
		return FALSE;
	}
	if(PtBaryCentric.y > 1.0f || PtBaryCentric.y < 0.0f)
	{
		// ���ߺ���ӽ�ƽ�еĻ��ͻ�������Ĵ�
		if(pIntersectAttribute)
			*pIntersectAttribute = INTERSECT_NO;
		return FALSE;
	}
	if(PtBaryCentric.z > 1.0f || PtBaryCentric.z < 0.0f)
	{
		// ���ߺ���ӽ�ƽ�еĻ��ͻ�������Ĵ�
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


/* ���溯����ԭʼ��
// ���߸����󽻣�������A->B->C������˳�����һ�£������Ƿ��ཻ��ͬʱ������������������ꡢ���㵽ԭʼ��ĳ��Ⱥ͵ѿ������꣨��ѡ��
// ���ص���������x, y, z���Էֱ����h, f, g�����ֱ�A1 A2 A3�ĳ��ȱ���
// ���һ���������ཻ״̬������ڵ�֪�Ƿ��ཻ�����֪������ཻ״̬�����ǲ��ཻ���Ǳ��룿�ཻ���ǹ��棿���Ϳ��Լ��ϸò���
BOOL GetIntersectTriangle3D(D3DXVECTOR3 PtStart, D3DXVECTOR3 VecRay, D3DXVECTOR3 PtA, D3DXVECTOR3 PtB, D3DXVECTOR3 PtC, LPD3DXVECTOR3 pPtBaryCoord, float *pLength, LPD3DXVECTOR3 pPtIntersect, MYRAYTRACING_INTERSECT *pIntersectAttribute)
{

if(!pPtBaryCoord || !pLength)
{
OutputDebugString("�������볤�Ⱥ����������ָ�룬�����ཻ�ж�ʧ�ܣ���\n");
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
//	if(absf(fDot) < 0.001f)		// �ӽ�ƽ��
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
D3DXVECTOR3 PtBaryCentric(0, 0, 0);	// f, g, h��h = 1 - f - g
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
// ���ߺ���ӽ�ƽ�еĻ��ͻ�������Ĵ�
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_NO;
return FALSE;
}
if(PtBaryCentric.y > 1.0f || PtBaryCentric.y < 0.0f)
{
// ���ߺ���ӽ�ƽ�еĻ��ͻ�������Ĵ�
if(pIntersectAttribute)
*pIntersectAttribute = INTERSECT_NO;
return FALSE;
}
if(PtBaryCentric.z > 1.0f || PtBaryCentric.z < 0.0f)
{
// ���ߺ���ӽ�ƽ�еĻ��ͻ�������Ĵ�
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
	// ������Shader Version�ִ��淶�����أ���ʱ֧�ֵ�SM4.x
	if( pszVersion[1] !='s' || pszVersion[3] > '4' || pszVersion[3] < '1' )
		return 0;

	char pVersion[7] = "";
	
	// �ж�VS�汾
	if(!bPixelShader)
	{
		// ������vs_x_y����yֻ��������
		if(pszVersion[0] != 'v' || pszVersion[5] > '9' || pszVersion[5] < '0')
			return 0;
		return (pszVersion[3] - 48) * 10 + (pszVersion[5] - 48);
	}
	// �ж�PS�汾
	else
	{
		if(pszVersion[0] != 'p')
			return 0;

		// ֻ��ps2.x��������ĸ��Ϊ���汾��
		UINT iSubVersion = pszVersion[5] - 48;
		if(pszVersion[3] == '2')
		{
			// ps2.x��ps2.0��ֵһ���������豸����Ϊ���ɣ��Ա�ר�Ž���PS20Caps���
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
	// �õ���ǰPS�汾
	memcpy(pszVersion, D3DXGetPixelShaderProfile(d3ddevice), 7);

	// ������Pixel Shader 2.0 Version�ִ��淶
	if( pszVersion[0] !='p' || pszVersion[1] !='s' || pszVersion[3] < '2' )
		return FALSE;
	
	// �豸֧�ֵ����汾����SM2
	if(pszVersion[3] > '2')
		return TRUE;

	// �豸��֧�������ƴ���������Ѱַ��������ps2.x
	if(!(d3dcaps.PS20Caps.Caps & D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT))
		return FALSE;

	// �豸֧�ֵ����ָ����������ps2xӲ������֧��512��ָ���������ps2.x
	if(d3dcaps.PS20Caps.NumInstructionSlots < 512)
		return FALSE;

	// �豸��֧�����⽻���ϣ������ﲻ���жϣ�������Ҫ��Shader��ʹ�����⽻���ϣ��õ���ָ����棬Ӳ�����Զ��Ż�������Ӱ�����ܵ�
//	if(!(d3dcaps.PS20Caps.Caps & D3DPS20CAPS_ARBITRARYSWIZZLE))
//		return FALSE;

	// �豸֧�ֵ���ʱ�Ĵ���������Ҫ���
	if(d3dcaps.PS20Caps.NumTemps < (int)iNumTemp)
		return FALSE;

	if(bFlowControl)
	{
		// �豸��֧�ַ�֧
		if(!d3dcaps.PS20Caps.StaticFlowControlDepth || !d3dcaps.PS20Caps.DynamicFlowControlDepth)
			return FALSE;
	}

	if(bNoLimitDependentRead)
	{
		// �豸��֧�������Ƶ�Dependent-LevelѰַ
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
	// �ȼ��Ҫ���õ�RT�Ƿ񻹴�����Ϊ��ͼ��״̬������ǣ������
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

	// ����RT
	LPDIRECT3DSURFACE9 pRTSurf = NULL;
	pRTTex->GetSurfaceLevel(0, &pRTSurf);
	HR_RETURN(d3ddevice->SetRenderTarget(iMRTNo, pRTSurf));
	if(pDepthStencil)
		HR_RETURN(d3ddevice->SetDepthStencilSurface(pDepthStencil));
	
	// ����
	SAFE_RELEASE(pRTSurf);
	return S_OK;
}



HRESULT SetTexturedRenderTarget(DWORD iMRTNo, DWORD iFaceNo, LPDIRECT3DCUBETEXTURE9 pRTTex, LPDIRECT3DSURFACE9 pDepthStencil)
{
	if(iMRTNo > 3 || iFaceNo > 5 || !pRTTex)
		return D3DERR_INVALIDCALL;

	HRESULT hr = S_OK;
	// �ȼ��Ҫ���õ�RT�Ƿ񻹴�����Ϊ��ͼ��״̬������ǣ������
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

	// ����RT
	LPDIRECT3DSURFACE9 pRTSurf = NULL;
	pRTTex->GetCubeMapSurface((_D3DCUBEMAP_FACES)iFaceNo, 0, &pRTSurf);
	HR_RETURN(d3ddevice->SetRenderTarget(iMRTNo, pRTSurf));
	if(pDepthStencil)
		HR_RETURN(d3ddevice->SetDepthStencilSurface(pDepthStencil));

	// ����
	SAFE_RELEASE(pRTSurf);
	return S_OK;
}





float GetFresnelCoef(float fni, float fno, float fcos)
{
	float fFresnel, g, c, n, fPrev, fNext;
	// cosҪǿ��Clamp��0
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