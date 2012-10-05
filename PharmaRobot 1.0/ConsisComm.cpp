
#include "stdafx.h"
#include "ConsisComm.h"
#define CONSIS_PORT "2765"
#define DEFAULT_BUFLEN 512


typedef long (*CIGetLastSystemError) (void);
CIGetLastSystemError ptCIGetLastSystemError = {NULL};

typedef SOCKET (*CIGetSocket)( void );
CIGetSocket ptCIGetSocket = {NULL};

typedef int (*CISockPort)(SOCKET socknumber);
CISockPort ptCISockPort = {NULL};

typedef int (*CIOpen)( char * pszClientName );
CIOpen ptCIOpen = {NULL};

typedef int (*CIClose)(void);
CIClose ptCIClose = {NULL};

typedef int (*CISendMessg)( char *pszMessg, int iMessgLen, long lTimeout );
CISendMessg ptCISendMessg = {NULL};

typedef int (*CIRecvMessg)( char *pszMessg, int * piLen, char *pbMessgPending, long lTimeout);
CIRecvMessg ptCIRecvMessg = {NULL};

typedef int (*CIRecvMessgNB)( char *pszMessg, int * piLen, char *pbMessgPending, long lTimeout);
CIRecvMessgNB ptCIRecvMessgNB = {NULL};

typedef int (*CIDeleteResponse)(void);
CIDeleteResponse ptCIDeleteResponse;


ConsisComm::ConsisComm()
{
	hinstLibCD = NULL;
	hinstLibCD = NULL;
	ConnectionStarted = FALSE;
	DllsLoaded = FALSE;
    sprintf_s (clinetname,"ShorTAbachnink");
}

ConsisComm::~ConsisComm()
{
	BOOL fFreeResult;

	if (result != NULL)
	{
		WSACleanup();
	}

	if (ConnectionStarted == TRUE)
	{
		int ret = ptCIClose();
		if (ret != 0 )		{
			OutputDebugString(L"Closed successfuly with returned value\n");// = %d\n", ret);
		}
		else		{
			OutputDebugString(L"Failed to close Connection with Client\n");
		}
	}

	if (DllsLoaded == TRUE)
	{
		fFreeResult = FreeLibrary(hinstLibAce);
		fFreeResult = FreeLibrary(hinstLibCD);
	}
}

int ConsisComm::ConnectToConsis(char* clientName, CListBox * dlglistBox, CButton * remotebutton)
{
	int returnedValue = 0;
	WCHAR mbstring[256];
	char * message;
	char message_buf[2048];
	int  message_len = 0;
	int  rc;
	char pending;
	const size_t newsize = 100;
	size_t convertedChars = 0;

	if (ConnectionStarted == FALSE)
	{
		if (DllsLoaded == FALSE)
		{
			// Get a handle to the DLL module.
			hinstLibAce = LoadLibrary(TEXT("ace.dll")); 
			hinstLibCD =  LoadLibrary(TEXT("cdclient.dll"));

			m_dlglistBox = dlglistBox;
			m_dlglistBox->AddString(L"Loaded DLLs");

			DllsLoaded = TRUE;

			if ((hinstLibCD != NULL) || (hinstLibCD != NULL) )
			{
				ptCIGetLastSystemError = (CIGetLastSystemError)GetProcAddress(hinstLibCD, "CIGetLastSystemError"); 
				ptCIGetSocket = (CIGetSocket)GetProcAddress(hinstLibCD, "CIGetSocket");
				ptCISockPort = (CISockPort)GetProcAddress(hinstLibCD, "CISockPort");
				ptCIOpen =  (CIOpen)GetProcAddress(hinstLibCD, "CIOpen");
				ptCIClose = (CIClose)GetProcAddress(hinstLibCD, "CIClose");
				ptCISendMessg = (CISendMessg)GetProcAddress(hinstLibCD, "CISendMessg");
				ptCIRecvMessg = (CIRecvMessg)GetProcAddress(hinstLibCD, "CIRecvMessg");
				ptCIRecvMessgNB = (CIRecvMessgNB)GetProcAddress(hinstLibCD, "CIRecvMessgNB");
				ptCIDeleteResponse = (CIDeleteResponse)GetProcAddress(hinstLibCD, "CIDeleteResponse");

				// If the function address is valid, call the function.

				if (NULL != ptCIGetLastSystemError) 		{
					OutputDebugString(L"Got ptCIGetLastSystemError function pointer\n");// 0x%x \n",ptCIGetLastSystemError); 
				}
				if (NULL != ptCIGetSocket) 		{
					OutputDebugString(L"Got ptCIGetSocket function pointer\n");// 0x%x \n", ptCIGetSocket); 
				}
				if (NULL != ptCISockPort) 		{
					OutputDebugString(L"Got ptCISockPort function pointer\n");// 0x%x \n", ptCISockPort); 
				}
				if (NULL != ptCIOpen) 		{
					OutputDebugString(L"Got ptCIOpen function pointer\n");//0x%x \n",ptCIOpen); 
				}
				if (NULL != ptCIClose) 		{
					OutputDebugString(L"Got ptCIClose function pointer\n");// 0x%x \n",ptCIClose); 
				}
				if (NULL != ptCISendMessg) 		{
					OutputDebugString(L"Got ptCISendMessg function pointer\n");// 0x%x \n",ptCISendMessg); 
				}
				if (NULL != ptCIRecvMessg) 		{
					OutputDebugString(L"Got ptCIRecvMessg function pointer \n");//0x%x \n",ptCIRecvMessg); 
				}
				if (NULL != ptCIRecvMessgNB) 		{
					OutputDebugString(L"Got ptCIRecvMessgNB function pointer\n");// 0x%x \n",ptCIRecvMessgNB); 
				}
				if (NULL != ptCIDeleteResponse) 		{
					OutputDebugString(L"Got ptCIClose function pointer \n");//0x%x \n",ptCIDeleteResponse); 
				}
			}
		}

		int ret = ptCIOpen(clinetname);
		if (ret != 0 )		{
			dlglistBox->AddString(L"Opened CONSIS successfuly");
		}
		else		{
			dlglistBox->AddString(L"Failed to open Connection with Client\n");
		}

		if (remotebutton->GetCheck())
		{
			ret = ptCISockPort(8006);
			if (ret != 0 )		{
				dlglistBox->AddString(L"Set Port to 8006\n");// = %d\n", ret);
			}
			else		{
				dlglistBox->AddString(L"Failed to Set Port in CONSIS Server\n");
			}
		}
		else
		{
			ret = ptCISockPort(2766);
			if (ret != 0 )		{
				dlglistBox->AddString(L"Set Port to 2766\n");// = %d\n", ret);
			}
			else		{
				dlglistBox->AddString(L"Failed to Set Port in CONSIS Server\n");
			}
		}

		ret = ptCIOpen(clinetname);
		if (ret != 0 )		{
			OutputDebugString(L"Opened CONSIS successfuly");
		}
		else		{
			OutputDebugString(L"Failed to open Connection with Client\n");
		}

		message = "R" "001" "C113" "004ASPIR";
		rc = ptCISendMessg( message, strlen(message),1000);
		if(rc!=0)
		{
			dlglistBox->AddString(L"sending R-message failed");//,rc);
		}
		else
		{
			/*
			* get the r-result message
			*/
			message_len = sizeof(message_buf); //ensure that the message_len field always contains the length of
			//the buffer before calling
			rc = ptCIRecvMessg( message_buf, &message_len, &pending,1000); //blocks until a message is pending
			if(rc!=0)
			{
				OutputDebugString(L"receiving R-response failed\n");//,rc);
				m_dlglistBox->AddString(L"receiving R-response failed");
			}
			else
			{
				message_buf[message_len]= '\0';
				OutputDebugString(L"got result:%s\n");
				size_t origsize = strlen(message_buf) + 1;
				wchar_t wcstring[newsize];
				mbstowcs_s(&convertedChars, wcstring, origsize, message_buf, _TRUNCATE);
				wsprintf(mbstring,L"got result %s",wcstring);
				m_dlglistBox->AddString(mbstring);
				ConnectionStarted = TRUE;
				returnedValue = 1;
			}
		}
	}
	return returnedValue;
}



BOOL ConsisComm::SendConsisMessage(char* MessageContent, size_t BufferSize)
{
	if 	(ptCISendMessg( MessageContent, BufferSize,3000) == 0)
		return TRUE;

	return FALSE;
}

BOOL ConsisComm::ReceiveConsisMessage(char* ReceiveBuffer, int * messageLength, int timeout)
{
	char pending = 1;
	while (pending)
	{
		if (ptCIRecvMessgNB( ReceiveBuffer, messageLength, &pending, timeout) != 0)
		return FALSE;
	}
	return TRUE;
}

int ConsisComm::SendStockQuery(char* MessageContent)
{
	int rc = ptCISendMessg( MessageContent, strlen(MessageContent),3000);
	if(rc!=0)
		m_dlglistBox->AddString(L"sending stock query failed");

	char message_buf[MAX_CONSIS_MESSAGE_SIZE];
	char pending;
	int message_len = sizeof(message_buf);
	rc = ptCIRecvMessgNB( message_buf, &message_len, &pending,1000); //block at most 1 second
	if(rc!=0)
	{
		m_dlglistBox->AddString(L"no message to receive");

	}
	else
	{
		message_buf[message_len]= '\0';
		size_t convertedChars = 0;
		wchar_t wcstring[MAX_CONSIS_MESSAGE_SIZE];
		mbstowcs_s(&convertedChars, wcstring, message_len + 1, message_buf, _TRUNCATE);
		m_dlglistBox->AddString(wcstring);
	}

	return 0;
}

int ConsisComm::SendDispnseCommand(char* MessageContent)
{
	int rc = ptCISendMessg( MessageContent, strlen(MessageContent),3000);
	if(rc!=0)
		m_dlglistBox->AddString(L"sending dispense command failed");

	char message_buf[MAX_CONSIS_MESSAGE_SIZE];
	memset(message_buf, 0, MAX_CONSIS_MESSAGE_SIZE);
	char pending;
	int message_len = sizeof(message_buf);

	do{
		rc = ptCIRecvMessgNB( message_buf, &message_len, &pending,1000); //block at most 1 second
		if(rc!=0)
		{
			m_dlglistBox->AddString(L"no message to receive");
			break;

		}
		else
		{
			message_buf[message_len]= '\0';
			size_t convertedChars = 0;
			wchar_t wcstring[MAX_CONSIS_MESSAGE_SIZE];
			mbstowcs_s(&convertedChars, wcstring, message_len + 1, message_buf, _TRUNCATE);
			m_dlglistBox->AddString(wcstring);
		}
	}while (message_buf[16] != '5');

	return 1;
}
