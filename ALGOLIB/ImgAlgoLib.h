#include "comlib.h"

//ͼ��ֱ��ͼ����
ALGOLIB_API BOOL Func_HistGramEqulizeEnhance(
	double * pfHist,			//����Ƶ��ֱ��ͼ����ָ�루�����ܶȣ�Ҳ���������ظ���ֱ��ͼ)(��С256��
	unsigned char* pucLUT,		//����ҶȲ��ұ����LUT����ָ�루��С256)
	bool stateEdgeValue = true	//�Ƿ�ͳ�Ʊ�Եֵ������0��255���Ҷ�ֵ��Ĭ��Ϊͳ��
	);

//ֱ��ͼ�涨���Ĵ���,GML��ӳ�䷽��
ALGOLIB_API BOOL standardlizeHistGML(
	double* pStandard,			//�涨����׼ֱ��ͼ
	double* pSrc,				//ԭֱ��ͼ
	BYTE* pOutMap,				//���ֱ��ͼӳ���ϵ��pOutMap[i]Ϊ���ֵ
	int grayScale = 256			//�ҽ׵ȼ�����Ϊ8��64��256��512��1024
	);
//ֱ��ͼ�涨�����Ĵ��룬SML��ӳ�䷽��
ALGOLIB_API BOOL standardlizeHist(
	double* pStandard,			//�涨����׼ֱ��ͼ
	double* pSrc,				//ԭֱ��ͼ
	BYTE* pOutMap				//���ֱ��ͼӳ���ϵ��pOutMap[i]Ϊ���ֵ
	);

class ALGOLIB_API Filter_IMG
{
public:
	Filter_IMG();
	virtual ~ Filter_IMG();

public:
	//����һά���飬����������ֵ
	static int MedianF(int* pData, int length);

};
