
#include "stdAfx.h"
#include "waterMagic_IPAS.h"
#include <math.h>
#include "HAPBEGThreadS.h"
#include "HAPBEGThreadW.h"
#include "HAPBEGThreadC.h"
#include "GDALLib.h"

WaterMagic_IPAS::WaterMagic_IPAS()
{
	//文件路径
	strcpy(m_FileName, "");
	//文件路径
	strcpy(m_OutFileName, "");
	m_iMaxValue = 255;
	m_ImgIO = NULL;
	m_ImageThold = -1;
	m_ImageTHoldUp = -1;
	m_SeedCoor[0] = NULL;
	m_SeedCoor[1] = NULL;
	m_smallSpot = 0;
	m_pBSeedReg = NULL;
}

WaterMagic_IPAS::~WaterMagic_IPAS()
{
	if (m_SeedCoor[0])
	{
		delete[] m_SeedCoor[0];
		delete[] m_SeedCoor[1];
		m_SeedCoor[0] = NULL;
	}
	if (m_pBSeedReg)
	{
		CComlib::ffree_1d((void*)m_pBSeedReg);
		m_pBSeedReg = NULL;
	}
	
}

void WaterMagic_IPAS::Set_FileName(const char* fileName)
{
	if (fileName != NULL)
	{
		strcpy(m_FileName, fileName);
	}
	//搜索中间结果文件
	std::string CSFileName;
	CSFileName.append(fileName);
	
	std::basic_string<char>::size_type indexCh;
	static const std::basic_string<char>::size_type nPos = -1;

	indexCh = CSFileName.rfind(".");
	std::string l_str1;
	char l_str2[128];
	sprintf(l_str2, "_MedResult.img");
	if (indexCh == nPos)
	{
		l_str1 = CSFileName + l_str2;
	}
	else
	{
		std::string SBaseName = CSFileName.substr(0, indexCh);
		l_str1 = SBaseName + l_str2;
	}
	strcpy(m_TempFileName, l_str1.c_str());
}

bool WaterMagic_IPAS::StandardProcess(DIMS dims, CGDALFileManager * pmgr1, CGDALFileManager * pmgr2)
{
	    
}