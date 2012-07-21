
// PharmaRobot 1.0Dlg.h : header file
//

#pragma once
#include "ConsisComm.h"

// CPharmaRobot10Dlg dialog
class CPharmaRobot10Dlg : public CDialogEx
{
// Construction
public:
	CPharmaRobot10Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PHARMAROBOT10_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA nidApp;
	ConsisComm Consis;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	CListBox m_listBoxMain;
	CEdit    m_EditCounterUnitB;
	CEdit    m_EditBarCodeB;
	CEdit    m_EditCounterUnitA;
	CEdit    m_EditBarCodeA;
	CEdit    m_EditDispenser;
	CEdit    m_EditPriority;
	CEdit    m_EditQuantity;
	CEdit    m_EditOrderNum;
	CButton  m_ButtonConnect;
	CButton  m_ButtonStock;
	CButton  m_ButtonDispense;
	CTabCtrl m_TabControl;

	CStatic  m_Static1;
	CStatic  m_Static2;
	CStatic  m_Static3;
	CStatic  m_Static4;
	CStatic  m_Static5;
	CStatic  m_Static6;
	CStatic  m_Static7;
	CStatic  m_Static8;	
	CStatic  m_Static9;
	CStatic  m_Static10;

	char     ConsisMessage[512];
	int      m_OrderNum;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};
