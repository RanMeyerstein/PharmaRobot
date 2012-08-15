﻿
#include "stdafx.h"
#include "PharmaRobot 1.0.h"
#include "PharmaRobot 1.0Dlg.h"
#include "afxdialogex.h"
#include <afxsock.h>    // For CSocket 
#include <iostream>
#include "Iphlpapi.h"

_TCHAR AckBuffer[100];

typedef enum QUERYRESPONSE
{
	Q_ERROR = 0,
	Q_NOACK,
	Q_SENDACK
};

struct PRORBTPARAMS
{
	_TCHAR Header[1],Barcode[14], Qty[4], SessionId[17], LineNum[5], TotalLines[5], Directive[2], CounterUnit[4];
};

QUERYRESPONSE HandleQueryCommand(PRORBTPARAMS * pProRbtParams, CPharmaRobot10Dlg* pdialog)
{
	if (pdialog->Consis.ConnectionStarted == FALSE)
	{
		pdialog->Consis.ConnectToConsis("ShorT", &(pdialog->m_listBoxMain));
	}

	if (pdialog->Consis.ConnectionStarted == TRUE)
	{
		memset(pdialog->ConsisMessage, '0', 41);
		pdialog->ConsisMessage[41] = '\0';

		size_t convertedChars = 0;

		/*Counter Unit*/
		CString CounterUnitString = pProRbtParams->CounterUnit;
		size_t len = CounterUnitString.GetLength();
		int location = 4 - len;
		wchar_t Source[4];
		wsprintf(Source, CounterUnitString.GetString());
		wcstombs(&(pdialog->ConsisMessage[location]), Source, len);

		/*Barcode*/
		CString BarCodeString = pProRbtParams->Barcode;
		len = BarCodeString.GetLength();
		location = 41 - len;
		wchar_t barcode[14];
		wsprintf(barcode, BarCodeString.GetString());
		wcstombs(&(pdialog->ConsisMessage[location]), barcode, len);

		pdialog->ConsisMessage[0] = 'B';

		pdialog->Consis.SendStockQuery(pdialog->ConsisMessage);
		
		wsprintf(AckBuffer,L"  הארון מכיל כמות מסוימת של פריטים בעלי ברקוד %s \0", pProRbtParams->Barcode);
		return Q_SENDACK;
	}
	else
	{
		wsprintf(AckBuffer,L" שרת קונסיס לא פעיל\0");
		return Q_SENDACK;
	}
}

DWORD WINAPI SocketThread(CPharmaRobot10Dlg* pdialog)
{
	QUERYRESPONSE res;
	char echoBuffer[sizeof(PRORBTPARAMS)]; // Buffer for echo string

	PRORBTPARAMS * pProRoboParams = (PRORBTPARAMS *)echoBuffer;

	// Initialize the AfxSocket
	AfxSocketInit(NULL);

	int echoServPort = 50004;  // First arg: local port
	CSocket servSock;                  // Socket descriptor for server

	// Create the server socket
	if (!servSock.Create(echoServPort)) {
		//DieWithError("servSock.Create() failed");
	}

	// Mark the socket so it will listen for incoming connections
	if (!servSock.Listen(5)) {
		//DieWithError("servSock.Listen() failed");
	}


	for(;;) { // Run forever
		CSocket clntSock;                // Socket descriptor for client
		SOCKADDR_IN echoClntAddr;        // Client address
		int clntLen;                     // Length of client address data structure 

		// Get the size of the in-out parameter
		clntLen = sizeof(echoClntAddr);

		// Wait for a client to connect
		if (!servSock.Accept(clntSock)) {
			//DieWithError("servSock.Accept() failed");
		}

		// ClntSock is connected to a client!

		// Get the client's host name
		if (!clntSock.GetPeerName((SOCKADDR*)&echoClntAddr, &clntLen)) {
			//DieWithError("clntSock.GetPeerName() failed");
		}
		char clientaddress[10];

		_ultoa_s(echoClntAddr.sin_addr.S_un.S_addr,clientaddress,10);


		int recvMsgSize;              // Size of received message

		// Recieve message from client 
		recvMsgSize = clntSock.Receive(echoBuffer, sizeof(PRORBTPARAMS), 0);
		if (recvMsgSize < 0) {
			// DieWithError("clntSock.Receive() failed");
		}

		CString st;
		if (pProRoboParams->Header[0] == '`')
		{
			st = "Received from Client: "; st += clientaddress; pdialog->m_listBoxMain.AddString(st);
			st = "Counter Unit ID: "; st +=  pProRoboParams->CounterUnit; pdialog->m_listBoxMain.AddString(st);
			st = "Directive: "; st += pProRoboParams->Directive; pdialog->m_listBoxMain.AddString(st);
			st = "Bracode: "; st += pProRoboParams->Barcode; pdialog->m_listBoxMain.AddString(st);
			st = "Qty: "; st += pProRoboParams->Qty; pdialog->m_listBoxMain.AddString(st);
			st = "SessionId: "; st += pProRoboParams->SessionId; pdialog->m_listBoxMain.AddString(st);
			st = "LineNum: "; st += pProRoboParams->LineNum; pdialog->m_listBoxMain.AddString(st);
			st = "TotalLines: "; st += pProRoboParams->TotalLines; pdialog->m_listBoxMain.AddString(st);

			if (pProRoboParams->Directive[0] == L'1')
			{
				res = HandleQueryCommand(pProRoboParams, pdialog);
			}
			else if (pProRoboParams->Directive[0] == L'0')
			{

			}
			switch (res)
			{
			case Q_ERROR:
				// Echo message back to client
				wsprintf(AckBuffer,L"נכשלה השליחה לשרת קונסיס \0");
				clntSock.Send(AckBuffer, sizeof(AckBuffer), 0);
				st.SetString(L"Ack Sent"); pdialog->m_listBoxMain.AddString(st);
				break;

			case Q_NOACK:
				break;

			case Q_SENDACK:
				// Echo message back to client
				clntSock.Send(AckBuffer, sizeof(AckBuffer), 0);
				st.SetString(L"Ack Sent"); pdialog->m_listBoxMain.AddString(st);
				break ;
			}

		}
		else
		{
			st.SetString(L"Bad Packet Content"); pdialog->m_listBoxMain.AddString(st);
		}
		clntSock.Close(); // Close client socket

	}

  // NOT REACHED

  return 0;
}