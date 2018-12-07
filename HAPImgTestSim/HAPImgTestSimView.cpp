
// HAPImgTestSimView.cpp : CHAPImgTestSimView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
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
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CHAPImgTestSimView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FILE_OPEN, &CHAPImgTestSimView::OnFileOpen)
END_MESSAGE_MAP()

// CHAPImgTestSimView ����/����

CHAPImgTestSimView::CHAPImgTestSimView()
{
	// TODO:  �ڴ˴���ӹ������
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
	// TODO:  �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CHAPImgTestSimView ����

void CHAPImgTestSimView::OnDraw(CDC* /*pDC*/)
{
	CHAPImgTestSimDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO:  �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CHAPImgTestSimView ��ӡ


void CHAPImgTestSimView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CHAPImgTestSimView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CHAPImgTestSimView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CHAPImgTestSimView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  ��Ӵ�ӡ����е��������
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


// CHAPImgTestSimView ���

#ifdef _DEBUG
void CHAPImgTestSimView::AssertValid() const
{
	CView::AssertValid();
}

void CHAPImgTestSimView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CHAPImgTestSimDoc* CHAPImgTestSimView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHAPImgTestSimDoc)));
	return (CHAPImgTestSimDoc*)m_pDocument;
}
#endif //_DEBUG

// CHAPImgTestSimView ��Ϣ�������
void CHAPImgTestSimView::OnFileOpen()
{
	// TODO:  �ڴ���������������
	CString CImgType = "����ͼ���ļ� (*.hap, *.img, *.bmp, *.tif, *.tiff, *.jpg)|*.hap; *.img; *.bmp; *.tif; *.tiff; *.jpg;*.png|\
					   					   ENVI�ļ�(*.img)|*.img|HAP�ļ�(*.hap)|*.hap|Bitmap Files (*.bmp)|*.bmp|GEOTIFF Files (*.tif, *.TIF,  *.tiff,  *.TIFF)|\
										   										   					   *.tif; *.TIF; *.tiff; *.TIFF|JPEG Files (*.jpg)|*.jpg|�����ļ�(*.*)|*.*||";
	CFileDialog * fileDlg = new CFileDialog(TRUE, "IMG", NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CImgType);
	fileDlg->m_ofn.lpstrTitle = "��HAP���ͼ���ļ�";
	if (fileDlg->DoModal() != IDOK)
	{
		delete fileDlg;
		return;
	}
	CString fileName = fileDlg->GetPathName();
	delete fileDlg;

	//��ֹ�ļ��ظ���
	if (m_FileName == fileName)
	{
		AfxMessageBox("�ļ��Ѿ���!");
		return;
	}

	//�ж��ļ��Ƿ������������е��ã���ֹ����Ϊ��
	CFile file;
	CFileException e;
	if (!file.Open(fileName, CFile::shareDenyNone, &e))
	{
#ifdef _DEBUG
		afxDump << "file could not be opened!" << e.m_cause << "\n";
#endif
		AfxMessageBox("�ļ��Ѿ�������������ʹ�ã����ȹر������������ļ���");
		return;
	}
	ULONGLONG ULLFileSize = file.GetLength();
	file.Close();
	char pcCurTime[20];
	g_CMT.getCurTimeString( pcCurTime );
	//��¼��ǰʱ��
	stHAPProg.writeDebugMessage(pcCurTime);
	stHAPProg.writeDebugMessage(fileName.GetBuffer());

	CGDALFileManager * pMgr = new CGDALFileManager;
	const char * cstr1 = fileName.GetBuffer();
	if (!pMgr->LoadFrom(cstr1))
	{

	}

}
