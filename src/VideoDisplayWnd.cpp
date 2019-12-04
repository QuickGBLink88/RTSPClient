// ColorStatic.cpp : implementation file
//

#include "stdafx.h"
#include "VideoDisplayWnd.h"
#include ".\videodisplaywnd.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CVideoDisplayWnd::CVideoDisplayWnd()
{
	Brush.CreateSolidBrush(RGB(0, 0, 0));
	m_Back = RGB(0, 0, 0);
	m_Fore = RGB(0, 0, 0);
	m_bCustomFont = FALSE;
	m_bPaintVideo = FALSE;
}

CVideoDisplayWnd::~CVideoDisplayWnd()
{
	Brush.DeleteObject();
	m_font.DeleteObject();
}


BEGIN_MESSAGE_MAP(CVideoDisplayWnd, CWnd)
	//{{AFX_MSG_MAP(CVideoDisplayWnd)
    ON_WM_PAINT()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	//ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVideoDisplayWnd message handlers

void CVideoDisplayWnd::SetColors(COLORREF crFore, COLORREF crBack)
{
	m_Back = crBack;
	m_Fore = crFore;
	Brush.DeleteObject();
	Brush.CreateSolidBrush(crBack);
	Invalidate();
}


void CVideoDisplayWnd::OnPaint()
{
	CPaintDC dc(this);

	RECT rect;
	GetClientRect(&rect);
	dc.FillSolidRect(&rect, m_Back);

}

void CVideoDisplayWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
    
    CWnd::OnLButtonDblClk(nFlags, point);
}


void CVideoDisplayWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	//TRACE("CVideoDisplayWnd:: OnSize() %d, %d\n", cx, cy);
}

BOOL CVideoDisplayWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
