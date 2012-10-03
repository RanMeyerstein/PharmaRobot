
#include "stdafx.h"
#include "ProRBT.h"


PRORBTPARAMSACK ackemessage;

struct REQUESTINTERMEIDATEDB {
	wchar_t ArctileIdRequested[31];
	int NumReq;
	int NumDis;
};


void ProRbtDb::InitCounterSession (PRORBTCOUNTERSESSION* pCounterSession)
{
	pCounterSession->CurrentSessionId  = -1;
	pCounterSession->PreviousSessionId = -1;
	pCounterSession->ExpectedNumLines  = -1;
	pCounterSession->ReceivedNumLines  = -1;

	memset (pCounterSession->RbtParamLinesArr, 0, sizeof(PRORBTPARAMS) * MAXIMAL_NUM_LINES_SUPPORTED);
}

void ProRbtDb::InitProRbtDb ()
{
	memset (m_DB.CounterIdToSessionEntryArr, -1, sizeof(int) * MAXIMAL_NUM_LINES_SUPPORTED);

	for (int i = 0 ; i < MAXIMAL_NUM_COUNTERS_SUPPORTED ; i++)
	{
		InitCounterSession(&(m_DB.CounterSessionArr[i]));
	}
}

BOOL ProRbtDb::FreeProRbtDbEntry (int CounterId)
{
	//Find entry of counter ID in array of Counter ID to Database entry
	for (int i = 0 ; i < MAXIMAL_NUM_COUNTERS_SUPPORTED ; i++)
	{
		if (CounterId == m_DB.CounterIdToSessionEntryArr[i])
		{//Found entry, return TRUE;
			m_DB.CounterIdToSessionEntryArr[i] = -1;
			InitCounterSession(&(m_DB.CounterSessionArr[i]));
			return TRUE;
		}
	}
	//entry not found and not freed, return FALSE
	return FALSE;
}

//Return DataBase entry of Counter ID (previously set)
int ProRbtDb::GetProRbtDbEntryFromCounterId (int counterid)
{
	//Find entry of counter ID in array of Counter ID to Database entry
	for (int i = 0 ; i < MAXIMAL_NUM_COUNTERS_SUPPORTED ; i++)
	{
		if (counterid == m_DB.CounterIdToSessionEntryArr[i])
		{//Found entry, return entry number
			return i;
		}
	}
	// entry not found
	return ENTRY_NOT_FOUND;
}

//Return Database entry number
int ProRbtDb::AcquireProRbtDbEntry(int counterid)
{
	//Find entry of counter ID in array of Counter ID to Database entry
	for (int i = 0 ; i < MAXIMAL_NUM_COUNTERS_SUPPORTED ; i++)
	{
		if (-1 == m_DB.CounterIdToSessionEntryArr[i])
		{//Found entry, return entry number
			m_DB.CounterIdToSessionEntryArr[i] = counterid;
			return i;
		}
	}
	//Database is full. MAJOR ERROR
	return ENTRY_NOT_FOUND;
}


BOOL ProRbtDb::FillProRbtDbLine (PRORBTPARAMS* pProRbtLine, int DBEntry)
{
	PRORBTCOUNTERSESSION * pCounterSession = &(m_DB.CounterSessionArr[DBEntry]);

	int expNumLine  = _wtoi(pProRbtLine->TotalLines);
	int CurrentLine = _wtoi(pProRbtLine->LineNum);
	int SesId       = _wtoi(pProRbtLine->SessionId);

	if (pCounterSession->CurrentSessionId == -1)//First Line in session
	{
		pCounterSession->CurrentSessionId = SesId;
		//Check if session ID is incremented from the previous one
	}
	else
	{
		if (pCounterSession->CurrentSessionId != (SesId-1)) 
		{//Unexpected Session ID.
			return FALSE;
		}
	}

	//Session ID OK, Continue to add line to database
	if (pCounterSession->ExpectedNumLines == -1)//Check if initial value
	{//First Line
		pCounterSession->ExpectedNumLines = expNumLine;
		pCounterSession->ReceivedNumLines = 1;
	}
	else
	{
		pCounterSession->ReceivedNumLines += 1;
	}
	//Copy Line into database according to line number from Yarpa
	memcpy(&(pCounterSession->RbtParamLinesArr[CurrentLine]), pProRbtLine, sizeof(PRORBTPARAMS));

	return TRUE;
}

QUERYRESPONSE ProRbtDb::HandleCounterIdEntry(PRORBTCOUNTERSESSION * pCounterSession, CPharmaRobot10Dlg* pdialog)
{
	QUERYRESPONSE returnvalue = Q_NOACK;
	size_t retsize, len;
	int MessageLength, MessageASize, NumLinesInConsisMessage;
	char nstring[100], buffer[MAX_CONSIS_MESSAGE_SIZE];
	aConsisReplyHeader *paMesHeader;
	aConsisReplyDispensedOcc* aocc;
	REQUESTINTERMEIDATEDB InterMDb[MAXIMAL_NUM_LINES_SUPPORTED];
	CString StringFromProRbt;


	if (pCounterSession->ExpectedNumLines == pCounterSession->ReceivedNumLines)
	{
		//Protect with Mutex the CONSIS resource
		CSingleLock singleLock(&(pdialog->m_Mutex));

		// Attempt to lock the shared resource
		if (singleLock.Lock(INFINITE))
		{
			//log locking success
		}
		//Clean intermideate database
		memset (InterMDb, 0, sizeof(InterMDb));
		int l = 0;
		//build intermediate database that agregates identical article IDs (barcodes)
		for (int j = 0 ; j < pCounterSession->ExpectedNumLines ; j++)
		{
			bool entryfound = false;
			while (InterMDb[l].NumReq != 0) //search for an empty entry indicating the last entry in database
			{
				if (wcscmp(InterMDb[l].ArctileIdRequested, pCounterSession->RbtParamLinesArr[j + 1].Barcode) == 0)
				{
					InterMDb[l].NumReq += _wtoi(pCounterSession->RbtParamLinesArr[j+1].Qty); //aggregate number of requested articles
					entryfound = true;
				}
				l++;//Check next entry in intermediate database
			}
			if(entryfound == false)
			{//barcode not found in any of the entries in the database. 
				//Create new entry - copy Barcode number and requested quantity to inetrmediate database.
				StringFromProRbt = pCounterSession->RbtParamLinesArr[j + 1].Barcode;
				len = StringFromProRbt.GetLength();
				memcpy(&(InterMDb[l].ArctileIdRequested),pCounterSession->RbtParamLinesArr[j+1].Barcode,len*sizeof(wchar_t));
				InterMDb[l].ArctileIdRequested[len] = L'\0';//insure string ends with a null
				InterMDb[l].NumReq = _wtoi(&(pCounterSession->RbtParamLinesArr[j+1].Qty[0])); //insert numer of requested articles
			}
			l = 0;
		}

		//Build CONSIS command, send it and wait for CONSIS to complete it
		int TensNumber = (pCounterSession->ExpectedNumLines / 10) + 1; //maximal number of items per 'A' command is ten.
		int CountDownLines = pCounterSession->ExpectedNumLines; 

		//build and receive dispense commands for 10 lines each time
		do{
			//Build an 'A' command every ten ProRBT lines
			if (pdialog->Consis.ConnectionStarted == FALSE)
			{
				if (pdialog->Consis.ConnectToConsis("ShorT", &(pdialog->m_listBoxMain), &(pdialog->m_CheckBoxRemoteSvr)))
					pdialog->EnableCondsisTab();
			}

			if (pdialog->Consis.ConnectionStarted == TRUE)
			{   //Build 'A' Message with Barcode from RBT parameters

				//Message size is a function of the number of lines in this ten line session
				NumLinesInConsisMessage = (CountDownLines <= 10 ? CountDownLines : 10);
				CountDownLines -= 10;
				MessageASize = 18 + (43*(NumLinesInConsisMessage));

				memset(pdialog->ConsisMessage, '0', MessageASize);
				pdialog->ConsisMessage[MessageASize] = '\0';

				/*Counter Unit taken from ProRBT parameters*/
				StringFromProRbt = pCounterSession->RbtParamLinesArr[1].CounterUnit;//Take counter ID from first line
				int len = StringFromProRbt.GetLength();
				int location = 12 - len;
				wchar_t Source[4];
				wsprintf(Source, StringFromProRbt.GetString());
				wcstombs(&(pdialog->ConsisMessage[location]), Source, len);

				/*Dispenser Taken from ProRBT parameters*/
				StringFromProRbt = pCounterSession->RbtParamLinesArr[1].Dispenser;//Take Dispenser ID from first line
				len = StringFromProRbt.GetLength();
				location = 15 - len;
				wsprintf(Source, StringFromProRbt.GetString());
				wcstombs(&(pdialog->ConsisMessage[location]), Source, len);

				//Fill Barcode and Quantity per line 
				for (int i = 0; i < NumLinesInConsisMessage ; i++){
					/*Barcode taken from ProRBT parameters*/
					StringFromProRbt = pCounterSession->RbtParamLinesArr[i + 1].Barcode;
					len = StringFromProRbt.GetLength();
					location = (18 + (43 * (i + 1))) - len;
					wchar_t barcodeSent[14];
					wsprintf(barcodeSent, StringFromProRbt.GetString());
					//Fill with blanks instaed of leading zeros
					memset(&(pdialog->ConsisMessage[(18 + (43 * (i + 1))) - 30]),' ', 30);
					//Set the barcode
					wcstombs(&(pdialog->ConsisMessage[location]), barcodeSent, len);

					/*Quantity Taken from ProRBT parameters*/
					StringFromProRbt = pCounterSession->RbtParamLinesArr[i + 1].Qty;
					len = StringFromProRbt.GetLength();
					location = (18 + (12 * (i + 1))) - len;
					wchar_t ReqQuantity[5];
					wsprintf(ReqQuantity, StringFromProRbt.GetString());
					wcstombs(&(pdialog->ConsisMessage[location]), ReqQuantity, len);
				}


				/*Order number Taken from GUI*/
				pdialog->m_OrderNum++; if (pdialog->m_OrderNum > 99999999) pdialog->m_OrderNum = 0;//for every request increase number
				WCHAR wideStr[10];
				wsprintf(wideStr,L"%d\0",pdialog->m_OrderNum);
				pdialog->m_EditOrderNum.SetWindowTextW(wideStr);

				size_t convertedChars;
				size_t origsize = pdialog->m_EditOrderNum.GetWindowTextLengthW() + 1;
				pdialog->m_EditOrderNum.GetWindowTextW(wideStr,origsize);
				wcstombs_s(&convertedChars, nstring, origsize, wideStr , _TRUNCATE);
				location = 9 - (origsize - 1);
				memcpy((void*)&(pdialog->ConsisMessage[location]), (void*) nstring, (origsize - 1));


				/*Priority Taken from GUI*/
				origsize = pdialog->m_EditPriority.GetWindowTextLengthW() + 1;
				pdialog->m_EditPriority.GetWindowTextW(wideStr,origsize);
				wcstombs_s(&convertedChars, nstring, origsize, wideStr , _TRUNCATE);
				location = 16 - (origsize - 1);
				memcpy((void*)&(pdialog->ConsisMessage[location]), (void*) nstring, (origsize - 1));

				pdialog->ConsisMessage[0] = 'A';

				/* Send A message to CONSIS */
				pdialog->Consis.SendConsisMessage(pdialog->ConsisMessage, MessageASize);

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

					//TEMP RANM DEBUG
					size_t origsize = strlen(buffer) + 1;
					wchar_t wcstring[100];
					mbstowcs_s(&convertedChars, wcstring, origsize, buffer, _TRUNCATE);
					CString st = wcstring; pdialog->m_listBoxMain.AddString(st);
					//TEMP RANM DEBUG

				}while ( strcmp(orderState,"04") != 0 );//Supposed to wait for quantity change by CONSIS to PMS, but now wait for ready

				//Extract number of locations
				char numart[3];
				memcpy(numart, paMesHeader->NumberOfArticles, sizeof(paMesHeader->NumberOfArticles));
				numart[2] = '\0';
				int numArticles =  atoi(numart);

				//TEMP RANM DEBUG
				wchar_t Stringnumdif1f[3]; _itow(numArticles,Stringnumdif1f,10);
				Stringnumdif1f[2] = '\0';
				CString st = L"numArticles = "; st += Stringnumdif1f; pdialog->m_listBoxMain.AddString(st);
				//TEMP RANM DEBUG


				for (int i = 0 ; i < numArticles ; i++ )
				{
					//Extract Article ID and Dispensed quantity from Occupancy entry in 'a' message occupany entry
					char* address = (char*)paMesHeader + sizeof(aConsisReplyHeader) + (i * 42);
					aocc = (aConsisReplyDispensedOcc*)address;

					//Get article ID from 'a' response
					wchar_t articleID[31];
					articleID[30] = '\0';
					mbstowcs_s(&retsize, articleID, sizeof(aocc->ArticleId), aocc->ArticleId, _TRUNCATE);

					//Clean leading zeroes
					CString cleanArticleID;
					cleanArticleID.SetString(articleID);
					cleanArticleID.TrimLeft(L'0');
					wsprintf(articleID,cleanArticleID.GetString());
					articleID[cleanArticleID.GetLength()] = L'\0';

					//find article ID in intermediate database
					int k = 0;
					while (InterMDb[k].NumReq != 0)
					{
						if (wcscmp(articleID, InterMDb[k].ArctileIdRequested) == 0)
						{//found the entry accoridng to barcode in intermedeate database
							//Dispensed quantity handling. Fill intermediate database for handling at the end.
							char dispenssedQuantity[6];
							memcpy(dispenssedQuantity, aocc->DispensedQuantity, sizeof(aocc->DispensedQuantity));
							dispenssedQuantity[5] = '\0';
							InterMDb[k].NumDis += atoi(dispenssedQuantity);
						}
						k++;//serach for article ID in the intermediate database further
					}
				}
			}
			else
			{
				//Error with Consis, Init entire Database
				InitProRbtDb();
				singleLock.Unlock();
				memset(ackemessage.Message, 0, ACK_MESSAGE_SIZE * sizeof(_TCHAR));
				wsprintf(ackemessage.Message,L"תקלה בתקשורת לשרת קונסיס");
				return Q_ERROR;
			}

			if (TensNumber != 0 ) TensNumber--;//handled first ten, go on to next ten
		}while (TensNumber);

		//CONSIS Completed, Build Ack from intermediate database. Check if all entries in the aggregated database were dispensed.
		int m = 0;
		bool missingitems = false;
		CString AckedString = L"מבקשת הניפוק חסרים הפריטים הבאים"; AckedString += L":";AckedString += L"\n";
		while (InterMDb[m].NumReq != 0)
		{
			if (InterMDb[m].NumReq != InterMDb[m].NumDis)
			{
				missingitems = true;
				wchar_t description[256];
				//Get Description from Yarpa SQL. If failed publish back to Client the Barcode
				if (!pdialog->GetItemDescFromBarcode(InterMDb[m].ArctileIdRequested, description))
					wsprintf(description,  InterMDb[m].ArctileIdRequested);
				AckedString += L"סוג פריט"; AckedString += L":"; AckedString += L" ";
				AckedString += description; AckedString += L"\n";
				AckedString += L"חסרים"; AckedString += L":"; AckedString += L" ";
				wchar_t Stringnumdiff[3]; _itow(InterMDb[m].NumReq - InterMDb[m].NumDis,Stringnumdiff,10);
				AckedString += Stringnumdiff;
				AckedString += L"\n\n";
			}
			m++;//Go on to next item in intermediate databse.
		}

		if (missingitems)
		{
			memset(ackemessage.Message, 0, ACK_MESSAGE_SIZE * sizeof(_TCHAR));
			memcpy(ackemessage.Message, AckedString.GetString(), AckedString.GetLength() * sizeof(wchar_t));
			returnvalue = Q_SENDACK;
		}
		else
		{
			returnvalue = Q_NOACK;
		}

		//Prepare Database Line Entry for next Session
		pCounterSession->PreviousSessionId = pCounterSession->CurrentSessionId;
		pCounterSession->ExpectedNumLines = -1;
		pCounterSession->ReceivedNumLines = -1;
		pCounterSession->CurrentSessionId = -1;

		singleLock.Unlock();
	}
	return returnvalue;
}


QUERYRESPONSE ProRbtDb::HandleProRbtLine(PRORBTPARAMS* pProRbtLine, CPharmaRobot10Dlg* pdialog)
{
	//First Check if quantity is zero. If so do not handle entry and return error message to ProRBT.exe
	int quantitychecked = _wtoi(pProRbtLine->Qty);
	if (quantitychecked == 0) {
		memset(ackemessage.Message, 0, ACK_MESSAGE_SIZE * sizeof(_TCHAR));
		wsprintf(ackemessage.Message,L"כמות לניפוק אפס אינה חוקית");
		return Q_ERROR;//CONSIS does not habdle zero quantity well...
	}

	int counterid = _wtoi(pProRbtLine->CounterUnit);

	int dBEntry = GetProRbtDbEntryFromCounterId(counterid);

	if (dBEntry == ENTRY_NOT_FOUND)
	{//Entry not found, produce new one
		dBEntry = AcquireProRbtDbEntry(counterid);
		if (dBEntry == ENTRY_NOT_FOUND) {
			memset(ackemessage.Message, 0, ACK_MESSAGE_SIZE * sizeof(_TCHAR));
			wsprintf(ackemessage.Message,L" מסד נתונים מלא\0");
			return Q_ERROR; //database is full
		}
	}

	//Fill Line into Database and handle if all lines were intercepted
	if (FillProRbtDbLine(pProRbtLine, dBEntry) == FALSE) {
			memset(ackemessage.Message, 0, ACK_MESSAGE_SIZE * sizeof(_TCHAR));
			wsprintf(ackemessage.Message,L"Bad session ID");
			InitProRbtDb();
		return Q_ERROR; // Bad session ID intercepted
	}

	//Handle Counter ID Session entry. See if all lines were intercepted. Send command to CONSIS if they have.
	return HandleCounterIdEntry( &(m_DB.CounterSessionArr[dBEntry]), pdialog);
}
