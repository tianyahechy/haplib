//Ӱ���Զ��ֿ��д�ӿ�ģ����࣬���ö��̷߳�ʽʵ��IO,������߳�ͬ��
#pragma once
#include "GDALFileManager.h"

USING_NAMESPACE(HAPLIB)

class  HAPCLASS HAPBEGTBase
{
public:
	 HAPBEGTBase();
	virtual ~HAPBEGTBase();
	//���ö��ļ�ָ��
	virtual void Set_MgrIn(CGDALFileManager * mgr, DIMS dims = g_Zdims, int iBlockOverlap = 0) = 0;
	//����д�ļ�ָ��
	virtual bool Set_MgrOut(G)

private:

};
