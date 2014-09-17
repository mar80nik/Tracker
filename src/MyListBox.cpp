// MyListBox.cpp : implementation file
//

#include "stdafx.h"
#include "MyListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MyListBox

MyListBox::MyListBox()
{
}

MyListBox::~MyListBox()
{
}


BEGIN_MESSAGE_MAP(MyListBox, CListBox)
	//{{AFX_MSG_MAP(MyListox)
	ON_MESSAGE(LB_ADDSTRING, OnAddString)
	ON_MESSAGE(LB_INSERTSTRING, OnInsertString)
	ON_MESSAGE(LB_DELETESTRING, OnDeleteString)
	ON_MESSAGE(LB_DIR, OnDir)
	ON_MESSAGE(LB_RESETCONTENT, OnResetContent)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MyListBox message handlers
int MyListBox::AddString( LPCTSTR s, COLORREF col/*=0*/ )
{
    int result = CListBox::AddString(s);
	SetItemData(result,col);
    if(result < 0) return result;
	SetCurSel(GetCount()-1);
    return result;
   }

void MyListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		CDC dc; CString T; int n; COLORREF crOldTextColor, crOldBkColor, crBkColor;
		n = lpDrawItemStruct->itemID;
		if (n < 0) return;
		GetText(lpDrawItemStruct->itemID,T);
		n=GetItemData(lpDrawItemStruct->itemID);

		dc.Attach(lpDrawItemStruct->hDC);		

		crOldTextColor = dc.GetTextColor(); crBkColor = crOldBkColor = dc.GetBkColor();
		
		dc.SetTextColor(n);
		if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
			(lpDrawItemStruct->itemState & ODS_SELECTED))
		{
			crBkColor = ::GetSysColor(COLOR_HIGHLIGHT);
		}

		dc.FillSolidRect(&lpDrawItemStruct->rcItem, crBkColor);
		dc.TextOut(lpDrawItemStruct->rcItem.left,lpDrawItemStruct->rcItem.top,T);

		dc.SetTextColor(crOldTextColor); dc.SetBkColor(crOldBkColor);
		dc.Detach();
}

void MyListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{	
	ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);
	CString Test("Test");
	CSize   sz;
	CDC*    pDC = GetDC();

	sz = pDC->GetTextExtent(Test);
	ReleaseDC(pDC);
	lpMeasureItemStruct->itemHeight = sz.cy;
}

void MyListBox::SetNewHExtent(LPCTSTR lpszNewString)
{
	int iExt = GetTextLen(lpszNewString);
	if (iExt > GetHorizontalExtent())
		SetHorizontalExtent(iExt);
}

int MyListBox::GetTextLen(LPCTSTR lpszText)
{
	ASSERT(AfxIsValidString(lpszText));

	CDC *pDC = GetDC();
	ASSERT(pDC);

	CSize size;
	CFont* pOldFont = pDC->SelectObject(GetFont());
	if ((GetStyle() & LBS_USETABSTOPS) == 0)
	{
		size = pDC->GetTextExtent(lpszText, (int) _tcslen(lpszText));
		size.cx += 3;
	}
	else
	{
		// Expand tabs as well
		size = pDC->GetTabbedTextExtent(lpszText, (int)
			_tcslen(lpszText), 0, NULL);
		size.cx += 2;
	}
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	return size.cx;
}

void MyListBox::ResetHExtent()
{
	if (GetCount() == 0)
	{
		SetHorizontalExtent(0);
		return;
	}

	CWaitCursor cwc;
	int iMaxHExtent = 0;
	for (int i = 0; i < GetCount(); i++)
	{
		CString csText;
		GetText(i, csText);
		int iExt = GetTextLen(csText);
		if (iExt > iMaxHExtent)
			iMaxHExtent = iExt;
	}
	SetHorizontalExtent(iMaxHExtent);
}
///////////////////////////////////////////////////////////////////////////////
// OnAddString: wParam - none, lParam - string, returns - int
///////////////////////////////////////////////////////////////////////////////
LRESULT MyListBox::OnAddString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();
	if (!((lResult == LB_ERR) || (lResult == LB_ERRSPACE)))
		SetNewHExtent((LPCTSTR) lParam);
	return lResult;
}
///////////////////////////////////////////////////////////////////////////////
// OnInsertString: wParam - index, lParam - string, returns - int 
///////////////////////////////////////////////////////////////////////////////
LRESULT MyListBox::OnInsertString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();
	if (!((lResult == LB_ERR) || (lResult == LB_ERRSPACE)))
		SetNewHExtent((LPCTSTR) lParam);
	return lResult;
}
///////////////////////////////////////////////////////////////////////////////
// OnDeleteString: wParam - index, lParam - none, returns - int 
///////////////////////////////////////////////////////////////////////////////
LRESULT MyListBox::OnDeleteString(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();
	if (!((lResult == LB_ERR) || (lResult == LB_ERRSPACE)))
		ResetHExtent();
	return lResult;
}
///////////////////////////////////////////////////////////////////////////////
// OnDir: wParam - attr, lParam - wildcard, returns - int 
///////////////////////////////////////////////////////////////////////////////
LRESULT MyListBox::OnDir(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();
	if (!((lResult == LB_ERR) || (lResult == LB_ERRSPACE)))
		ResetHExtent();
	return lResult;
}
///////////////////////////////////////////////////////////////////////////////
// OnResetContent: wParam - none, lParam - none, returns - int 
///////////////////////////////////////////////////////////////////////////////
LRESULT MyListBox::OnResetContent(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();
	SetHorizontalExtent(0);
	return lResult;
}
int MyListBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

void MyListBox::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CListBox::OnShowWindow(bShow, nStatus);
}

BOOL MyListBox::OnInit()
{
	SetHorizontalExtent(0);
	return TRUE;
}

