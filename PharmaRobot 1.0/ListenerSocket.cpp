
#include "stdafx.h"
#include "PharmaRobot 1.0.h"
#include "PharmaRobot 1.0Dlg.h"
#include "afxdialogex.h"
#include <afxsock.h>    // For CSocket 
#include <iostream>
#include "Iphlpapi.h"
#include "ProRBT.h"
#include "ConsisComm.h"

extern 	PRORBTPARAMSACK ackemessage;

ProRbtDb g_ProRbtDb;


QUERYRESPONSE HandleDispenseCommand(PRORBTPARAMS * pProRbtParams, CPharmaRobot10Dlg* pdialog)
{

	CString st;
	size_t retsize, len;
	int location, origsize, MessageLength;
	char nstring[100], buffer[MAX_CONSIS_MESSAGE_SIZE];
	aConsisReplyHeader *paMesHeader;
	aConsisReplyDispensedOcc* aocc;

	//Protect with Mutex the CONSIS resource
	CSingleLock singleLock(&(pdialog->m_Mutex));

	// Attempt to lock the shared resource
	if (singleLock.Lock(INFINITE)) {

		if (pdialog->Consis.ConnectionStarted == FALSE)
		{
			if (pdialog->Consis.ConnectToConsis("ShorT", &(pdialog->m_listBoxMain), &(pdialog->m_CheckBoxRemoteSvr)))
				pdialog->EnableCondsisTab();
		}

		if (pdialog->Consis.ConnectionStarted == TRUE)
		{//Build 'A' Message with Barcode from RBT parameters

			memset(pdialog->ConsisMessage, '0', 61);
			pdialog->ConsisMessage[61] = '\0';

			/*Counter Unit taken from ProRBT parameters*/
			CString StringFromProRbt = pProRbtParams->CounterUnit;
			len = StringFromProRbt.GetLength();
			location = 12 - len;
			wchar_t Source[4];
			wsprintf(Source, StringFromProRbt.GetString());
			wcstombs(&(pdialog->ConsisMessage[location]), Source, len);

			/*Barcode taken from ProRBT parameters*/
			StringFromProRbt = pProRbtParams->Barcode;
			len = StringFromProRbt.GetLength();
			location = 60 - len;
			wchar_t barcodeSent[14];
			wsprintf(barcodeSent, StringFromProRbt.GetString());
			wcstombs(&(pdialog->ConsisMessage[location]), barcodeSent, len);

			/*Order number Taken from GUI*/
			pdialog->m_OrderNum++; if (pdialog->m_OrderNum > 99999999) pdialog->m_OrderNum = 0;//for every request increase number
			WCHAR wideStr[10];
			wsprintf(wideStr,L"%d\0",pdialog->m_OrderNum);
			pdialog->m_EditOrderNum.SetWindowTextW(wideStr);

			size_t convertedChars;
			origsize = pdialog->m_EditOrderNum.GetWindowTextLengthW() + 1;
			pdialog->m_EditOrderNum.GetWindowTextW(wideStr,origsize);
			wcstombs_s(&convertedChars, nstring, origsize, wideStr , _TRUNCATE);
			location = 9 - (origsize - 1);
			memcpy((void*)&(pdialog->ConsisMessage[location]), (void*) nstring, (origsize - 1));

			/*Quantity Taken from ProRBT parameters*/
			StringFromProRbt = pProRbtParams->Qty;
			len = StringFromProRbt.GetLength();
			location = 30 - len;
			wchar_t ReqQuantity[5];
			wsprintf(ReqQuantity, StringFromProRbt.GetString());
			wcstombs(&(pdialog->ConsisMessage[location]), ReqQuantity, len);

			/*Priority Taken from GUI*/
			origsize = pdialog->m_EditPriority.GetWindowTextLengthW() + 1;
			pdialog->m_EditPriority.GetWindowTextW(wideStr,origsize);
			wcstombs_s(&convertedChars, nstring, origsize, wideStr , _TRUNCATE);
			location = 16 - (origsize - 1);
			memcpy((void*)&(pdialog->ConsisMessage[location]), (void*) nstring, (origsize - 1));

			/*Dispenser Taken from GUI*/
			origsize = pdialog->m_EditDispenser.GetWindowTextLengthW() + 1;
			pdialog->m_EditDispenser.GetWindowTextW(wideStr,origsize);
			wcstombs_s(&convertedChars, nstring, origsize, wideStr , _TRUNCATE);
			location = 15 - (origsize - 1);
			memcpy((void*)&(pdialog->ConsisMessage[location]), (void*) nstring, (origsize - 1));

			pdialog->ConsisMessage[0] = 'A';

			/* Send A message to CONSIS */
			pdialog->Consis.SendConsisMessage(pdialog->ConsisMessage, 61);

			//Session state depends on the order state
			//Last message Should be '4' - Ready, but Server returns '5', so that's what we'll wait for
			char orderState[3];
			memset(&(orderState[2]),'\0',1);
			memset(orderState,'0',2);

			do
			{
				MessageLength = sizeof(buffer);
				pdialog->Consis.ReceiveConsisMessage(buffer, &MessageLength, 1000);

				buffer[MessageLength] = '\0';
				paMesHeader = (aConsisReplyHeader *)buffer;

				memcpy(orderState, paMesHeader->OrderState, sizeof(paMesHeader->OrderState));
				orderState[2] = '\0';

			}while ( strcmp(orderState,"05") != 0 );//Waiting for quantity change by CONSIS to PMS

			//Extract number of locations
			char numart[3];
			memcpy(numart, paMesHeader->NumberOfArticles, sizeof(paMesHeader->NumberOfArticles));
			numart[2] = '\0';
			int numArticles =  atoi(numart);

			//Extract Article ID and Dispensed quantity from Occupancy entry in 'a' message occupany entry
			char* address = (char*)paMesHeader + sizeof(aConsisReplyHeader) + (numArticles * (sizeof(aConsisReplyDispensedOcc) - 1));
			aocc = (aConsisReplyDispensedOcc*)address;

			wchar_t articleID[31];
			articleID[30] = '\0';
			mbstowcs_s(&retsize, articleID, sizeof(aocc->ArticleId), aocc->ArticleId, _TRUNCATE);

			//Clean leading zeroes
			CString cleanArticleID;
			cleanArticleID.SetString(articleID);
			cleanArticleID.TrimLeft(L'0');
			wsprintf(articleID,cleanArticleID.GetString());

			wchar_t description[256];

			//Get Description from Yarpa SQL. If failed publish back to Client the Barcode
			if (!pdialog->GetItemDescFromBarcode(articleID, description))
				wsprintf(description,  articleID);

			//Dispensed quantity handling
			char dispenssedQuantity[6];
			memcpy(dispenssedQuantity, aocc->DispensedQuantity, sizeof(aocc->DispensedQuantity));
			dispenssedQuantity[5] = '\0';
			int DisQuan = atoi(dispenssedQuantity);
			int ReqQuan = _wtoi(ReqQuantity);
			if(DisQuan < ReqQuan)
			{
				wsprintf(ackemessage.Message,L"מפריט מסוג\n%s\n\nחסרים\n%d\nמבקשת הניפוק",description , ReqQuan - DisQuan);
			}
			else
			{
				wsprintf(ackemessage.Message,L"הונפקו מפריט מסוג\n%s\n %d פריטים",description , DisQuan);
			}
			singleLock.Unlock();
			return Q_SENDACK;
		}

		singleLock.Unlock();
	}
	wsprintf(ackemessage.Message,L" שרת קונסיס אינו זמין");
	return Q_SENDACK;
}


QUERYRESPONSE HandleQueryCommand(PRORBTPARAMS * pProRbtParams, CPharmaRobot10Dlg* pdialog)
{
	size_t retsize;
	BConsisStockRequest * pBresponse = (BConsisStockRequest *)pdialog->ConsisMessage;
	//Protect with Mutex the CONSIS resource
	CSingleLock singleLock(&(pdialog->m_Mutex));

	// Attempt to lock the shared resource
	if (singleLock.Lock(INFINITE))
	{
		//Log Lock success
	}

	if (pdialog->Consis.ConnectionStarted == FALSE)
	{
		if (pdialog->Consis.ConnectToConsis("ShorT", &(pdialog->m_listBoxMain), &(pdialog->m_CheckBoxRemoteSvr)))
			pdialog->EnableCondsisTab();
	}


	if (pdialog->Consis.ConnectionStarted == TRUE) 
	{//Build B Message with Barcode from RBT parameters
		memset(pdialog->ConsisMessage, '0', 41);
		pdialog->ConsisMessage[41] = '\0';

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

		memset(pBresponse->ArticleId,' ',30); //Clear leading zeros

		wcstombs(&(pdialog->ConsisMessage[location]), barcode, len);

		pdialog->ConsisMessage[0] = 'B';

		char buffer[MAX_CONSIS_MESSAGE_SIZE]; buffer[0] = 0;
		int MessageLength;
		while (buffer[0] != 'b')//BUG override. Sometimes the function returns an 'a' response
		{
			/* Send B message to CONSIS */
			pdialog->Consis.SendConsisMessage(pdialog->ConsisMessage, 42);
			MessageLength = sizeof(buffer);
			pdialog->Consis.ReceiveConsisMessage(buffer, &MessageLength, 1000);
		}
		buffer[MessageLength] = '\0';

		bConsisReplyHeader *pHeader = (bConsisReplyHeader *)buffer;

		//Extract number of locations
		char numloc[3];
		memcpy(numloc, pHeader->NumStockLocations, sizeof(pHeader->NumStockLocations));
		numloc[2] = '\0';
		int numLocation =  atoi(numloc);

		//Extract Total Quantity of Item
		char TotalQua[6];
		TotalQua[5] = '\0';
		memcpy(TotalQua, pHeader->TotalQuantity, sizeof(pHeader->TotalQuantity));
		int totalQuantity =  atoi(TotalQua);

		//Find Article ID which is in 'b' Footer after article locations
		char* address = (char*)pHeader + sizeof(bConsisReplyHeader) + (numLocation * (sizeof(bConsisReplyStockLocations)));
		bConsisReplyFooter* bfooter = (bConsisReplyFooter*)address;

		//Extract Article ID
		wchar_t articleID[31];
		articleID[30] = '\0';
		mbstowcs_s(&retsize, articleID, sizeof(bfooter->ArticleId) + 1, bfooter->ArticleId, _TRUNCATE);

		//Clean leading zeroes
		CString cleanArticleID;
		cleanArticleID.SetString(articleID);
		cleanArticleID.TrimLeft(L' ');
		wsprintf(articleID,cleanArticleID.GetString());

		wchar_t description[256];
		//Get Description from Yarpa SQL
		if (pdialog->GetItemDescFromBarcode(articleID, description))
		{
			if (totalQuantity)
			{
				wsprintf(ackemessage.Message,L"מצב מלאי\nמספר מזהה: %s\nמספר במלאי [%d]", description, totalQuantity);
			}
			else
			{
				wsprintf(ackemessage.Message,L"אין פריטים במלאי בעלי מספר מזהה \n%s", description);
			}
		}
		else
		{
			//Fill Ack message content
			if (totalQuantity)
			{
				wsprintf(ackemessage.Message,L"מפריט מסוג\n%s\nקיימים %d\nבמלאי", cleanArticleID.GetString(), totalQuantity);
			}
			else
			{
				wsprintf(ackemessage.Message,L"פריט מסוג\n%s\nאינו קיים במלאי", cleanArticleID.GetString());
			}
		}

		singleLock.Unlock();
		return Q_SENDACK;
	}
	wsprintf(ackemessage.Message,L" שרת קונסיס לא זמין\0");
	singleLock.Unlock();
	return Q_SENDACK;
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

		//_ultoa_s(echoClntAddr.sin_addr.S_un.S_addr,clientaddress,10);


		int recvMsgSize;              // Size of received message

		// Recieve message from client 
		recvMsgSize = clntSock.Receive(echoBuffer, sizeof(PRORBTPARAMS), 0);
		if (recvMsgSize < 0) {
			// DieWithError("clntSock.Receive() failed");
		}

		CString st;
		if (pProRoboParams->Header[0] == '`')
		{
			pdialog->m_listBoxMain.ResetContent();
			//st = "Received from Client: "; st += clientaddress; pdialog->m_listBoxMain.AddString(st);
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
			else if (pProRoboParams->Directive[0] == L'2')
			{
				res = g_ProRbtDb.HandleProRbtLine(pProRoboParams, pdialog);
				//res = HandleDispenseCommand(pProRoboParams, pdialog);

			}
			switch (res)
			{
			case Q_ERROR:
				// Echo message back to client
				ackemessage.Header[0] = L'`';
				ackemessage.Type[0] = L'1';
				clntSock.Send((wchar_t*)ackemessage.Header, sizeof(ackemessage), 0);
				st.SetString(L"Ack Sent"); pdialog->m_listBoxMain.AddString(st);
				break;

			case Q_NOACK:
				ackemessage.Header[0] = L'`';
				ackemessage.Type[0] = L'0';
				clntSock.Send((wchar_t*)ackemessage.Header, sizeof(ackemessage), 0);
				break;

			case Q_SENDACK:
				// Echo message back to client
				ackemessage.Header[0] = L'`';
				ackemessage.Type[0] = L'1';
				clntSock.Send((wchar_t*)ackemessage.Header, sizeof(ackemessage), 0);
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