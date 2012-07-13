
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
	char StockMessage[42];
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
