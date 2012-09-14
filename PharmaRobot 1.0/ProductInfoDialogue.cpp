/*
Handles 'P' Product Information Request from CONSIS, queries Yarpa SQL database for product description
and send back a 'p' response to CONSIS.
*/

#include "stdafx.h"
#include "ConsisComm.h"
#include "PharmaRobot 1.0.h"
#include "PharmaRobot 1.0Dlg.h"


/* Low priority task checks if there's a message to be received from CONSIS and then sleeps for 200 msec */
DWORD WINAPI PDialogueListenerThread(CPharmaRobot10Dlg* pdialog)
{
	int MessageLength, MessagePSize = 114;
	char buffer[MAX_CONSIS_MESSAGE_SIZE];
	size_t retsize;
	pConsisRequestMessage * ppRequestMessage;
	PConsisPmsResponseMessage ResponseToConsis;
	wchar_t description[256];

	for (;;) //Run forever
	{
		//Perform Message checking only if CONSIS is connected
		if (pdialog->Consis.ConnectionStarted == TRUE)
		{

			//Try to catch the Mutex for CONSIS Access
			//Protect with Mutex the CONSIS resource
			CSingleLock singleLock(&(pdialog->m_Mutex));

			// Attempt to lock the shared resource
			if (singleLock.Lock(INFINITE))
			{
				//log locking success
			}

			//clear buffer
			memset(buffer, 0, MAX_CONSIS_MESSAGE_SIZE);

			MessageLength = sizeof(buffer);
			pdialog->Consis.ReceiveConsisMessage(buffer, &MessageLength, 0 ); //no timeout

			//Check if we received a 'p' request and handle it
			if (buffer[0] == 'p')
			{//igonre everything but a 'p' message
				ppRequestMessage = (pConsisRequestMessage *)buffer;
				//Extract Article ID
				wchar_t articleID[31];
				articleID[30] = '\0';
				mbstowcs_s(&retsize, articleID, sizeof(ppRequestMessage->ArticleId) + 1, ppRequestMessage->ArticleId, _TRUNCATE);

				//Clean leading zeroes
				CString cleanArticleID;
				cleanArticleID.SetString(articleID);
				cleanArticleID.TrimLeft(L'0');
				wsprintf(articleID,cleanArticleID.GetString());

				//clean the message to CONSIS
				memset ((void*)&ResponseToConsis, '0',sizeof(ResponseToConsis));

				//Get Description from Yarpa SQL
				if (pdialog->GetItemDescFromBarcode(articleID, description))
				{//Got a description
					char DescriptionInChar[40];
					wcstombs(DescriptionInChar, description, 39);
					memcpy(&(ResponseToConsis.ArticleName), DescriptionInChar, sizeof(ResponseToConsis.ArticleName));
				}
				else
				{
					sprintf(ResponseToConsis.ArticleName, "Name Missing");
				}
				ResponseToConsis.RecordType = 'P';
				memcpy(&(ResponseToConsis.ArticleId), ppRequestMessage->ArticleId, sizeof(ResponseToConsis.ArticleId));

				//Send a response to CONSIS
				pdialog->Consis.SendConsisMessage((char*)&ResponseToConsis, MessagePSize);
			}

			singleLock.Unlock();
		
		}//CONSIS connected if statement context

		Sleep(200);
	}
	// NOT REACHED

	return 0;
}
