//影像自动分块读写接口模块基类，采用多线程方式实现IO,与计算线程同步
#pragma once
#include "GDALFileManager.h"

USING_NAMESPACE(HAPLIB)

class  HAPCLASS HAPBEGTBase
{
public:
	 HAPBEGTBase();
	virtual ~HAPBEGTBase();
	//设置读文件指针
	virtual void Set_MgrIn(CGDALFileManager * mgr, DIMS dims = g_Zdims, int iBlockOverlap = 0) = 0;
	//设置写文件指针
	virtual bool Set_MgrOut(G)

private:

};
