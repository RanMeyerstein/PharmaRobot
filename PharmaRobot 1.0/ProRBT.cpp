
#include "stdafx.h"
#include "ProRBT.h"


PRORBTPARAMSACK ackemessage;

BOOL ProRbtDb::CheckAmountInStock(int * retAmount, char* Barcode, CPharmaRobot10Dlg* pdialog)
{
	size_t convertedChars, origsize;
	CString st;
	wchar_t wcstring[100];
	BConsisStockRequest * pBrequest = (BConsisStockRequest *)pdialog->ConsisMessage;

	//Build B Message
	memset(pdialog->ConsisMessage, '0', 41);
	pdialog->ConsisMessage[41] = '\0';

	/*Counter Unit*/
	pBrequest->DemandingCounterUnitId[3] = '1';

	/*Barcode*/
	memset(pBrequest->ArticleId,' ',30);
	size_t barcodesize = strlen(Barcode);
	memcpy(&(pBrequest->ArticleId[30 - barcodesize]), Barcode, barcodesize);

	pdialog->ConsisMessage[0] = 'B';

	//Print in dialog box the B request
	origsize = 43;
	mbstowcs_s(&convertedChars, wcstring, origsize, pdialog->ConsisMessage, _TRUNCATE);
	st = wcstring; pdialog->m_listBoxMain.AddString(st);

	char buffer[MAX_CONSIS_MESSAGE_SIZE]; buffer[0] = 0;
	int MessageLength;
	while (buffer[0] != 'b')//BUG override. Sometimes the function returns an 'a' response
	{
		/* Send B message to CONSIS */
		if (pdialog->Consis.SendConsisMessage(pdialog->ConsisMessage, 42) == FALSE)
		{
			return FALSE;
		}
		MessageLength = sizeof(buffer);
		pdialog->Consis.ReceiveConsisMessage(buffer, &MessageLength, 1000);
	}
	buffer[MessageLength] = '\0';

	//Print in dialog box the b response
	origsize = MessageLength + 1;
	mbstowcs_s(&convertedChars, wcstring, origsize, buffer, _TRUNCATE);
	st = wcstring; pdialog->m_listBoxMain.AddString(st);

	bConsisReplyHeader *pHeader = (bConsisReplyHeader *)buffer;

	//Extract Total Quantity of Item
	char TotalQua[6];
	TotalQua[5] = '\0';
	memcpy(TotalQua, pHeader->TotalQuantity, sizeof(pHeader->TotalQuantity));
	*retAmount =  atoi(TotalQua);

	return TRUE;
}

BOOL ProRbtDb::BuildAndSendACommand(REQUESTINTERMEIDATEDB * pInteDb, CPharmaRobot10Dlg* pdialog, PRORBTPARAMS* pFirstLine)
{
	AConsisRequestHeader * pARequestHeader;
	int MessageASize, indexInAcommand = 0;;
	CString StringFromProRbt;
	char nstring[100], numart[3];;
	size_t convertedChars;

	//Clear message
	memset(pdialog->ConsisMessage, '0', 448); //18 header + 10 * 43 OCC size

	pARequestHeader = (AConsisRequestHeader *)pdialog->ConsisMessage;

	/*Counter Unit taken from ProRBT parameters*/
	StringFromProRbt = pFirstLine->CounterUnit;//Take counter ID from first line
	int len = StringFromProRbt.GetLength();
	int location = 12 - len;
	wchar_t Source[4];
	wsprintf(Source, StringFromProRbt.GetString());
	wcstombs(&(pdialog->ConsisMessage[location]), Source, len);

	/*Dispenser Taken from ProRBT parameters*/
	StringFromProRbt = pFirstLine->Dispenser;//Take Dispenser ID from first line
	len = StringFromProRbt.GetLength();
	location = 15 - len;
	wsprintf(Source, StringFromProRbt.GetString());
	wcstombs(&(pdialog->ConsisMessage[location]), Source, len);

	//Fill Barcode and Quantity per entry in stock
	int i = pInteDb->firstIndexToStartfrom;
	while (i < pInteDb->sizeRequested)
	{
		if (pInteDb->Entry[i].NumInStock > 0) //only if article is in stock, add it to the 'A' command
		{
			/*Barcode taken from ProRBT parameters*/
			StringFromProRbt = pInteDb->Entry[i].ArctileIdRequested;
			len = StringFromProRbt.GetLength();
			location = (18 + (43 * (indexInAcommand + 1))) - len;
			wchar_t barcodeSent[14];
			wsprintf(barcodeSent, StringFromProRbt.GetString());
			//Fill with blanks instaed of leading zeros
			memset(&(pdialog->ConsisMessage[(18 + (43 * (indexInAcommand + 1))) - 30]),' ', 30);
			//Set the barcode
			wcstombs(&(pdialog->ConsisMessage[location]), barcodeSent, len);

			/*Quantity Taken from ProRBT parameters*/
			wchar_t ReqQuantity[5] = {0,0,0,0,0};
			wsprintf(ReqQuantity,L"%d",pInteDb->Entry[i].NumInStock);
			StringFromProRbt = ReqQuantity;
			len = StringFromProRbt.GetLength();
			location = (30 + (indexInAcommand * 43)) - len;
			wcstombs(&(pdialog->ConsisMessage[location]), ReqQuantity, len);

			/* Clear PZN */
			location = (18 + (43 * indexInAcommand));
			memset(&(pdialog->ConsisMessage[location]),' ', 7);

			indexInAcommand++;//One more article in the 'A' command
		}
		//For Next time start scan from next index
		i += 1;
		pInteDb->firstIndexToStartfrom = i;
		if (indexInAcommand == 10) break; //Break from while, reached maximum allowed number of articles in 'A' command.
	}

	//Message size is a function of the number of articles in this 'A' command
	MessageASize = 18 + (43*(indexInAcommand));
	pdialog->ConsisMessage[indexInAcommand] = '\0';

	/* Number of articles */
	sprintf(numart,"%d",indexInAcommand);
	if (indexInAcommand < 10){
		pARequestHeader->NumberOfArticles[1] = numart[0];
	}
	else {
		pARequestHeader->NumberOfArticles[0] = numart[0];
		pARequestHeader->NumberOfArticles[1] = numart[1];
	}

	/*Order number Taken from GUI*/
	pdialog->m_OrderNum++; if (pdialog->m_OrderNum > 99999999) pdialog->m_OrderNum = 0;//for every request increase number
	WCHAR wideStr[10];
	wsprintf(wideStr,L"%d\0",pdialog->m_OrderNum);
	pdialog->m_EditOrderNum.SetWindowTextW(wideStr);

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

	//Print in dialog box the A request
	origsize = MessageASize + 1;
	wchar_t wcstring[1000];
	mbstowcs_s(&convertedChars, wcstring, origsize, pdialog->ConsisMessage, _TRUNCATE);
	CString st = wcstring; pdialog->m_listBoxMain.AddString(st);

	/* Send 'A' message to CONSIS */
	if (pdialog->Consis.SendConsisMessage(pdialog->ConsisMessage, MessageASize) == FALSE)
		return FALSE;

	return TRUE;
}

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
#ifdef SESSION_ID_CHECKED
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
		pCounterSession->CurrentSessionId = SesId;
	}

	//Session ID OK, Continue to add line to database
#endif //SESSION_ID_CHECKED
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
	size_t retsize, len, convertedChars;
	int MessageLength, NumTenBatches;
	char buffer[MAX_CONSIS_MESSAGE_SIZE], numart[3];
	aConsisReplyHeader *paMesHeader;
	aConsisReplyDispensedOcc* aocc;
	REQUESTINTERMEIDATEDB InterMDb;
	CString StringFromProRbt, st;

	if (pCounterSession->ExpectedNumLines == pCounterSession->ReceivedNumLines)
	{
		//Protect with Mutex the CONSIS resource
		CSingleLock singleLock(&(pdialog->m_Mutex));

		// Attempt to lock the shared resource
		if (singleLock.Lock(INFINITE))
		{
			//log locking success
		}

		if (pdialog->Consis.ConnectionStarted == FALSE)
		{
			if (pdialog->Consis.ConnectToConsis("ShorT", &(pdialog->m_listBoxMain), &(pdialog->m_CheckBoxRemoteSvr)))
				pdialog->EnableCondsisTab();
		}

		if (pdialog->Consis.ConnectionStarted == TRUE)
		{ 
			//Clean intermideate database
			memset (&InterMDb, 0, sizeof(InterMDb));
			InterMDb.firstIndexToStartfrom = INITIAL;

			int l = 0;
			//build intermediate database that agregates identical article IDs (barcodes)
			for (int j = 0 ; j < pCounterSession->ExpectedNumLines ; j++)
			{
				bool entryfound = false;
				while (InterMDb.Entry[l].NumReq != 0) //search for an empty entry indicating the last entry in database
				{
					if (wcscmp(InterMDb.Entry[l].ArctileIdRequested, pCounterSession->RbtParamLinesArr[j + 1].Barcode) == 0)
					{
						InterMDb.Entry[l].NumReq += _wtoi(pCounterSession->RbtParamLinesArr[j+1].Qty); //aggregate number of requested articles
						entryfound = true;
					}
					l++;//Check next entry in intermediate database
				}
				if(entryfound == false)
				{//barcode not found in any of the entries in the database. 
					//Create new entry - copy Barcode number and requested quantity to inetrmediate database.
					StringFromProRbt = pCounterSession->RbtParamLinesArr[j + 1].Barcode;
					len = StringFromProRbt.GetLength();
					memcpy(&(InterMDb.Entry[l].ArctileIdRequested),pCounterSession->RbtParamLinesArr[j+1].Barcode,len*sizeof(wchar_t));
					InterMDb.Entry[l].ArctileIdRequested[len] = L'\0';//insure string ends with a null
					InterMDb.Entry[l].NumReq = _wtoi(&(pCounterSession->RbtParamLinesArr[j+1].Qty[0])); //insert numer of requested articles
					//Check if Item is in Stock
					char BarcodeString[31];	memset(BarcodeString, 0, 30);
					wcstombs(BarcodeString, InterMDb.Entry[l].ArctileIdRequested, len);
					int TotalStock;
					if (CheckAmountInStock(&TotalStock, BarcodeString, pdialog) == FALSE)
					{
						//Error with Consis, Init entire Database
						InitProRbtDb();
						singleLock.Unlock();
						memset(ackemessage.Message, 0, ACK_MESSAGE_SIZE * sizeof(_TCHAR));
						wsprintf(ackemessage.Message,L"תקלה בשליחת שאילתת כמות במלאי לשרת קונסיס");
						return Q_ERROR;
					}
					if (TotalStock >= InterMDb.Entry[l].NumReq) 
					{
						InterMDb.Entry[l].NumInStock = InterMDb.Entry[l].NumReq;
					}
					else
					{
						InterMDb.Entry[l].NumInStock = TotalStock;
					}
					//the Item is in stock and may be dispensed
					if (InterMDb.Entry[l].NumInStock > 0) 
					{
						InterMDb.sizeInStock += 1;
						if (InterMDb.firstIndexToStartfrom == INITIAL)
						{
							InterMDb.firstIndexToStartfrom = l;
						}
					}
				}
				//record intermediate database size for all items inserted to it
				InterMDb.sizeRequested = l + 1;
				l = 0;
			}

			//Handling 'A' dialogue is done in a batches of 10
			NumTenBatches = 1 + (InterMDb.sizeInStock / 10);

			while (NumTenBatches !=0)
			{
				//Build 'A' Message with Barcode from RBT parameters only if there are items left to send
				if (InterMDb.sizeInStock > 0)
				{
					//This function will send the following 10 items in the database that are in stock
					if (BuildAndSendACommand(&InterMDb, pdialog, &(pCounterSession->RbtParamLinesArr[1])) == FALSE)
					{
						//Error with Consis, Init entire Database
						InitProRbtDb();
						singleLock.Unlock();
						memset(ackemessage.Message, 0, ACK_MESSAGE_SIZE * sizeof(_TCHAR));
						wsprintf(ackemessage.Message,L"תקלה בשליחת בקשת ניפוק לשרת קונסיס");
						return Q_ERROR;
					}		

					if (InterMDb.sizeInStock >= 10) InterMDb.sizeInStock -= 10; //10 less in stock

					//Session state depends on the order state
					//Last message Should be '4' - Ready, but Server returns '5', so that's what we'll wait for
					char orderState[3];
					memset(&(orderState[2]),'\0',1);
					memset(orderState,'0',2);

					do
					{
						MessageLength = sizeof(buffer);
						//Break out of while if message reception fails
						if (pdialog->Consis.ReceiveConsisMessage(buffer, &MessageLength, 1000) == FALSE) break;

						buffer[MessageLength] = '\0';
						paMesHeader = (aConsisReplyHeader *)buffer;

						memcpy(orderState, paMesHeader->OrderState, sizeof(paMesHeader->OrderState));
						orderState[2] = '\0';

						//Print the reply of CONSIS 'a' to the dialog box
						size_t origsize = strlen(buffer) + 1;
						wchar_t wcstring[1000];
						mbstowcs_s(&convertedChars, wcstring, origsize, buffer, _TRUNCATE);
						st = wcstring; pdialog->m_listBoxMain.AddString(st);

					}while ((paMesHeader->OrderState[1] != '4') && (paMesHeader->OrderState[1] != '3'));
					//Supposed to wait for quantity change by CONSIS to PMS, but now wait for ready. If Cancelled stop all.

					//'A' command was cancelled. Items are missing from Stock in consis. Find which items are missing
					//using the 'B' command, and build a new 'A' command.
					if (paMesHeader->OrderState[1] == '3')
					{//We checked that all the items are in stock. If Cancelled somthing is wrong in CONSIS
						//Error with Consis, Init entire Database
						InitProRbtDb();
						singleLock.Unlock();
						memset(ackemessage.Message, 0, ACK_MESSAGE_SIZE * sizeof(_TCHAR));
						wsprintf(ackemessage.Message,L"תקלה בקבלת תשובה לבקשת ניפוק לשרת קונסיס");
						return Q_ERROR;
					}
					else //The 'A' request was not cancelled, go through the reply
					{
						//Extract number of locations
						memcpy(numart, paMesHeader->NumberOfArticles, sizeof(paMesHeader->NumberOfArticles));
						numart[2] = '\0';
						int numArticles =  atoi(numart);

						//TEMP RANM DEBUG
						wchar_t Stringnumdif1f[3]; _itow(numArticles,Stringnumdif1f,10);
						Stringnumdif1f[2] = '\0';
						st = L"numArticles = "; st += Stringnumdif1f; pdialog->m_listBoxMain.AddString(st);
						//TEMP RANM DEBUG

						for (int i = 0 ; i < numArticles ; i++ )
						{
							//Extract Article ID and Dispensed quantity from Occupancy entry in 'a' message occupany entry
							char* address = (char*)paMesHeader + sizeof(aConsisReplyHeader) + (i * 42);
							aocc = (aConsisReplyDispensedOcc*)address;

							//Get article ID from 'a' response
							wchar_t articleID[31];
							articleID[30] = '\0';
							mbstowcs_s(&retsize, articleID, sizeof(aocc->ArticleId) + 1, aocc->ArticleId, _TRUNCATE);

							//Clean leading zeroes
							CString cleanArticleID;
							cleanArticleID.SetString(articleID);
							cleanArticleID.TrimLeft(L'0');
							cleanArticleID.TrimLeft(L' ');
							wsprintf(articleID,cleanArticleID.GetString());
							articleID[cleanArticleID.GetLength()] = L'\0';

							//find article ID in intermediate database
							int k = 0;
							while (InterMDb.Entry[k].NumReq != 0)
							{
								if (wcscmp(articleID, InterMDb.Entry[k].ArctileIdRequested) == 0)
								{//found the entry accoridng to barcode in intermedeate database
									//Dispensed quantity handling. Fill intermediate database for handling at the end.
									char dispenssedQuantity[6];
									memcpy(dispenssedQuantity, aocc->DispensedQuantity, sizeof(aocc->DispensedQuantity));
									dispenssedQuantity[5] = '\0';
									//InterMDb.Entry[k].NumDis += atoi(dispenssedQuantity); //CONSIS don't support the dispenssed RANM BUG in consis!!!
									//IF NO CANCELLED STATE RECEIVED THEN ALL ITEMS WERE DISPENSSED
									InterMDb.Entry[k].NumDis = InterMDb.Entry[k].NumInStock; //CONSIS don't support the dispenssed RANM BUG in consis!!!
								}
								k++;//serach for article ID in the intermediate database further
							}
						}

					}

				}
				NumTenBatches--;
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

		//CONSIS Completed, Build Ack from intermediate database. Check if all entries in the aggregated database were dispensed.
		int m = 0;
		bool missingitems = false;
		CString AckedString = L"מבקשת הניפוק חסרים הפריטים הבאים"; AckedString += L":";AckedString += L"\n";
		while (InterMDb.Entry[m].NumReq != 0)
		{
			if (InterMDb.Entry[m].NumReq != InterMDb.Entry[m].NumDis)
			{
				missingitems = true;
				wchar_t description[256];
				//Get Description from Yarpa SQL. If failed publish back to Client the Barcode
				if (!pdialog->GetItemDescFromBarcode(InterMDb.Entry[m].ArctileIdRequested, description))
					wsprintf(description,  InterMDb.Entry[m].ArctileIdRequested);
				AckedString += L"סוג פריט"; AckedString += L":"; AckedString += L" ";
				AckedString += description; AckedString += L"\n";
				AckedString += L"חסרים"; AckedString += L":"; AckedString += L" ";
				wchar_t Stringnumdiff[3]; _itow(InterMDb.Entry[m].NumReq - InterMDb.Entry[m].NumDis,Stringnumdiff,10);
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
