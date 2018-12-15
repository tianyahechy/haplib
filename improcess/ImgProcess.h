#pragma once

#include "hpscegsbaseTmpl.h"
#include "HAPBEGThread.h"

#define IMGPROCESS_EXPORTS
#ifdef IMGPROCESS_EXPORTS
#define IMGPROCESS_API	__declspec(dllexport)
#else
#define IMGPROCESS_API	__declspec(dllimport)
#endif

#pragma warning(disable : 4996)
//�����û���������
#define ImgRead *(TT*) m_ImgIO->Read
#define ImgReadL *(TT*) m_ImgIO->ReadL
#define ImgWrite(Value) *(TT*) m_ImgIO->Write(&Value);

//ͼ������࣬�����㷨�̳и���
class IMGPROCESS_API Base_IPAS
{
public:
	Base_IPAS();
	virtual ~Base_IPAS();
	void Set_FileName(const char* fileName);
	//��׼���������㷨�����Ĵ���-����ͼ���ļ���׼��������������㣬���ͼ���ļ�
	//��ʼ��m_ImgIO,�㷨�����������������͵��ò�ͬ��Process()����
	bool StandardProcess(DIMS dims, CGDALFileManager * pmgr1, CGDALFileManager* pMgr2 = NULL);
	char * GetErrorMessage();

protected:
	//����ͼ��������ݣ�ֻ�������ļ�������ļ������
	//int cpuNo:cpu��ţ���1��ʼ
	template<class TT>
	bool Process(int cpuNo = 0);
	//׼�����������ͬ��ͼ�β���������ֻ�������ļ���ȡ�йأ�������ļ��޹�
	template<class TT>
	bool PreparePara(int cpuNo = 0);
	//���ö�̬������ƣ��������ָ��ΪHAPBEGThread�����Ҳ������������������ֻ�ṩ1��
	HAPBEGTBase* m_ImgIO;
	//�ļ�·��
	char m_FileName[512];
	//������������Լ����ģ�庯���ͼ��㲿�ֲ���ʹ��
	HAPDataType m_iDataTypeIn;
	//�������ʱ������Ϣ
	char m_pcErrMessage[256];

};
