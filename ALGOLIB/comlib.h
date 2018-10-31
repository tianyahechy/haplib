#pragma once

#ifdef WIN32
#ifdef ALGOLIB_EXPORTS
#define ALGOLIB_API	__declspec(dllexport)
#else
#define ALGOLIB_API	__declspec(dllimport)
#endif
#endif

#ifdef _LINUX
#define ALGOLIB_API
#endif

#pragma warning(disable:4996)
#include <stdio.h>

//��c���Ի�ȡ�ļ���С��������Ҫ��
__int64 ALGOLIB_API	fsize(FILE* fp);
struct ALGOLIB_API sttAlgoProg
{
	sttAlgoProg();
	~sttAlgoProg();

	void writeDebugMessage(char* pcMes);
	//������ָʾλ�ã�0-100֮�䣬-1����ʾ������������Ϊ10�ı���ʱ���Ը��½�����
	//���������㷨ִ��״̬��ʾ��Ϣ
	char pcAlgoStatus[256];
	//��¼������Ϣ
	char pcErrorMessage[256];
	char pcMessageSort[5];
	FILE * ptf;
};
extern ALGOLIB_API sttAlgoProg stHAPProg;
class ALGOLIB_API CComlib
{
public:
	 CComlib();
	virtual~ CComlib();

public:
	//�洢�ռ�
	//����һά��̬�ڴ�
	static void *fspace_1d(int col, int length);
	//�����ά��̬�ڴ�
	static void **fspace_2d(int row, int col, int length);
	//������ά��̬�ڴ�
	static void ***fspace_3d(int row1, int row2, int row3, int length);
	static void ffree_1d(void* a);
	static void ffree_2d(void** a, int row);
	static void ffree_3d(void*** a, int row1, int row2);
	//��ȡ�ļ���չ��
	static void GetExt(const char* strPathName, char* ext1);
};
