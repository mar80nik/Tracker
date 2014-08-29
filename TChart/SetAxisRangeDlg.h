#pragma once

#include "resource.h"

class SetAxisRangeDlg : public CDialog
{
	DECLARE_DYNAMIC(SetAxisRangeDlg)

public:
	SetAxisRangeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SetAxisRangeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG19 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double MaxY;
	double MinY;
};
