#pragma once


// OwnerDrawBtn

class OwnerDrawBtn : public CButton
{
	DECLARE_DYNAMIC(OwnerDrawBtn)

public:
	COLORREF FaceColor,lastFaceColor;
	CBrush FaceBrush;
	
	OwnerDrawBtn();
	virtual ~OwnerDrawBtn();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


