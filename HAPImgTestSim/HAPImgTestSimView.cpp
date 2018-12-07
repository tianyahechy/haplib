
// HAPImgTestSimView.cpp : CHAPImgTestSimView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "HAPImgTestSim.h"
#endif

#include "HAPImgTestSimDoc.h"
#include "HAPImgTestSimView.h"
#include "MyTimer.h"
#include "comlib.h"
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CHAPImgTestSimView

IMPLEMENT_DYNCREATE(CHAPImgTestSimView, CView)

BEGIN_MESSAGE_MAP(CHAPImgTestSimView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CHAPImgTestSimView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FILE_OPEN, &CHAPImgTestSimView::OnFileOpen)
END_MESSAGE_MAP()

// CHAPImgTestSimView 构造/析构

CHAPImgTestSimView::CHAPImgTestSimView()
{
	// TODO:  在此处添加构造代码
	m_pMgr = NULL;
	m_FileName = "";

}

CHAPImgTestSimView::~CHAPImgTestSimView()
{
	if (m_pMgr != NULL)
	{
		delete m_pMgr;
		m_pMgr = NULL;

	}
	stHAPProg.ProgPo = -1;
}

BOOL CHAPImgTestSimView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CHAPImgTestSimView 绘制

void CHAPImgTestSimView::OnDraw(CDC* /*pDC*/)
{
	CHAPImgTestSimDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO:  在此处为本机数据添加绘制代码
}


// CHAPImgTestSimView 打印


void CHAPImgTestSimView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CHAPImgTestSimView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CHAPImgTestSimView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  添加额外的打印前进行的初始化过程
}

void CHAPImgTestSimView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  添加打印后进行的清理过程
}

void CHAPImgTestSimView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CHAPImgTestSimView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CHAPImgTestSimView 诊断

#ifdef _DEBUG
void CHAPImgTestSimView::AssertValid() const
{
	CView::AssertValid();
}

void CHAPImgTestSimView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CHAPImgTestSimDoc* CHAPImgTestSimView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHAPImgTestSimDoc)));
	return (CHAPImgTestSimDoc*)m_pDocument;
}
#endif //_DEBUG

// CHAPImgTestSimView 消息处理程序
void CHAPImgTestSimView::OnFileOpen()
{
	// TODO:  在此添加命令处理程序代码
	CString CImgType = "所有图像文件 (*.hap, *.img, *.bmp, *.tif, *.tiff, *.jpg)|*.hap; *.img; *.bmp; *.tif; *.tiff; *.jpg;*.png|\
					   					   ENVI文件(*.img)|*.img|HAP文件(*.hap)|*.hap|Bitmap Files (*.bmp)|*.bmp|GEOTIFF Files (*.tif, *.TIF,  *.tiff,  *.TIFF)|\
										   										   					   *.tif; *.TIF; *.tiff; *.TIFF|JPEG Files (*.jpg)|*.jpg|所有文件(*.*)|*.*||";
	CFileDialog * fileDlg = new CFileDialog(TRUE, "IMG", NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CImgType);
	fileDlg->m_ofn.lpstrTitle = "打开HAP相关图像文件";
	if (fileDlg->DoModal() != IDOK)
	{
		delete fileDlg;
		return;
	}
	CString fileName = fileDlg->GetPathName();
	delete fileDlg;

	//防止文件重复打开
	if (m_FileName == fileName)
	{
		AfxMessageBox("文件已经打开!");
		return;
	}

	//判断文件是否在其他程序中调用，防止共享为例
	CFile file;
	CFileException e;
	if (!file.Open(fileName, CFile::shareDenyNone, &e))
	{
#ifdef _DEBUG
		afxDump << "file could not be opened!" << e.m_cause << "\n";
#endif
		AfxMessageBox("文件已经在其他程序中使用，请先关闭其他程序中文件！");
		return;
	}
	ULONGLONG ULLFileSize = file.GetLength();
	file.Close();
	char pcCurTime[20];
	g_CMT.getCurTimeString( pcCurTime );
	//记录当前时间
	stHAPProg.writeDebugMessage(pcCurTime);
	stHAPProg.writeDebugMessage(fileName.GetBuffer());

	CGDALFileManager * pMgr = new CGDALFileManager;
	const char * cstr1 = fileName.GetBuffer();
	if (!pMgr->LoadFrom(cstr1))
	{

	}

}
