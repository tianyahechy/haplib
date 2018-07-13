#pragma once
#include "HAPLib.h"
USING_NAMESPACE(HAPLIB)
#pragma warning(disable:4996)

class HAPCLASS CCacheQueue
{
public:
	 CCacheQueue();
	~ CCacheQueue(); //析构缓存
	BYTE * requestCache(DIMS * &pSBlockInf);
	//输入一级缓冲区
	static BYTE * m_bBlockIn;
	//输入二级缓冲区
	static BYTE * m_bBlockIn2;
	//输出缓冲区
	static BYTE * m_bBlockOut;
	//输入一级缓冲区信息
	DIMS m_SBlockInInf;
	//输入二级缓冲区信息
	DIMS m_SBlockInInf2;
	//输出缓冲区信息
	DIMS m_SBlockOutInf;

protected:
	//标记哪一号缓冲区数据即将丢弃
	int m_iFlag;
};
