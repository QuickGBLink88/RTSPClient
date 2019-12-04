#if !defined(AFX_COLORSTATIC_H__181B3431_2DCE_43E3_9763_C31A2C7078DC__INCLUDED_)
#define AFX_COLORSTATIC_H__181B3431_2DCE_43E3_9763_C31A2C7078DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorStatic.h : header file
//


#define WM_SHOW_MAXIMIZED    WM_USER+45

/////////////////////////////////////////////////////////////////////////////
// CVideoDisplayWnd window

class CVideoDisplayWnd : public CWnd
{
// Construction
public:
	CVideoDisplayWnd();

    void SetColors(COLORREF, COLORREF);
	void  EnablePaintVideo(BOOL bEnable);

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoDisplayWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CVideoDisplayWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CVideoDisplayWnd)
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
	BOOL     m_bPaintVideo;
	CBrush   Brush;
	COLORREF m_Fore;
	COLORREF m_Back;
    CFont    m_font;
	BOOL     m_bCustomFont;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORSTATIC_H__181B3431_2DCE_43E3_9763_C31A2C7078DC__INCLUDED_)
