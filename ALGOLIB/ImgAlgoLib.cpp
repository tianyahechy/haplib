#include "include_global.h"
#include "ImgAlgoLib.h"
#include <math.h>

BOOL Func_HistGramEqulizeEnhance(
	double * pfHist,			//输入频率直方图数据指针（概率密度，也可以是像素个数直方图)(大小256）
	unsigned char* pucLUT,		//输出灰度查找表，输出LUT数据指针（大小256)
	bool stateEdgeValue			//是否统计边缘值，即【0，255】灰度值，默认为统计
	)
{
	double hist[256];
	memcpy(hist, pfHist, sizeof(double)* 256);
	//不统计边缘灰度值
	if (!stateEdgeValue)
	{
		hist[0] = 0;
		hist[255] = 0;
	}
	try
	{
		//统计累计直方图
		for (int k = 0; k < 256; k++)
		{
			hist[k] = hist[k - 1] + hist[k];
		}
		for (int k = 0; k < 256; k++)
		{
			hist[k] = 255.0 * hist[k] / hist[255];
		}
		double max1 = 0;
		double min1 = 9000;
		for (int k = 0; k < 256; k++)
		{
			if ((long)hist[k] > max1)
			{
				max1 = hist[k];
			}
			if ((long)hist[k] < min1)
			{
				min1 = hist[k];
			}
		}
		if (max1 == min1)
		{
			for (int k = 0; k < 256; k++)
			{
				pucLUT[k] = (unsigned char)k;
			}
			return FALSE;
		}
		double dValue;
		for (int k = 0; k < 256; k++)
		{
			dValue = 255.0 * (hist[k] - min1) / (max1 - min1);
			if (dValue > 255)
			{
				dValue = 255;
			}
			//确保HAP高精度，做四舍五入
			pucLUT[k] = (unsigned char)(dValue + 0.5);
		}
	}
	catch (...)
	{
		for (int k = 0; k < 256; k++)
		{
			pucLUT[k] = (unsigned char)k;
		}
	}
	return TRUE;
}

//直方图规定化核心代码，SML单映射方法
BOOL standardlizeHis(
	double * pStandard,		//规定化标准直方图
	double * pSrc,			//原直方图
	BYTE * pOutMap			//输出直方图
	)
{
	double bMap1[256], bMap2[256];
	memset(bMap1, 0, sizeof(double)* 256);
	memset(bMap2, 0, sizeof(double)* 256);
	double lTemp1, lTemp2;
	//计算累计直方图bMap1,bMap2
	for (int i = 0; i < 256; i++)
	{
		lTemp1 = 0;
		lTemp2 = 0;
		for (int j = 0; j <= i; j++)
		{
			lTemp1 += pStandard[j];
			lTemp2 += pSrc[j];
		}
		bMap1[i] = lTemp1;
		bMap2[i] = lTemp2;
	}
	double temp;
	double min;
	for (int i = 0; i < 256; i++)
	{
		min = 256;
		for (int j = 0; j < 256; j++)
		{
			temp = bMap1[i] - bMap2[j];
			if (temp < 0 )
			{
				temp = (-1) * temp;
			}
			if (temp < min)
			{
				pOutMap[i] = (BYTE)j;
				min = temp;
			}
		}
	}
	return TRUE;
	
}