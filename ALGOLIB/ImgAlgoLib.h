#include "comlib.h"

//图像直方图均衡
ALGOLIB_API BOOL Func_HistGramEqulizeEnhance(
	double * pfHist,			//输入频率直方图数据指针（概率密度，也可以是像素个数直方图)(大小256）
	unsigned char* pucLUT,		//输出灰度查找表，输出LUT数据指针（大小256)
	bool stateEdgeValue = true	//是否统计边缘值，即【0，255】灰度值，默认为统计
	);

//直方图规定核心代码,GML组映射方法
ALGOLIB_API BOOL standardlizeHistGML(
	double* pStandard,			//规定化标准直方图
	double* pSrc,				//原直方图
	BYTE* pOutMap,				//输出直方图映射关系，pOutMap[i]为输出值
	int grayScale = 256			//灰阶等级，可为8，64，256，512，1024
	);
//直方图规定化核心代码，SML单映射方法
ALGOLIB_API BOOL standardlizeHist(
	double* pStandard,			//规定化标准直方图
	double* pSrc,				//原直方图
	BYTE* pOutMap				//输出直方图映射关系，pOutMap[i]为输出值
	);

class ALGOLIB_API Filter_IMG
{
public:
	Filter_IMG();
	virtual ~ Filter_IMG();

public:
	//输入一维数组，返回数据中值
	static int MedianF(int* pData, int length);

};
