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

//用c语言获取文件大小，经常需要用
__int64 ALGOLIB_API	fsize(FILE* fp);
struct ALGOLIB_API sttAlgoProg
{
	sttAlgoProg();
	~sttAlgoProg();

	void writeDebugMessage(char* pcMes);
	//进度条指示位置，0-100之间，-1：表示进度条结束。为10的倍数时可以更新进度条
	//进度条上算法执行状态提示信息
	char pcAlgoStatus[256];
	//记录出错信息
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
	//存储空间
	//分配一维动态内存
	static void *fspace_1d(int col, int length);
	//分配二维动态内存
	static void **fspace_2d(int row, int col, int length);
	//分配三维动态内存
	static void ***fspace_3d(int row1, int row2, int row3, int length);
	static void ffree_1d(void* a);
	static void ffree_2d(void** a, int row);
	static void ffree_3d(void*** a, int row1, int row2);
	//获取文件扩展名
	static void GetExt(const char* strPathName, char* ext1);
};
