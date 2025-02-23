// GPIO-WinGUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GPIO-WinGUI.h"
#include "GPIO-WinGUIDlg.h"

#include "SLABHIDDevice.h"
#pragma comment (lib, "SLABHIDDevice.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define VID								0x10C4
#define PID								0xEAC9

// HID Report IDs
#define ID_IN_CONTROL					0xFE
#define ID_OUT_CONTROL					0xFD
#define ID_IN_DATA						0x01
#define ID_OUT_DATA						0x02

#define MAX_GPIO_NUM  20
#define SYNC_TIMER    0
#define SYNC_TIME     100

HANDLE m_hid;
BOOL blConnected = FALSE;
BYTE report[512], getreport[512];

CGPIOWinGUIDlg::CGPIOWinGUIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGPIOWinGUIDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGPIOWinGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_comboDevice);
}

BEGIN_MESSAGE_MAP(CGPIOWinGUIDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SYNC, &CGPIOWinGUIDlg::OnBnClickedButtonSync)
	ON_BN_CLICKED(IDC_BUTTON_CONN, &CGPIOWinGUIDlg::OnBnClickedButtonConn)
	ON_BN_CLICKED(IDC_BUTTON_SCAN, &CGPIOWinGUIDlg::OnBnClickedButtonScan)
	ON_BN_CLICKED(IDC_BUTTON_AUTO, &CGPIOWinGUIDlg::OnBnClickedButtonAuto)
	ON_BN_CLICKED(IDC_BUTTON_GET, &CGPIOWinGUIDlg::OnBnClickedButtonGet)
	ON_BN_CLICKED(IDC_SET_ONE_PINMODE, &CGPIOWinGUIDlg::OnBnClickedSetOnePinmode)
	ON_BN_CLICKED(IDC_SET_ONE_PIN, &CGPIOWinGUIDlg::OnBnClickedSetOnePin)
	ON_BN_CLICKED(IDC_GET_ONE_PIN, &CGPIOWinGUIDlg::OnBnClickedGetOnePin)
	ON_BN_CLICKED(IDC_CLEAR_ONE_PIN, &CGPIOWinGUIDlg::OnBnClickedClearOnePin)
END_MESSAGE_MAP()


// CGPIOWinGUIDlg message handlers

BOOL CGPIOWinGUIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SetTimer( SYNC_TIMER, SYNC_TIME, NULL );
	((CButton*)GetDlgItem(IDC_RADIO_DIS))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_RADIO_INPUT))->SetCheck(1);
    ((CButton*)GetDlgItem(IDC_RADIO_OPENDRAIN))->SetCheck(1);
	GetDlgItem(IDC_EDIT_PIN)->SetWindowText("8");
	blContinus = 0;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGPIOWinGUIDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGPIOWinGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int pullmode_old = 0;
int portinoutmode_old[MAX_GPIO_NUM] = {0};
int portoutputmode_old[MAX_GPIO_NUM] = {0};
int portoutputpin_old[MAX_GPIO_NUM] = {0};

void CGPIOWinGUIDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	int changed = 0;

	if( pullmode_old != ((CButton*)GetDlgItem(IDC_RADIO_EN))->GetCheck() )
	{
		pullmode_old = ((CButton*)GetDlgItem(IDC_RADIO_EN))->GetCheck();
		changed = 1;
	}

	for( int i = 0; i < MAX_GPIO_NUM; i++ ) // Output or input
	{
		if( portinoutmode_old[i] != ((CButton*)GetDlgItem(IDC_CHECK_GPIO40+i))->GetCheck())
		{
			portinoutmode_old[i] = ((CButton*)GetDlgItem(IDC_CHECK_GPIO40+i))->GetCheck();
			changed = 1;

			if( 1 == ((CButton*)GetDlgItem(IDC_CHECK_GPIO40+i))->GetCheck() )
			{
				GetDlgItem(IDC_CHECK_GPIO0+i)->EnableWindow(1);
				GetDlgItem(IDC_CHECK_GPIO20+i)->EnableWindow(1);
			}
			else
			{
				GetDlgItem(IDC_CHECK_GPIO0+i)->EnableWindow(0);
				GetDlgItem(IDC_CHECK_GPIO20+i)->EnableWindow(0);
				((CButton*)GetDlgItem(IDC_CHECK_GPIO0+i))->SetCheck(0);
				((CButton*)GetDlgItem(IDC_CHECK_GPIO20+i))->SetCheck(1);
			}
		}
	}

	for( int i = 0; i < MAX_GPIO_NUM; i++ ) // Open-drain or push-pull
	{
		if( portoutputmode_old[i] != ((CButton*)GetDlgItem(IDC_CHECK_GPIO0+i))->GetCheck())
		{
			portoutputmode_old[i] = ((CButton*)GetDlgItem(IDC_CHECK_GPIO0+i))->GetCheck();
			changed = 1;
		}
	}

	for( int i = 0; i < MAX_GPIO_NUM; i++ ) // Output 0 or 1
	{
		if( portoutputpin_old[i] != ((CButton*)GetDlgItem(IDC_CHECK_GPIO20+i))->GetCheck())
		{
			portoutputpin_old[i] = ((CButton*)GetDlgItem(IDC_CHECK_GPIO20+i))->GetCheck();
			changed = 1;
		}
	}

	if( changed )
	{
		changed = 0;
	}

	if( blContinus )
	{
		OnBnClickedButtonSync();
	}
}

void CGPIOWinGUIDlg::OnBnClickedButtonConn()
{
	// TODO: Add your control notification handler code here
	if( blConnected == FALSE )
	{
		int sel = m_comboDevice.GetCurSel();
		if (sel >= 0)
		{
			BYTE status = HidDevice_Open(&m_hid, 0, VID, PID, MAX_REPORT_REQUEST_XP);
			if( status != HID_DEVICE_SUCCESS )
			{
				AfxMessageBox( "Device open failed" );
				return;
			}

			HidDevice_SetTimeouts( m_hid, 1000, 500 ); // get/set timeout
			blConnected = TRUE;

			m_comboDevice.EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_CONN)->SetWindowText("Disconnect");
		}
	}
	else
	{
		if( blConnected == TRUE )
		{
			m_comboDevice.EnableWindow(TRUE);

			if( m_hid != NULL )
				HidDevice_Close(m_hid);
			m_hid = NULL;
			blConnected = FALSE;
			GetDlgItem(IDC_BUTTON_CONN)->SetWindowText("Connect");
			OnBnClickedButtonScan();
		}
	}
}

void CGPIOWinGUIDlg::OnBnClickedButtonScan()
{
	// TODO: Add your control notification handler code here
	char deviceString[MAX_PATH_LENGTH];
	int devnum = HidDevice_GetNumHidDevices(VID, PID);
	m_comboDevice.ResetContent();

	for( int i = 0; i < devnum; i++ )
	{
		if( HidDevice_GetHidString(i, VID, PID, HID_PATH_STRING, deviceString, MAX_PATH_LENGTH) == HID_DEVICE_SUCCESS )
		{
			m_comboDevice.AddString(deviceString );
		}
	}

	if( m_comboDevice.GetCount() != 0 )
	{
		// If selText was not found in the combo
		// then select the first item
		m_comboDevice.SetCurSel(0);
	}
}

void CGPIOWinGUIDlg::OnBnClickedButtonAuto()
{
	// TODO: Add your control notification handler code here
	if( blContinus )
	{
		blContinus = 0;
		GetDlgItem(IDC_BUTTON_SYNC)->EnableWindow();
	}
	else
	{
		blContinus = 1;
		GetDlgItem(IDC_BUTTON_SYNC)->EnableWindow(0);
	}
}

void CGPIOWinGUIDlg::OnBnClickedButtonSync()
{
	// TODO: Add your control notification handler code here
	int inputmode = 0, outputmode = 0, outpin = 0;

	if( m_hid == NULL )
	{
		return;
	}

	for( int i = 0; i < MAX_GPIO_NUM; i++ )
	{
		if( portinoutmode_old[ i ] )
		{
			inputmode |= (1<<i);
		}

		if(	portoutputmode_old[ i ] )
		{
			outputmode |= (1<<i);
		}

		if(	portoutputpin_old[ i ] ) //| (portinoutmode_old[i] == 0) )
		{
			outpin |= (1<<i);
		}
	}

	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  8;   // Number
	report[ 3 ] =  0x50;// Command
	report[ 4 ] = pullmode_old;   // pull up 0-enable, no 0-disable
	report[ 5 ] = inputmode>>12; // 0-output 1-input
	report[ 6 ] = inputmode>>4;
	report[ 7 ] = inputmode&0x0f;
	report[ 8 ] = outputmode>>12; // 0-opendrain, 1-pushpull
	report[ 9 ] = outputmode>>4;
	report[ 10 ] = outputmode&0x0f;
	report[ 11 ] = outpin>>12;     // 0-out low, 1-out high or input(with open drain)
	report[ 12 ] = outpin>>4;
	report[ 13 ] = outpin&0x0f;
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
		int tmpinport = (getreport[3]<<12) | (getreport[4] <<4) | getreport[5];
		int tmpportmode = (getreport[6]<<12) | (getreport[7] <<4) | getreport[8];
		int tmpportpin = (getreport[9]<<12) | (getreport[10] <<4) | getreport[11];
		for( int i = 0; i < MAX_GPIO_NUM; i++ )
		{
			if( tmpinport & (1<<i) )
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO40+i))->SetCheck(1);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO40+i))->SetCheck(0);
			}

			if( tmpportpin & (1<<i) )
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO20+i))->SetCheck(1);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO20+i))->SetCheck(0);
			}

			if( tmpportmode & (1<<i) )
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO0+i))->SetCheck(1);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO0+i))->SetCheck(0);
			}
		}
	}
	else
	{
		// Close and open
		if( m_hid != NULL )
		{
			HidDevice_Close(m_hid); m_hid = NULL;
		}
	}
}

void CGPIOWinGUIDlg::OnBnClickedButtonGet()
{
	// TODO: Add your control notification handler code here
	if( m_hid == NULL )
	{
		return;
	}

	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  1;   // Number
	report[ 3 ] =  0x51;// Command
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
		if( getreport[2] ) // 0-pull up Enable
		{
			((CButton*)GetDlgItem(IDC_RADIO_EN))->SetCheck(0);
			((CButton*)GetDlgItem(IDC_RADIO_DIS))->SetCheck(1);
		}
		else
		{
			((CButton*)GetDlgItem(IDC_RADIO_EN))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_RADIO_DIS))->SetCheck(0);			
		}

		int tmpinport = (getreport[3]<<12) | (getreport[4] <<4) | getreport[5];
		int tmpportmode = (getreport[6]<<12) | (getreport[7] <<4) | getreport[8];
		int tmpportpin = (getreport[9]<<12) | (getreport[10] <<4) | getreport[11];
		for( int i = 0; i < MAX_GPIO_NUM; i++ )
		{
			if( tmpinport & (1<<i) )
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO40+i))->SetCheck(1);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO40+i))->SetCheck(0);
			}

			if( tmpportpin & (1<<i) )
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO20+i))->SetCheck(1);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO20+i))->SetCheck(0);
			}

			if( tmpportmode & (1<<i) )
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO0+i))->SetCheck(1);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_CHECK_GPIO0+i))->SetCheck(0);
			}
		}
	}
}

int CGPIOWinGUIDlg::GetPin()
{
	CString str;
	GetDlgItem(IDC_EDIT_PIN)->GetWindowText( str );

	int pin = atoi(str);

	return pin;
}

void CGPIOWinGUIDlg::OnBnClickedSetOnePinmode()
{
	// TODO: Add your control notification handler code here
	if( m_hid == NULL )
	{
		return;
	}

	int pin = GetPin();
	if( pin >= MAX_GPIO_NUM )
	{
		AfxMessageBox( "Pin is wrong. (Pin<20)" );
		return;
	}

	int output = 0;
	if( ((CButton*)GetDlgItem(IDC_RADIO_OUTPUT))->GetCheck() )
	{
		output = 1;
	}

	int opend = 0;
	if( ((CButton*)GetDlgItem(IDC_RADIO_PUSHPULL))->GetCheck() )
	{
		opend = 1;
	}

	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  5;   // Number
	report[ 3 ] =  0x52;// Command
	report[ 4 ] =  pin;  // pin
	report[ 5 ] =  output;   // 0-input or 1-output mode
	report[ 6 ] =  opend;   // 0-Open drain or 1-pushpull
	report[ 7 ] =  1;   // 0-low or 1-high
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
	}
}

void CGPIOWinGUIDlg::OnBnClickedSetOnePin()
{
	// TODO: Add your control notification handler code here
	if( m_hid == NULL )
	{
		return;
	}

	int pin = GetPin();
	if( pin >= MAX_GPIO_NUM )
	{
		AfxMessageBox( "Pin is wrong. (Pin<20)" );
		return;
	}

	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  3;   // Number
	report[ 3 ] =  0x53;// Command
	report[ 4 ] =  pin;  // pin
	report[ 5 ] =  1;   // 0-low, 1-high
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
	}
}

void CGPIOWinGUIDlg::OnBnClickedClearOnePin()
{
	// TODO: Add your control notification handler code here
	if( m_hid == NULL )
	{
		return;
	}

	int pin = GetPin();
	if( pin >= MAX_GPIO_NUM )
	{
		AfxMessageBox( "Pin is wrong. (Pin<20)" );
		return;
	}

	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  3;   // Number
	report[ 3 ] =  0x53;// Command
	report[ 4 ] =  pin;  // pin
	report[ 5 ] =  0;   // 0-low, 1-high
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
	}
}

void CGPIOWinGUIDlg::OnBnClickedGetOnePin()
{
	// TODO: Add your control notification handler code here
	if( m_hid == NULL )
	{
		return;
	}

	int pin = GetPin();
	if( pin >= MAX_GPIO_NUM )
	{
		AfxMessageBox( "Pin is wrong. (Pin<20)" );
		return;
	}

	memset( report, 0, 65 );
	report[0] = ID_OUT_CONTROL;

	report[ 1 ] = '$';  // Start
	report[ 2 ] =  2;   // Number
	report[ 3 ] =  0x54;// Command
	report[ 4 ] =  pin;  // pin
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
	}

	DWORD bytesReturned = 0;
	BYTE err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 65, 1, &bytesReturned );
	if( err == HID_DEVICE_SUCCESS )
	{
		if( getreport[2] )
		{
			((CButton*)GetDlgItem(IDC_CHECK_GPIO20+pin))->SetCheck(1);
		}
		else
		{
			((CButton*)GetDlgItem(IDC_CHECK_GPIO20+pin))->SetCheck(0);
		}
	}
}
