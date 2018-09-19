#include "GDALFileManager.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include <string>

//���濪�ٴ�С
int HAPLIB::g_BlockSize = 16 * 1024 * 1024;
//д���濪��
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
	//����gdal���õ�ַ
	m_poDataset = (void*)poDataSet;
	if (m_poDataset == NULL)
	{
		strcpy(pcErrMessage, "�ļ����ܴ򿪣����������Ƿ���ȷ!");
		return false;
	}
	//�õ�tif��ͼ���������
	m_header.m_nSamples = poDataSet->GetRasterXSize();
	m_header.m_nLines = poDataSet->GetRasterYSize();
	//��ȡ������
	m_header.m_nBands = poDataSet->GetRasterCount();
	if (m_header.m_nSamples <= 0 || m_header.m_nLines <= 0 || m_header.m_nBands <= 0)
	{
		strcpy(pcErrMessage, "�����ļ�ͷ��Ϣ���󣬲��Ϸ���hap�ļ�!");
		return false;
	}
	//��ȡͶӰ��Ϣ
	double AfTransform[6];
	memset(AfTransform, 0, sizeof(double)* 6);
	CPLErr error = poDataSet->GetGeoTransform(AfTransform);
	if (error == CE_None)
	{
		MapInfo * headerMapInfo = new MapInfo;
		//���Ͻ�x����
		headerMapInfo->m_dLeftUpper[0] = AfTransform[0];
		//���Ͻ�y����
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
			//����ָ�룬
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
					strcpy(pcErrMessage, "�ݲ�֧�ָ�ͶӰ,ͶӰ�ݲ���ȡ");
					delete headerMapInfo;
					headerMapInfo = NULL;
				}
			}
			delete[] strRel;
		}
		else
		{
			strcpy(headerMapInfo->m_cProName, "Arbitrary");
			//wkt��ʽ�й̶����Ⱥ�˳�����unitû�ж�����Ϊunknown
			strcpy(headerMapInfo->m_cUnit, "Meters");
			strcpy(headerMapInfo->m_cDatum, "Unknown");
		}
		//��ֹ�ļ���ʱ���ظ�����ͶӰ��Ϣ��ſռ䣬�����ڴ�й©
		if (m_header.m_MapInfo != NULL)
		{
			delete m_header.m_MapInfo;
		}
		m_header.m_MapInfo = headerMapInfo;
	}
	//ͶӰ��Ϣ��ȡ���
	strcpy(m_header.m_cFileType, "Envi standard");
	m_header.m_nHeaderOffset = 0;
	strcpy(m_header.m_cInterLeave, "bsq");
	SetFileName(imfileName);
	GDALRasterBand * poBand = poDataSet->GetRasterBand(1);
	GDALDataType DT = poBand->GetRasterDataType();
	HAPDataType * pHeadDT = &(m_header.m_nDataType);

	//����envi��ʽ����
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
		strcpy(pcErrMessage, "�ݲ�֧�ָ����Ͷ�ȡ������DataType!");
		return false;
	}
	return true;
	
}

void CGDALFileManager::GetDIMSDataByBlock(const DIMS& dims, int i, int NumOfBlock, BYTE ** data, int& dataSize)
{
	//��ȡ������
	int bands = m_header.m_nBands;
	//ÿ�������м���
	int bandSize = NumOfBlock / bands;
	//��i��Ϊdims������ʼ�ĵڼ���
	int bandB = i % bandSize;
	//��i����dims�ڼ���������(��1��ʼ��
	int bandNum = (i / bandSize + 1);
	//��ʵ�������
	int RbandNum = 0;
	int k = 0;
	//��RbandNum����bandNum,���ǵ��п����м�һЩ�����û�û��ѡ��������Ҫ�ж����ڴ������׵ڼ���������
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
	//�󲿷�ÿ�����ж����У���ȡ��
	int BlockRowLen = DimsRow / bandSize;
	//���ڲ��ε���ʼ��λ��
	int DimsBRow = bandB * BlockRowLen;
	//����ͼ�����ʼ��λ��
	int ImBRow = dims.yStart + DimsBRow;
	//ÿ�����ε�ĩβ���������������һ��
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
		//��1��ʼ
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
		//��1��ʼ
		GDALRasterBand * poBand = poDataset->GetRasterBand(psBlockInfo->band + 1);
		poBand->RasterIO(GF_Read, psBlockInfo->xStart,  psBlockInfo->yStart, DimsCol, DimsRow, data, DimsCol, DimsRow, poBand->GetRasterDataType(), 0, 0);
		return;
	}
}

bool CGDALFileManager::GetDIMSDataByBlockInfo(SimDIMS * psBlockInfo, DIMS* pDims, BYTE * data)
{
	//����Ӱ�����������жϻ�ȡ���ݷ�ʽ
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
		//��1��ʼ
		GDALRasterBand * poBand;
		int iImageBandNo = pDims->getImgBandNum(psBlockInfo->band);
		//���ܴ���psblockinfoָ�����β������������ͼ���С�����β���
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
	//��ȡ������Ŀ�Ͳ�������
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
	//��ȡ�ನ�����飬�洢ΪBSQ��ʽ
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

//�ļ�ͼ������
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

//�����෽��
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

//ģʽ����
bool CGDALFileManager::BoolIsImgInMem()
{
	//���ڴ�ģʽ
	return false;
}

//�ر�����ļ�
void CGDALFileManager::Close()
{
	if (m_OutFile != NULL)
	{
		fclose(m_OutFile);
		m_OutFile = NULL;
		this->Open();	//��Ϊ�����ļ����������̻�����
	}
	return;
}

//���Ѿ��رյ����ɵ�ͼ��
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
           
bool CGDALFileManager::SaveHeader(string sFileHeader)
{
	size_t headerSize = strlen(sFileHeader.c_str());
	int len = strcmp(sFileHeader.c_str(), "");
	if ( headerSize == 0 || len == 0 )
	{
		return false;
	}
	strcpy(m_szHeaderFileName, sFileHeader.c_str());

	int iLines = m_header.m_nLines;
	int iSamples = m_header.m_nSamples;
	int iBands = m_header.m_nBands;
	int iDataType = m_header.m_nDataType;

	strcat(m_header.m_cDescription, "��������:");
	char dbuffer[9];
	char tbuffer[9];
	_strdate(dbuffer);
	strcat(m_header.m_cDescription, dbuffer);
	strcat(m_header.m_cDescription, ",����ʱ��: ");
	_strtime(tbuffer);
	strcat(m_header.m_cDescription, ",HAPIMG File V1.0 created by spacestartMan");
	strcpy(m_header.m_cInterLeave, "bsq");
	m_header.m_nHeaderOffset = 0;
	strcpy(m_header.m_cFileType, "ENVI Standard");
	string l_str1;
	char l_str2[512];
	sprintf(l_str2, "ENVI\n[HAPIMG]\n");
	l_str1 += l_str2;
	sprintf(l_str2, "description = {\n%s}\n", m_header.m_cDescription);
	l_str1 += l_str2;
	sprintf(l_str2,
		"sample = %d\n lines = %d\n bands = %d\n data type = %d\n header offset = %d\n",
		iSamples,
		iLines,
		iBands,
		iDataType,
		m_header.m_nHeaderOffset);
	l_str1 += l_str2;
	sprintf(l_str2, "file type = %s\ninterleave = %s\n", m_header.m_cFileType, m_header.m_cInterLeave);
	l_str1 += l_str2;

	//���ͶӰ��Ϣ
	if (m_header.m_MapInfo != NULL)
	{
		char * projectName = m_header.m_MapInfo->m_cProName;
		//�ֱ�����ʱ��С����ָ����ʽ��ʾ
		sprintf(l_str2, "map info = {%s, %.4f, %.4f, %.4f, %.4f, %.4e, %.4e",
			projectName, 1.0, 1.0, m_header.m_MapInfo->m_dLeftUpper[0], m_header.m_MapInfo->m_dLeftUpper[1],
			m_header.m_MapInfo->m_dPixelSize[0],
			abs(m_header.m_MapInfo->m_dPixelSize[1]));
		l_str1 += l_str2;
		if (strcmp(projectName, "UTM") == 0 )
		{
			char cpUTMns[10];
			strcpy(cpUTMns, (m_header.m_MapInfo->m_UTM.m_NS == true) ? "North" : "South");
			sprintf(l_str2, "%s, units = %s}\n", "WGS-84", m_header.m_MapInfo->m_cUnit);
			l_str1 += l_str2;
			if (strcmp(m_header.m_MapInfo->m_cDatum, "WGS_1984") == 0 )
			{
				sprintf(l_str2, "units = %s\n", m_header.m_MapInfo->m_cUnit);
				l_str1 += l_str2;
			}
		}
		else
		{
			sprintf(l_str2, "%s, units = %s}\n", m_header.m_MapInfo->m_cDatum, m_header.m_MapInfo->m_cUnit);
			l_str1 += l_str2;
		}
	}
	FILE * fp;
	if ((fp = fopen(sFileHeader.c_str(), "w") ) == NULL )
	{
		printf("%s can not be opened for writing!", sFileHeader);
		return false;
	}
	fprintf(fp, "%s\n", l_str1.c_str());
	fclose(fp);
	return true;
}

bool CGDALFileManager::WWriteBlock2(BYTE * pdata, int bufferSize, int writedoneBuffer)
{
	//��ȡͼ�������Ϣ
	int width = m_header.m_nSamples;
	int height = m_header.m_nLines;
	int band = m_header.m_nBands;
	int nDt = m_header.getBytesPerPt(); 

	//��ȡͼ��һ�������ֽ���
	__int64 bytesPerBand = width * height * nDt;
	//����ÿ��������Ҫ�����buffer��С
	int bufferPerBand = bufferSize / band;
	//��ȡ��ǰ׼���洢���ļ�ָ��λ��
	__int64 currentPos = ftell(m_OutFile);
	//��ʼ��ÿ������д������
	long i;
	for ( i = 0; i < band; i++)
	{
		//�ļ�ָ���ƶ�
		//֧�ֳ���4G���ݵ�д��Ѱַ
		_fseeki64(m_OutFile, __int64(i*bytesPerBand + currentPos), 0);
		fwrite(pdata + i * bufferPerBand, sizeof(BYTE), bufferPerBand, m_OutFile);
	}
	//֧�ֳ���4G���ݵ�д��Ѱַ
	_fseeki64(m_OutFile, __int64((1 - band) * bytesPerBand), 1);
	return true;
}

bool CGDALFileManager::WWriteBlockB(BYTE * pdata, int bufferSize)
{
	//��ȡͼ�������Ϣ
	int width = m_header.m_nSamples;
	int height = m_header.m_nLines;
	int band = m_header.m_nBands;
	int nDt = m_header.getBytesPerPt();

	//��ȡͼ��һ�������ֽ���
	__int64 bytesPerBand = width * height * nDt;
	//����ÿ��������Ҫ�����buffer��С
	int bufferPerBand = bufferSize / band;
	BYTE * pbbandblock = new BYTE[bufferPerBand];

	//��ʼ��ÿ������д������
	long i, j;
	for (i = 0; i < band; i++)
	{
		//��123 123 123 ...�洢��ʽ����111.222.333
		for ( j = 0; j < bufferPerBand / nDt; j++)
		{
			memcpy(pbbandblock + j * nDt, pdata + (j*band + i) * nDt, nDt);
		}
		//�ļ�ָ���ƶ�
		//֧�ֳ���4G���ݵ�д��Ѱַ
		_fseeki64(m_OutFile, __int64(i*bytesPerBand), 0);
		fwrite(pbbandblock, sizeof(BYTE), bufferPerBand, m_OutFile);
	}
	//֧�ֳ���4G���ݵ�д��Ѱַ
	_fseeki64(m_OutFile, __int64((1 - band) * bytesPerBand), 1);
	delete pbbandblock;
	return true;
}
