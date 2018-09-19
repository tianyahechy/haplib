#include "GDALFileManager.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include <string>

//缓存开辟大小
int HAPLIB::g_BlockSize = 16 * 1024 * 1024;
//写缓存开辟
int HAPLIB::g_BlockSizeW = 16 * 1024 * 1024;
int HAPLIB::g_SquareBorderSize = 128;

CGDALFileManager::CGDALFileManager()
{
	m_HAPFlag = 1;
	m_poDataset = NULL;
	m_OutFile = NULL;
	strcpy(pcErrMessage, "");
	strcpy(m_szFileName, ""); 
}

CGDALFileManager::CGDALFileManager()
{
	if (m_poDataset)
	{
		GDALClose((GDALDataset *)m_poDataset);
		GDALDestroyDriverManager();
	}
	if (m_OutFile)
	{
		fclose(m_OutFile);
		m_OutFile = NULL;
	}
}

bool CGDALFileManager::LoadFrom(const char * imfileName)
{
	m_HAPFlag = 0;
	GDALAllRegister();
	GDALDataset * poDataSet = (GDALDataset*)GDALOpen(imfileName, GA_ReadOnly);
	//保存gdal调用地址
	m_poDataset = (void*)poDataSet;
	if (m_poDataset == NULL)
	{
		strcpy(pcErrMessage, "文件不能打开！请检查类型是否正确!");
		return false;
	}
	//得到tif等图像的行列数
	m_header.m_nSamples = poDataSet->GetRasterXSize();
	m_header.m_nLines = poDataSet->GetRasterYSize();
	//获取波段数
	m_header.m_nBands = poDataSet->GetRasterCount();
	if (m_header.m_nSamples <= 0 || m_header.m_nLines <= 0 || m_header.m_nBands <= 0)
	{
		strcpy(pcErrMessage, "读入文件头信息错误，不合法的hap文件!");
		return false;
	}
	//获取投影信息
	double AfTransform[6];
	memset(AfTransform, 0, sizeof(double)* 6);
	CPLErr error = poDataSet->GetGeoTransform(AfTransform);
	if (error == CE_None)
	{
		MapInfo * headerMapInfo = new MapInfo;
		//左上角x坐标
		headerMapInfo->m_dLeftUpper[0] = AfTransform[0];
		//左上角y坐标
		headerMapInfo->m_dLeftUpper[1] = AfTransform[3];
		headerMapInfo->m_dOffset[0] = AfTransform[2];
		headerMapInfo->m_dOffset[1] = AfTransform[4];
		headerMapInfo->m_dPixelSize[0] = AfTransform[1];
		headerMapInfo->m_dPixelSize[1] = AfTransform[5];
		headerMapInfo->m_dRightDown[0] = AfTransform[0] + m_header.m_nSamples * AfTransform[1] + m_header.m_nLines * AfTransform[2];
		headerMapInfo->m_dRightDown[1] = AfTransform[3] + m_header.m_nSamples * AfTransform[4] + m_header.m_nLines * AfTransform[5];

		const char * pszSRS_WKT = poDataSet->GetProjectionRef();
		if (strcmp(pszSRS_WKT,"" ) != NULL )
		{
			char * strRel = new char[strlen(pszSRS_WKT) + 1];
			strcpy(strRel, pszSRS_WKT);
			OGRSpatialReference oSRS;
			//二级指针，
			oSRS.importFromWkt(&strRel);
			int IsNorth;
			int zone = oSRS.GetUTMZone(&IsNorth);
			const char * cpTemp = oSRS.GetAttrValue("PROJCS");
			if (cpTemp == NULL)
			{
				strcpy(headerMapInfo->m_cProName, "Arbitrary");
				strcpy(headerMapInfo->m_cUnit, oSRS.GetAttrValue("UNIT"));
				strcpy(headerMapInfo->m_cDatum, oSRS.GetAttrValue("DATUM"));
			}
			else
			{
				cpTemp = oSRS.GetAttrValue("PROJECTION");
				if (strcmp(cpTemp,"Transverse_Mercator") == 0)
				{
					strcpy(headerMapInfo->m_cProName, "UTM");
					headerMapInfo->m_UTM.m_nZone = zone;
					headerMapInfo->m_UTM.m_NS = (IsNorth = 0 ? false : true);
					strcpy(headerMapInfo->m_cUnit, "Meters");
					strcpy(headerMapInfo->m_cDatum, oSRS.GetAttrValue("DATUM"));
				}
				else
				{
					strcpy(pcErrMessage, "暂不支持该投影,投影暂不读取");
					delete headerMapInfo;
					headerMapInfo = NULL;
				}
			}
			delete[] strRel;
		}
		else
		{
			strcpy(headerMapInfo->m_cProName, "Arbitrary");
			//wkt格式有固定的先后顺序，如果unit没有定义则为unknown
			strcpy(headerMapInfo->m_cUnit, "Meters");
			strcpy(headerMapInfo->m_cDatum, "Unknown");
		}
		//防止文件打开时，重复开辟投影信息存放空间，导致内存泄漏
		if (m_header.m_MapInfo != NULL)
		{
			delete m_header.m_MapInfo;
		}
		m_header.m_MapInfo = headerMapInfo;
	}
	//投影信息获取完成
	strcpy(m_header.m_cFileType, "Envi standard");
	m_header.m_nHeaderOffset = 0;
	strcpy(m_header.m_cInterLeave, "bsq");
	SetFileName(imfileName);
	GDALRasterBand * poBand = poDataSet->GetRasterBand(1);
	GDALDataType DT = poBand->GetRasterDataType();
	HAPDataType * pHeadDT = &(m_header.m_nDataType);

	//按照envi格式定义
	switch (DT)
	{
	case GDT_Byte:
		*pHeadDT = HAP_Byte;
		break;

	case GDT_UInt16:
		*pHeadDT = HAP_UInt16;
		break;

	case GDT_Int16:
		*pHeadDT = HAP_Int16;
		break;

	case GDT_Int32:
		*pHeadDT = HAP_Int32;
		break;

	case GDT_Float32:
		*pHeadDT = HAP_Float32;
		break;

	case GDT_Float64:
		*pHeadDT = HAP_Float64;
		break;

	default:
		strcpy(pcErrMessage, "暂不支持该类型读取，请检查DataType!");
		return false;
	}
	return true;
	
}

void CGDALFileManager::GetDIMSDataByBlock(const DIMS& dims, int i, int NumOfBlock, BYTE ** data, int& dataSize)
{
	//获取波段数
	int bands = m_header.m_nBands;
	//每个波段有几块
	int bandSize = NumOfBlock / bands;
	//第i块为dims波段起始的第几块
	int bandB = i % bandSize;
	//第i块在dims第几个波段内(从1开始）
	int bandNum = (i / bandSize + 1);
	//真实波段序号
	int RbandNum = 0;
	int k = 0;
	//用RbandNum代替bandNum,考虑到有可能中间一些波段用户没有选择，所以需要判断是在处理多光谱第几个波段了
	while (RbandNum < bands)
	{
		if (dims.m_pBand[k])
		{
			k++;
		}
		RbandNum++;
		if (k==bandNum)
		{
			break;
		}
	}

	int DimsCol = dims.xEnd - dims.xStart;
	int DimsRow = dims.yEnd - dims.yStart;
	//大部分每个块有多少行，下取整
	int BlockRowLen = DimsRow / bandSize;
	//块在波段的起始行位置
	int DimsBRow = bandB * BlockRowLen;
	//块在图像的起始行位置
	int ImBRow = dims.yStart + DimsBRow;
	//每个波段的末尾都有余数加入最后一段
	if ( i % bandSize == bandSize - 1)
	{
		int leaveB1o = dims.yEnd - ImBRow - BlockRowLen;
		if (leaveB1o != 0)
		{
			BlockRowLen += leaveB1o;
		}
	}

	GDALDataset * poDataset = (GDALDataset *)m_poDataset;
	if (poDataset)
	{
		//从1开始
		GDALRasterBand * poBand = poDataset->GetRasterBand(RbandNum);
		data[0] = new BYTE[DimsCol * BlockRowLen * m_header.getBytesPerPt()];
		poBand->RasterIO(GF_Read, dims.xStart, ImBRow, DimsCol, BlockRowLen, data[0], DimsCol, BlockRowLen, poBand->GetRasterDataType(), 0, 0);
		dataSize = DimsCol * BlockRowLen * m_header.getBytesPerPt();
		return;
	}

}

void CGDALFileManager::GetDIMSDataByBlockInfo(SimDIMS * psBlockInfo, BYTE * data)
{
	int DimsRow = psBlockInfo->getHeight();
	int DimsCol = psBlockInfo->getWidth();
	GDALDataset * poDataset = (GDALDataset*)m_poDataset;
	if (poDataset)
	{
		//从1开始
		GDALRasterBand * poBand = poDataset->GetRasterBand(psBlockInfo->band + 1);
		poBand->RasterIO(GF_Read, psBlockInfo->xStart,  psBlockInfo->yStart, DimsCol, DimsRow, data, DimsCol, DimsRow, poBand->GetRasterDataType(), 0, 0);
		return;
	}
}

bool CGDALFileManager::GetDIMSDataByBlockInfo(SimDIMS * psBlockInfo, DIMS* pDims, BYTE * data)
{
	//依据影像数据类型判断获取数据方式
	if (psBlockInfo->xStart == -1)
	{
		return false;
	}
	int DimsRow = psBlockInfo->getHeight();
	int DimsCol = psBlockInfo->getWidth();
	CPLErr GdalErr = CE_None;
	GDALDataset * poDataset = (GDALDataset*)m_poDataset;
	if (poDataset)
	{
		//从1开始
		GDALRasterBand * poBand;
		int iImageBandNo = pDims->getImgBandNum(psBlockInfo->band);
		//可能存在psblockinfo指定波段不存在情况，如图像很小，波段不多
		if (iImageBandNo <= -1)
		{
			return false;
		}
		poBand = poDataset->GetRasterBand(iImageBandNo + 1);
		GdalErr = poBand->RasterIO(GF_Read, pDims->xStart + psBlockInfo->xStart, pDims->yStart + psBlockInfo->yStart, DimsCol, DimsRow, data, DimsCol, DimsRow, poBand->GetRasterDataType(), 0, 0);
	}
	return true;
}

bool CGDALFileManager::WGetDIMSDataByBlockInfo(SimDIMS * psBlockInfo, DIMS* pDims, BYTE * data)
{
	int bands = pDims->getDIMSBands();
	if (bands != psBlockInfo->band)
	{
		return false;
	}

	int DimsRow = psBlockInfo->getHeight();
	int DimsCol = psBlockInfo->getWidth();
	//获取波段数目和波段数组
	int * pBandArray = new int[bands];
	int i = 0; 
	int j = 0;
	for ( i = 0; i < m_header.m_nBands; i++)
	{
		if (pDims->m_pBand[i])
		{
			pBandArray[j] = i + 1;
			j++;
		}
	}
	GDALDataset * poDataset = (GDALDataset*)m_poDataset;
	//读取多波段数组，存储为BSQ格式
	if (poDataset)
	{
		poDataset->RasterIO(
			GF_Read, 
			pDims->xStart + psBlockInfo->xStart, 
			pDims->yStart + psBlockInfo->yStart, 
			DimsCol, 
			DimsRow, 
			data, 
			DimsCol, 
			DimsRow, 
			poDataset->GetRasterBand(1)->GetRasterDataType(),
			bands,
			pBandArray,
			0, 
			0,
			0);
	}
	delete[] pBandArray;
	return true;
}

//文件图像属性
void CGDALFileManager::GetFileName(char fileName[])
{
	if ( !fileName)
	{
		memcpy(fileName, m_szFileName, HAP_MAX_PATH);
	}
	return;
}

void CGDALFileManager::SetFileName(const char* fileName)
{
	memcpy(m_szFileName, fileName, HAP_MAX_PATH);

}

//保存类方法
bool CGDALFileManager::HpsSaveImageAs(const char * szImageName)
{
	char szExt[_MAX_EXT] = "";
	char * pszFormat;
	_splitpath(szImageName, NULL, NULL, NULL, szExt);
	if (stricmp(".img", szExt) == 0 || stricmp(".hap", szExt)  == 0 )
	{
		pszFormat = "HAPFile";
		this->SetFileName(szImageName);
		std::string CSFileName;
		CSFileName.append(szImageName);

		std::string SBaseNameHeader;
		basic_string<char>::size_type indexCh;
		static const basic_string<char>::size_type nPos = -1;
		indexCh = CSFileName.rfind(".");
		if (indexCh == nPos)
		{
			SBaseNameHeader = CSFileName + ".hdr";
		}
		else
		{
			string SBaseName = CSFileName.substr(0, indexCh);
			SBaseNameHeader = SBaseName + ".hdr";
		}
		this->SaveHeader(SBaseNameHeader);
		if ( ( m_OutFile = fopen(szImageName, "wb") ) == NULL)
		{
			sprintf(pcErrMessage, "Error:\n %s\n File could not be opened!", szImageName);
			m_OutFile = NULL;
			exit(0);
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;

}

bool CGDALFileManager::WriteBlock(BYTE * pdata, int bufferSize)
{
	fwrite(pdata, sizeof(BYTE), bufferSize, m_OutFile);
	return true;
}

bool CGDALFileManager::WriteBlockByInfo(BYTE * pdata, SimDIMS * pSBlockInfo)
{
	return true;
}

//模式管理
bool CGDALFileManager::BoolIsImgInMem()
{
	//非内存模式
	return false;
}

//关闭输出文件
void CGDALFileManager::Close()
{
	if (m_OutFile != NULL)
	{
		fclose(m_OutFile);
		m_OutFile = NULL;
		this->Open();	//变为输入文件，便于流程化管理
	}
	return;
}

//打开已经关闭的生成的图像
bool CGDALFileManager::Open()
{
	if (strcmp(m_szFileName,"") == 0 )
	{
		return false;
	}
	if (m_poDataset != NULL)
	{
		return false;
	}
	m_HAPFlag = 0;
	GDALAllRegister();
	GDALDataset * poDataset = (GDALDataset *)GDALOpen(m_szFileName, GA_ReadOnly);
	m_poDataset = (void *)poDataset;
	return true;
}
