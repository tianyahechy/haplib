
// testHaplibView.cpp : CtestHaplibView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "testHaplib.h"
#endif

#include "testHaplibDoc.h"
#include "testHaplibView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CtestHaplibView

IMPLEMENT_DYNCREATE(CtestHaplibView, CView)

BEGIN_MESSAGE_MAP(CtestHaplibView, CView)
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CtestHaplibView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CtestHaplibView ����/����

CtestHaplibView::CtestHaplibView()
{
	// TODO:  �ڴ˴���ӹ������

}

CtestHaplibView::~CtestHaplibView()
{
}

BOOL CtestHaplibView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CtestHaplibView ����

void CtestHaplibView::OnDraw(CDC* /*pDC*/)
{
	CtestHaplibDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO:  �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CtestHaplibView ��ӡ


void CtestHaplibView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CtestHaplibView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CtestHaplibView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CtestHaplibView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  ��Ӵ�ӡ����е��������
}

void CtestHaplibView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CtestHaplibView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CtestHaplibView ���

#ifdef _DEBUG
void CtestHaplibView::AssertValid() const
{
	CView::AssertValid();
}

void CtestHaplibView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CtestHaplibDoc* CtestHaplibView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CtestHaplibDoc)));
	return (CtestHaplibDoc*)m_pDocument;
}
#endif //_DEBUG


// CtestHaplibView ��Ϣ�������
