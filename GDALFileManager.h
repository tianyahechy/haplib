// GDALFileManager.h: interface for the CGDALFileManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GDALFILEMANAGER_H__5F0E867D_59AD_4884_A337_BD382A4FD558__INCLUDED_)
#define AFX_GDALFILEMANAGER_H__5F0E867D_59AD_4884_A337_BD382A4FD558__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "hapcfile.h"
#include <string>


using namespace std;
USING_NAMESPACE(HAPLIB)


class HAPCLASS CGDALFileManager   // : public HpsCFileManager  
{
public:
	CGDALFileManager();
	virtual ~CGDALFileManager();

	//�ļ���ͼ�����Բ���
	void GetFileName(char filename[HAP_MAX_PATH]);
	bool LoadFrom(const char* Imfilename);

	void GetDIMSDataByBlock(const DIMS &dims,int i, int NumOfBlock,BYTE **data,int &datasize);
	/************************************************************************/
	/* ͨ���������ݿ��С������ȡͼ������ָ��       
	DIMS& SBlockInfo: �������Ϣ
	BYTE *data�������������ݣ��ڴ治�ڴ˷��� 
	*/
	/************************************************************************/
	void GetDIMSDataByBlockInfo(SimDIMS* pSBlockInfo, BYTE *data);
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	bool GetDIMSDataByBlockInfo(SimDIMS* pSBlockInfo, DIMS* pDims, BYTE *data);
	//[11/18/2011 liuxiang]����SimDims�����м�¼���ι��ܣ��������DIMS������
	bool WGetDIMSDataByBlockInfo(SimDIMS* pSBlockInfo, DIMS* pDims, BYTE *data);

	//��ȡGDAL�������������
//	inline GDALDataType GetGDALDataType();
	///************************************************************************/
	///* �����ڴ���С����ȡͼ���ܵķֿ���������ﰴ���ηֿ�ȡ��                                                                     */
	///************************************************************************/
	//int InitBlocks(DIMS dims); 
	////	bool BlocksDone(void);
	////	int GetBlockInBand(const int i, const int NumOfBlock,int &StrRow,int &NumofRow);

	////������
	////����ͼ���кŻ�ȡ���������ţ��ڿ��е�ʼ��λ�ã��߶ȵ���Ϣ��
	//int GetBlockSize(int row,int &StRow,int &NumRow,int &NOBlock);
	//int GetBlockSize(int index, int numofblock);

	/************************************************************************/
	/* ͨ��ͼ���С��С����κ�ֱ��ȷ���������ڵĿ�������Ϣ��                                                                     */
	/************************************************************************/
	void GetBlockInfo(int col, int row, int band, DIMS* pSBlockInfo);

	// 	�����෽��
	bool HpsSaveImageAs(const char *szImageName);  //���ڴ�ģʽ
	bool WriteBlock(BYTE *pdata, int buffersize);
	bool SaveHeader(string sFileHeader);	

	//�ನ��д�����ݣ�buffersizeΪbuffer��ʵ��С������������
	//��Ϊbuffer����Ѱַ��ԭ��writedonebufferΪ��buffer�ڴ洢����������С
	//writedownbuffer = writecount*ptLength
	bool WWriteBlockB(BYTE *pdata, int buffersize);	
	bool WWriteBlock2(BYTE *pdata, int buffersize, int writedonebuffer);//yy��д
	bool WWriteBlock(BYTE *pdata, int buffersize);
	//���ݻ�������Ϣд��ͼ������
	bool WriteBlockByInfo(BYTE *pdata, SimDIMS* pSBlockInfo);
	// 	//ģʽ����
	bool BoolIsImgInMem();

	//��ʼ��������
	void Close();
	bool Open();

	int m_HAPFlag;				//�ļ����ͱ�־����0: GDAL֧�����ͣ�1: hapƽ̨�������ͣ�2: �ⲿ��չ���͡�
	HAPCFileHeader m_header;    //ͼ����Ϣͷ�ṹ����
	char pcErrMessage[256];		// ���������ʾ��Ϣ��


protected:
	void SetFileName(const char *filename); //����ͼ���ļ�·��

	void *m_poDataset; //GDALDataset�������õ�ʱ����ת����
//	int m_iCurBand;//��ǰ���κű�־
	FILE *m_OutFile;
//	char* m_pcFileName; //�ļ�·��ָ��
	char m_szFileName[HAP_MAX_PATH]; //�ļ�����
	char m_szHeaderFileName[HAP_MAX_PATH]; //ͷ�ļ�����
	int iCutBlockMode; //=0, Ĭ�ϰ��зֿ��ȡ��
};

#endif // !defined(AFX_GDALFILEMANAGER_H__5F0E867D_59AD_4884_A337_BD382A4FD558__INCLUDED_)
