
// PharmaRobot 1.0Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "PharmaRobot 1.0.h"
#include "PharmaRobot 1.0Dlg.h"
#include "afxdialogex.h"

//#define __DEBUGPHARMA

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C" {
	//extern int CIOpenStd( char * pszClientName );

}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPharmaRobot10Dlg dialog

CPharmaRobot10Dlg::CPharmaRobot10Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPharmaRobot10Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//m_listBoxMain = (CListBox *)GetDlgItem(IDC_LIST1);
	//GotoDlgCtrl(m_listBoxMain);
}

void CPharmaRobot10Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST1, m_listBoxMain);
	DDX_Control(pDX, IDC_EDIT1, m_EditCounterUnitB);
	DDX_Control(pDX, IDC_EDIT2, m_EditBarCodeB);
	DDX_Control(pDX, IDC_EDIT4, m_EditCounterUnitA);
	DDX_Control(pDX, IDC_EDIT8, m_EditBarCodeA);
	DDX_Control(pDX, IDC_EDIT5, m_EditDispenser);
	DDX_Control(pDX, IDC_EDIT6, m_EditPriority);
	DDX_Control(pDX, IDC_EDIT7, m_EditQuantity);
	DDX_Control(pDX, IDC_EDIT3, m_EditOrderNum);
	DDX_Control(pDX, IDC_BUTTON2, m_ButtonStock);
	DDX_Control(pDX, IDC_BUTTON1, m_ButtonConnect);
	DDX_Control(pDX, IDC_BUTTON3, m_ButtonDispense);
	DDX_Control(pDX, IDC_TAB1, m_TabControl);

	DDX_Control(pDX, IDC_STATIC1, m_Static1);
	DDX_Control(pDX, IDC_STATIC2, m_Static2);
	DDX_Control(pDX, IDC_STATIC3, m_Static3);
	DDX_Control(pDX, IDC_STATIC4, m_Static4);
	DDX_Control(pDX, IDC_STATIC5, m_Static5);
	DDX_Control(pDX, IDC_STATIC6, m_Static6);
	DDX_Control(pDX, IDC_STATIC7, m_Static7);
	DDX_Control(pDX, IDC_STATIC8, m_Static8);
	DDX_Control(pDX, IDC_STATIC9, m_Static9);
	DDX_Control(pDX, IDC_STATIC10, m_Static10);
	//*/

	DDX_Control(pDX, IDC_BUTTONGetSQLDesc, m_ButtonGetSQLDesc);
	DDX_Control(pDX, IDC_LISTSQLDesc, m_ListSQLDesc);
	DDX_Control(pDX, IDC_STATICBarcodeSQL, m_StaticBarcodeSQL);
	DDX_Control(pDX, IDC_EDITBarcodeSQL, m_EditBarcodeSQL);

}

BEGIN_MESSAGE_MAP(CPharmaRobot10Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CPharmaRobot10Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPharmaRobot10Dlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CPharmaRobot10Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPharmaRobot10Dlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CPharmaRobot10Dlg::OnBnClickedButton3)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CPharmaRobot10Dlg::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDC_BUTTONGetSQLDesc, &CPharmaRobot10Dlg::OnBnClickedButtongetsqldesc)
END_MESSAGE_MAP()


// CPharmaRobot10Dlg message handlers

BOOL CPharmaRobot10Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	nidApp.cbSize = sizeof(NOTIFYICONDATA); 
	nidApp.hWnd = (HWND)m_hWnd;
	nidApp.uID = IDI_SYSTRAYDEMO; 
	nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; 
	nidApp.hIcon = m_hIcon; 
	nidApp.uCallbackMessage = WM_SYSCOMMAND; 
	wcscpy_s(nidApp.szTip, _T("יעל היפה בנשים"));
	m_OrderNum = 0;

	Shell_NotifyIcon(NIM_ADD, &nidApp);

	// TODO: Add extra initialization here
	m_EditBarCodeB.SetWindowTextW(L"1234567890123");
	m_EditCounterUnitB.SetWindowTextW(L"1");
	m_EditCounterUnitA.SetWindowTextW(L"1");
	m_EditBarCodeA.SetWindowTextW(L"1234567890123");
	m_EditDispenser.SetWindowTextW(L"1");
	m_EditPriority.SetWindowTextW(L"3");
	m_EditQuantity.SetWindowTextW(L"1");
	m_EditBarcodeSQL.SetWindowTextW(L"7290004239988");
	WCHAR strQuan[10];
	wsprintf(strQuan,L"%d",m_OrderNum);
	m_EditOrderNum.SetWindowTextW(strQuan);

	TCITEM tcItem;
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("CONSIS");

	m_TabControl.InsertItem(0,&tcItem);

	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("SQL");

	m_TabControl.InsertItem(1,&tcItem);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPharmaRobot10Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == 0xF060)
	{
		Shell_NotifyIcon(NIM_DELETE,&nidApp);
	}

	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if ((nID & 0xFFF0) == IDI_SYSTRAYDEMO )
	{
		switch (LOWORD(lParam)){
		case WM_LBUTTONDBLCLK:
			{
				//					char strClientname[6] = {'S','h','m','o','r','T'};
			}
			break;
		default:
			{

			}
			break;
		}
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPharmaRobot10Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPharmaRobot10Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPharmaRobot10Dlg::OnBnClickedOk()
{
	Shell_NotifyIcon(NIM_DELETE,&nidApp);
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CPharmaRobot10Dlg::OnBnClickedCancel()
{
	Shell_NotifyIcon(NIM_DELETE,&nidApp);

	CDialogEx::OnCancel();
}


void CPharmaRobot10Dlg::OnBnClickedButton1()
{
	m_listBoxMain.AddString(L"Button Pressed");
	if (Consis.ConnectToConsis("ShorT", &m_listBoxMain))
	{
		m_EditCounterUnitB.EnableWindow();
		m_EditBarCodeB.EnableWindow();
		m_ButtonStock.EnableWindow();
		m_EditCounterUnitA.EnableWindow();
		m_EditBarCodeA.EnableWindow();
		m_EditDispenser.EnableWindow();
		m_EditPriority.EnableWindow();
		m_EditQuantity.EnableWindow();
		m_ButtonDispense.EnableWindow();
		m_ButtonConnect.EnableWindow(FALSE);
	}
}

void CPharmaRobot10Dlg::OnBnClickedButton2()
{
	int fieldLength;
	wchar_t wcstring[100];
	WCHAR orig[14];
	char nstring[100];

	memset(ConsisMessage, '0', 41);
	ConsisMessage[41] = '\0';

	size_t convertedChars = 0;

	if(m_EditCounterUnitB.GetWindowTextLengthW() > 3)
	{
		m_listBoxMain.AddString(L"Bad Counter unit to big (up to 3 digits)");
		return;
	}
	if(m_EditBarCodeB.GetWindowTextLengthW() > 13)
	{
		m_listBoxMain.AddString(L"Bad Barcode number (up to 13 digits)");
		return;
	}

	/*Counter Unit*/
	size_t origsize = m_EditCounterUnitB.GetWindowTextLengthW() + 1;
	fieldLength = m_EditCounterUnitB.GetWindowTextLengthW();
	m_EditCounterUnitB.GetWindowTextW(orig,origsize);
	wcstombs_s(&convertedChars, nstring, origsize, orig , _TRUNCATE);
	int location = 4 - m_EditCounterUnitB.GetWindowTextLengthW();

	memcpy(&(ConsisMessage[location]), nstring, origsize - 1);

	/*Barcode*/
	origsize = m_EditBarCodeB.GetWindowTextLengthW() + 1;
	m_EditBarCodeB.GetWindowTextW(orig,origsize);
	wcstombs_s(&convertedChars, nstring, origsize, orig , _TRUNCATE);
	location = 41 - m_EditBarCodeB.GetWindowTextLengthW();

	memcpy((void*)&(ConsisMessage[location]), (void*) nstring, origsize - 1);

	ConsisMessage[0] = 'B';

	mbstowcs_s(&convertedChars, wcstring, 42, ConsisMessage, _TRUNCATE);

	m_listBoxMain.AddString(wcstring);

	Consis.SendStockQuery(ConsisMessage);
}


void CPharmaRobot10Dlg::OnBnClickedButton3()
{
	wchar_t wcstring[100];
	WCHAR orig[14];
	char nstring[100];

	memset(ConsisMessage, '0', 512);
	ConsisMessage[61] = '\0';

	size_t convertedChars = 0;

	if(m_EditCounterUnitA.GetWindowTextLengthW() > 3)
	{
		m_listBoxMain.AddString(L"Bad Counter unit to big (up to 3 digits)");
		return;
	}
	if(m_EditBarCodeA.GetWindowTextLengthW() > 13)
	{
		m_listBoxMain.AddString(L"Bad Barcode number (up to 13 digits)");
		return;
	}
	if(m_EditDispenser.GetWindowTextLengthW() > 3)
	{
		m_listBoxMain.AddString(L"Bad Dispemser number (up to 3 digits)");
		return;
	}
	if(m_EditPriority.GetWindowTextLengthW() > 1)
	{
		m_listBoxMain.AddString(L"Bad Priority (one digit)");
		return;
	}
	if(m_EditQuantity.GetWindowTextLengthW() > 5)
	{
		m_listBoxMain.AddString(L"Bad Quantity (up to 5 digits)");
		return;
	}


	/*Counter Unit*/
	size_t origsize = m_EditCounterUnitA.GetWindowTextLengthW() + 1;
	m_EditCounterUnitA.GetWindowTextW(orig,origsize);
	wcstombs_s(&convertedChars, nstring, origsize, orig , _TRUNCATE);
	int location = 12 - (origsize - 1);

	memcpy(&(ConsisMessage[location]), nstring, (origsize - 1));

	m_OrderNum++; if (m_OrderNum > 99999999) m_OrderNum = 0;//for every request increase number
	WCHAR strQuan[10];
	wsprintf(strQuan,L"%d",m_OrderNum);
	m_EditOrderNum.SetWindowTextW(strQuan);

	/*Barcode*/
	origsize = m_EditBarCodeA.GetWindowTextLengthW() + 1;
	m_EditBarCodeA.GetWindowTextW(orig,origsize);
	wcstombs_s(&convertedChars, nstring, origsize, orig , _TRUNCATE);
	location = 60 - (origsize - 1);

	memcpy((void*)&(ConsisMessage[location]), (void*) nstring, (origsize - 1));

	/*Order number*/
	origsize = m_EditOrderNum.GetWindowTextLengthW() + 1;
	m_EditOrderNum.GetWindowTextW(orig,origsize);
	wcstombs_s(&convertedChars, nstring, origsize, orig , _TRUNCATE);
	location = 9 - (origsize - 1);

	memcpy((void*)&(ConsisMessage[location]), (void*) nstring, (origsize - 1));

	/*Quantity*/
	origsize = m_EditQuantity.GetWindowTextLengthW() + 1;
	m_EditQuantity.GetWindowTextW(orig,origsize);
	wcstombs_s(&convertedChars, nstring, origsize, orig , _TRUNCATE);
	location = 30 - (origsize - 1);

	memcpy((void*)&(ConsisMessage[location]), (void*) nstring, (origsize - 1));

	/*Priority*/
	origsize = m_EditPriority.GetWindowTextLengthW() + 1;
	m_EditPriority.GetWindowTextW(orig,origsize);
	wcstombs_s(&convertedChars, nstring, origsize, orig , _TRUNCATE);
	location = 16 - (origsize - 1);

	memcpy((void*)&(ConsisMessage[location]), (void*) nstring, (origsize - 1));

	/*Dispenser*/
	origsize = m_EditDispenser.GetWindowTextLengthW() + 1;
	m_EditDispenser.GetWindowTextW(orig,origsize);
	wcstombs_s(&convertedChars, nstring, origsize, orig , _TRUNCATE);
	location = 15 - (origsize - 1);

	memcpy((void*)&(ConsisMessage[location]), (void*) nstring, (origsize - 1));

	ConsisMessage[0] = 'A';

	mbstowcs_s(&convertedChars, wcstring, 61, ConsisMessage, _TRUNCATE);

	m_listBoxMain.AddString(wcstring);

	Consis.SendDispnseCommand(ConsisMessage);
}


void CPharmaRobot10Dlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_TabControl.GetCurSel() == 0)
	{
		m_EditCounterUnitB.ShowWindow(SW_SHOW);
		m_EditBarCodeB.ShowWindow(SW_SHOW);
		m_EditCounterUnitA.ShowWindow(SW_SHOW);
		m_EditBarCodeA.ShowWindow(SW_SHOW);
		m_EditDispenser.ShowWindow(SW_SHOW);
		m_EditPriority.ShowWindow(SW_SHOW);
		m_EditQuantity.ShowWindow(SW_SHOW);
		m_EditOrderNum.ShowWindow(SW_SHOW);
		m_ButtonStock.ShowWindow(SW_SHOW);
		m_ButtonDispense.ShowWindow(SW_SHOW);

		m_Static1.ShowWindow(SW_SHOW);
		m_Static2.ShowWindow(SW_SHOW);
		m_Static3.ShowWindow(SW_SHOW);
		m_Static4.ShowWindow(SW_SHOW);
		m_Static5.ShowWindow(SW_SHOW);
		m_Static6.ShowWindow(SW_SHOW);
		m_Static7.ShowWindow(SW_SHOW);
		m_Static8.ShowWindow(SW_SHOW);
		m_Static9.ShowWindow(SW_SHOW);
		m_Static10.ShowWindow(SW_SHOW);

		m_ListSQLDesc.ShowWindow(SW_HIDE);
		m_EditBarcodeSQL.ShowWindow(SW_HIDE);
		m_ButtonGetSQLDesc.ShowWindow(SW_HIDE);
		m_StaticBarcodeSQL.ShowWindow(SW_HIDE);
	}

	else
	{
		m_EditCounterUnitB.ShowWindow(SW_HIDE);
		m_EditBarCodeB.ShowWindow(SW_HIDE);
		m_EditCounterUnitA.ShowWindow(SW_HIDE);
		m_EditBarCodeA.ShowWindow(SW_HIDE);
		m_EditDispenser.ShowWindow(SW_HIDE);
		m_EditPriority.ShowWindow(SW_HIDE);
		m_EditQuantity.ShowWindow(SW_HIDE);
		m_EditOrderNum.ShowWindow(SW_HIDE);
		m_ButtonStock.ShowWindow(SW_HIDE);
		m_ButtonDispense.ShowWindow(SW_HIDE);

		m_Static1.ShowWindow(SW_HIDE);
		m_Static2.ShowWindow(SW_HIDE);
		m_Static3.ShowWindow(SW_HIDE);
		m_Static4.ShowWindow(SW_HIDE);
		m_Static5.ShowWindow(SW_HIDE);
		m_Static6.ShowWindow(SW_HIDE);
		m_Static7.ShowWindow(SW_HIDE);
		m_Static8.ShowWindow(SW_HIDE);
		m_Static9.ShowWindow(SW_HIDE);
		m_Static10.ShowWindow(SW_HIDE);

		m_ListSQLDesc.ShowWindow(SW_SHOW);
		m_EditBarcodeSQL.ShowWindow(SW_SHOW);
		m_ButtonGetSQLDesc.ShowWindow(SW_SHOW);
		m_StaticBarcodeSQL.ShowWindow(SW_SHOW);

	}
	*pResult = 0;
}


void CPharmaRobot10Dlg::OnBnClickedButtongetsqldesc()
{
	wchar_t StringSQL[100], st[100];

	m_YarpaDb.OpenEx( _T( "ODBC;DSN=PT;SERVER=192.168.10.5;DATABASE=PIRYON;UID=sa;PWD=1234;TABLE=Xitems"),
		CDatabase::openReadOnly |
		CDatabase::noOdbcDialog);

	if(m_EditBarcodeSQL.GetWindowTextLengthW() > 13)
	{
		m_listBoxMain.AddString(L"Bad Barcode number (up to 13 digits)");
		return;
	}

	m_EditBarcodeSQL.GetWindowTextW(StringSQL,m_EditBarcodeSQL.GetWindowTextLengthW() + 1);

#ifdef __DEBUGPHARMA
	wsprintf(st,L"SELECT DISTINCT * FROM XITEMS WHERE BARCODE='%s';ORDER BY BARCODE;\0",StringSQL);
	m_listBoxMain.AddString(st);
#endif

	CRecordset rs(&m_YarpaDb);
	if (rs.Open(CRecordset::snapshot,StringSQL,CRecordset::readOnly))
	{
		// Create a CDBVariant object to
		// store field data
		CDBVariant varValue;
		// Loop through the recordset,
		// using GetFieldValue and
		// GetODBCFieldCount to retrieve
		// data in all columns
		short nFields = rs.GetODBCFieldCount();
		while(!rs.IsEOF())
		{
			for(short index = 0; index < nFields; index++)
			{
				rs.GetFieldValue(index, varValue);
				m_ListSQLDesc.AddString(varValue.m_pstringW->GetString());
			}
			rs.MoveNext();
		}

		rs.Close();
	}
	else
	{
		m_listBoxMain.AddString(L"Failed to read Barcode from SQL");
	}

	m_YarpaDb.Close();
}

void CTabPharms::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
	TC_ITEM tci;
	CDC* pDC = CDC::FromHandle(lpdis->hDC);
	CRect rect(lpdis->rcItem);
	wchar_t szTabText[256];
	memset(szTabText,'\0',sizeof(szTabText));

	tci.mask = TCIF_TEXT;
	tci.pszText = szTabText;
	tci.cchTextMax = sizeof(szTabText) -1;
	GetItem(lpdis->itemID, &tci);

	if (lpdis->itemAction & ODA_DRAWENTIRE)
	{
		pDC->TextOut(rect.left+5, rect.top+5, tci.pszText);
	}

	if ((lpdis->itemState & ODS_SELECTED) && 
		(lpdis->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		//Make the color of text of the selected tab to be BLUE.
		pDC->SetTextColor(RGB(0,0,255));
		pDC->TextOut(rect.left+5, rect.top+5, tci.pszText);
	}
}