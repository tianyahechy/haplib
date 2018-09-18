#include "GDALFileManager.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h"

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