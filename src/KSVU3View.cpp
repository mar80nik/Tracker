//

#include "stdafx.h"
#include "KSVU3.h"

#include "KSVU3Doc.h"
#include "KSVU3View.h"
#include ".\ksvu3view.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKSVU3View

IMPLEMENT_DYNCREATE(CKSVU3View, CScrollView)

BEGIN_MESSAGE_MAP(CKSVU3View, CScrollView)
	//{{AFX_MSG_MAP(CKSVU3View)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKSVU3View construction/destruction

CKSVU3View::CKSVU3View() {}

CKSVU3View::~CKSVU3View() {}

//BOOL CKSVU3View::PreCreateWindow(CREATESTRUCT& cs)
//{
//	return CView::PreCreateWindow(cs);
//}

/////////////////////////////////////////////////////////////////////////////
// CKSVU3View drawing

void CKSVU3View::OnDraw(CDC* pDC)
{
	CKSVU3Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);	
}

/////////////////////////////////////////////////////////////////////////////
// CKSVU3View diagnostics

#ifdef _DEBUG

CKSVU3Doc* CKSVU3View::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CKSVU3Doc)));
	return (CKSVU3Doc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CKSVU3View message handlers

void CKSVU3View::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CScrollView::OnShowWindow(bShow, nStatus);	
	// TODO: Add your message handler code here
	
}

void CKSVU3View::OnSetFocus(CWnd* pOldWnd)
{
	CScrollView::OnSetFocus(pOldWnd);

}

void CKSVU3View::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);	

	CMainFrame* mf=(CMainFrame*)AfxGetMainWnd();
	if(mf==NULL) return;
	if(mf->Chart1.m_hWnd!=NULL) 
	{
		if( (mf->Chart1.GetStyle() & WS_VISIBLE) )
		{
			mf->Chart1.SetWindowPos(NULL,0,0,cx,cy,SWP_NOZORDER | SWP_NOMOVE);
			SetScrollSizes(MM_TEXT, CSize(cx, cy));
		}
	}
}

void CKSVU3View::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	SetScrollSizes(MM_TEXT, CSize(100, 100));
}
