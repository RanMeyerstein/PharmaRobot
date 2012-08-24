
#include "stdafx.h"
#include "ProRBT.h"


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
		if (pCounterSession->CurrentSessionId != SesId) 
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

BOOL ProRbtDb::HandleCounterIdEntry(PRORBTCOUNTERSESSION * pCounterSession)
{
	if (pCounterSession->ExpectedNumLines == pCounterSession->ReceivedNumLines)
	{
		//Build CONSIS command, send it and wait for CONSIS to complete it

		//CONSIS Completed, Build Ack

		//Prepare Database Line Entry for next Session
		pCounterSession->PreviousSessionId = pCounterSession->CurrentSessionId;
		pCounterSession->ExpectedNumLines = -1;
		pCounterSession->ReceivedNumLines = -1;
		pCounterSession->CurrentSessionId = -1;
	}

	return TRUE;
}


BOOL ProRbtDb::HandleProRbtLine(PRORBTPARAMS* pProRbtLine)
{
	int counterid = _wtoi(pProRbtLine->CounterUnit);

	int dBEntry = GetProRbtDbEntryFromCounterId(counterid);

	if (dBEntry == ENTRY_NOT_FOUND)
	{//Entry not found, produce new one
		dBEntry = AcquireProRbtDbEntry(counterid);
		if (dBEntry == ENTRY_NOT_FOUND) {
			return FALSE; //database is full
		}
	}

	//Fill Line into Database and handle if all lines were intercepted
	if (FillProRbtDbLine(pProRbtLine, dBEntry) == FALSE) {
		return FALSE; // Bad session ID intercepted
	}

	//Handle Counter ID Session entry. See if all lines were intercepted. Send command to CONSIS if they have.
	HandleCounterIdEntry( &(m_DB.CounterSessionArr[dBEntry]));

	return TRUE;
}
