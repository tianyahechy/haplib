#include "ImgProcess.h"

Base_IPAS::Base_IPAS()
{
	m_iDataTypeIn = HAP_Byte;
	//文件路径
	strcpy(m_FileName, "");
	//记录错误信息
	strcpy(m_pcErrMessage, "");
	m_ImgIO = NULL;
}

Base_IPAS::~Base_IPAS()
{
	if (m_ImgIO)
	{
		delete m_ImgIO;
		m_ImgIO = NULL;
	}
}

void Base_IPAS::Set_FileName(const char * fileName)
{
	if (fileName != NULL)
	{
		strcpy(m_FileName, fileName);
	}
}

bool Base_IPAS::StandardProcess(DIMS dims, CGDALFileManager* pmgr1, CGDALFileManager* pmgr2)
{
	if (m_ImgIO)
	{
		delete m_ImgIO;
	}
	//设置为多线程模式
	m_ImgIO = new HAPBEGThread;
	m_iDataTypeIn = (HAPDataType)pmgr1->m_header.m_nDataType;
	pmgr2->m_header = pmgr1->m_header;
	//还需简化dims操作
	pmgr2->m_header.m_nSamples = dims.getWidth();
	pmgr2->m_header.m_nLines = dims.getHeight();
	pmgr2->m_header.m_nBands = dims.getDIMSBands();

	if (!m_ImgIO->Set_MgrIn(pmgr1, dims))
	{
		return false;
	}

	//条件判断，准备参数，如计算直方图，信息熵，协方差矩阵，输出临时影像等
	bool bYesOrNo = false;
	switch (m_iDataTypeIn)
	{
	case HAP_Byte:
		bYesOrNo = PreparePara<BYTE>();
		break;
	case HAP_UInt16:
		bYesOrNo = PreparePara<unsigned short>();
		break;
	case HAP_Int16:
		bYesOrNo = PreparePara<short int>();
		break;
	case HAP_Int32:
		bYesOrNo = PreparePara<int>();
		break;
	case HAP_Float32:
		bYesOrNo = PreparePara<float>();
		break;
	case HAP_Float64:
		bYesOrNo = PreparePara<double>();
		break;
	default:
		bYesOrNo = false;
		break;
	}
	//判断是否为单输入模式
	if (!pmgr2)
	{
		m_ImgIO->Close_MgrIn();
		//执行出错
		if (!bYesOrNo)
		{
			return false;
		}
		return true;
	}

	if (!m_ImgIO->Set_MgrOut(pmgr2, m_FileName))
	{
		return false;
	}

	//条件判断，正规输出结果影像
	switch (m_iDataTypeIn)
	{
	case HAP_Byte:
		bYesOrNo = Process<BYTE>();
		break;
	case HAP_UInt16:
		bYesOrNo = Process<unsigned short>();
		break;
	case HAP_Int16:
		bYesOrNo = Process<short int>();
		break;
	case HAP_Int32:
		bYesOrNo = Process<int>();
		break;
	case HAP_Float32:
		bYesOrNo = Process<float>();
		break;
	case HAP_Float64:
		bYesOrNo = Process<double>();
		break;
	default:
		bYesOrNo = false;
		break;
	}
	m_ImgIO->Close_MgrIn();
	m_ImgIO->Close_MgrOut();

	//执行出错
	if (!bYesOrNo)
	{
		return false;
	}
	return true;

}
template <class TT>
bool Base_IPAS::PreparePara(int cpuNo)
{
	return true;
}
template <class TT>
bool Base_IPAS::Process(int cpuNo)
{
	return true;
}
char * Base_IPAS::GetErrorMessage()
{
	return m_pcErrMessage;
}