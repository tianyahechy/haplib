
// HAPImgTestSimView.h : CHAPImgTestSimView ��Ľӿ�
//

#pragma once
#include "GDALFileManager.h"
#include "ProgDlg.h"

class CHAPImgTestSimView : public CView
{
protected: // �������л�����
	CHAPImgTestSimView();
	DECLARE_DYNCREATE(CHAPImgTestSimView)

// ����
public:
	CHAPImgTestSimDoc* GetDocument() const;

// ����
public:

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~CHAPImgTestSimView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//�򿪵��ļ�·��
	CString m_FileName;
	//�򿪵�Ӱ���ļ�
	CGDALFileManager * m_pMgr;
	//������ָʾ�Ի���
	CProgressDlg * m_pProgdlg;
// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileOpen();
};

#ifndef _DEBUG  // HAPImgTestSimView.cpp �еĵ��԰汾
inline CHAPImgTestSimDoc* CHAPImgTestSimView::GetDocument() const
   { return reinterpret_cast<CHAPImgTestSimDoc*>(m_pDocument); }
#endif

