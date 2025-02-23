// GPIO-WinGUIDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CGPIOWinGUIDlg dialog
class CGPIOWinGUIDlg : public CDialog
{
// Construction
public:
	CGPIOWinGUIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GPIOWINGUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonSync();
	afx_msg void OnBnClickedButtonConn();
	afx_msg void OnBnClickedButtonScan();
	CComboBox m_comboDevice;
public:
	afx_msg void OnBnClickedButtonAuto();
	BOOL blContinus;
public:
	afx_msg void OnBnClickedButtonGet();
public:
	afx_msg void OnBnClickedSetOnePinmode();
public:
	afx_msg void OnBnClickedSetOnePin();
public:
	afx_msg void OnBnClickedGetOnePin();
	afx_msg void OnBnClickedSetOnePinmode2();
	afx_msg void OnBnClickedSetOnePin2();
	afx_msg void OnBnClickedClearOnePin();
	afx_msg int GetPin();
};
