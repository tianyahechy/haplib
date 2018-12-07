
// HAPImgTestSimView.h : CHAPImgTestSimView 类的接口
//

#pragma once
#include "GDALFileManager.h"
#include "ProgDlg.h"

class CHAPImgTestSimView : public CView
{
protected: // 仅从序列化创建
	CHAPImgTestSimView();
	DECLARE_DYNCREATE(CHAPImgTestSimView)

// 特性
public:
	CHAPImgTestSimDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CHAPImgTestSimView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//打开的文件路径
	CString m_FileName;
	//打开的影像文件
	CGDALFileManager * m_pMgr;
	//进度条指示对话框
	CProgressDlg * m_pProgdlg;
// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileOpen();
};

#ifndef _DEBUG  // HAPImgTestSimView.cpp 中的调试版本
inline CHAPImgTestSimDoc* CHAPImgTestSimView::GetDocument() const
   { return reinterpret_cast<CHAPImgTestSimDoc*>(m_pDocument); }
#endif

