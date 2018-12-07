#include "include_global.h"
#include "comlib.h"
#include <string>
#include <iostream>
#include <io.h>

__int64 fsize(FILE * fp)
{
	//当前位置
	fpos_t fpos;
	//获取当前位置
	fgetpos(fp, &fpos);
	_fseeki64(fp, 0, SEEK_END);
	__int64 n = _ftelli64(fp);
	//恢复之前的位置
	fsetpos(fp, &fpos);
	return n;
}

//定义进度条指示、出错信息
sttAlgoProg stHAPProg;

sttAlgoProg::sttAlgoProg()
{
	ProgPo = -1;
	strcpy(pcErrorMessage, "");
	strcpy(pcAlgoStatus, "");
	strcpy(pcErrorMessage, "");
	strcpy(pcMessageSort, "错误！");
	ptf = fopen("E:\\TraceFile.txt", "a");
	ptf = NULL;
}

sttAlgoProg::~sttAlgoProg()
{
	if (ptf)
	{
		fprintf(ptf, "\n");
		fclose(ptf);
	}
	ptf = NULL;
}

void sttAlgoProg::writeDebugMessage(char* pcMes)
{
	if (pcMes&& ptf)
	{
		fprintf(ptf, "%s\n", pcMes);
	}
}

static BOOL ASSERTVALIDPOINTER(void * pointer, char* message)
{
	if (pointer == NULL)
	{
		sprintf(stHAPProg.pcErrorMessage, "Error Message:%s\n", message);
		return false;
	}
	return TRUE;
}

CComlib::CComlib()
{

}

CComlib::~CComlib()
{

}
void * CComlib::fspace_1d(int col, int length)
{
	void * b = (void*)calloc(length, col);
	char err[50];
	sprintf(err, "Sorry , not enough space for you!\n");
	ASSERTVALIDPOINTER(b, err);
	return b;
}

void ** CComlib::fspace_2d(int row, int col, int length)
{
	void ** b = new void*[row];
	char err[50];
	sprintf(err, "Sorry , not enough space for you!\n");
	ASSERTVALIDPOINTER(b, err);
	for (int i = 0; i < row; i++)
	{
		b[i] = (void*) new char[col*length];
		ASSERTVALIDPOINTER(b[i], err);
	}
	return b;
}

void *** CComlib::fspace_3d(int row1, int row2, int row3, int length)
{
	void ***b = (void***)calloc(sizeof(void**), row1);
	char err[50];
	sprintf(err, "Sorry , not enough space for you!\n");
	ASSERTVALIDPOINTER(b, err);
	for (int i = 0; i < row1; i++)
	{
		b[i] = (void**)fspace_2d(row2, row3, length);
	}
	return b;

}

void CComlib::ffree_1d(void *a)
{
	if (a == NULL)
	{
		return;
	}
	free(a);
	a = NULL;

}

void CComlib::ffree_2d(void** a, int row)
{
	if (a == NULL)
	{
		return;
	}
	for (int i = 0; i < row; i++)
	{
		delete[] a[i];
	}
	delete[] a;
	a = NULL;
}

void CComlib::GetExt(const char* strPathName, char* ext)
{
	char * pExt = strrchr((char*)strPathName, '.');
	if (NULL == pExt)
	{
		strcpy(ext, "");
		return;
	}
	strcpy(ext, ++pExt);
	strlwr(ext);
}
