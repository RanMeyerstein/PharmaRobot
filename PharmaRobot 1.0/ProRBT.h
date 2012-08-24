#include "stdafx.h"
#include <afx.h>

#define MAXIMAL_NUM_LINES_SUPPORTED 101
#define MAXIMAL_NUM_COUNTERS_SUPPORTED 18
#define ENTRY_NOT_FOUND -1

struct PRORBTPARAMS
{
	_TCHAR Header[1],Barcode[14], Qty[4], SessionId[17], LineNum[5], TotalLines[5], Directive[2], CounterUnit[4];
};

struct PRORBTCOUNTERSESSION{
	int CurrentSessionId;
	int PreviousSessionId;
	int ExpectedNumLines;
	int ReceivedNumLines;
	PRORBTPARAMS RbtParamLinesArr[MAXIMAL_NUM_LINES_SUPPORTED];
};


struct PRORBTDB {
	int CounterIdToSessionEntryArr[MAXIMAL_NUM_COUNTERS_SUPPORTED];
	PRORBTCOUNTERSESSION CounterSessionArr[MAXIMAL_NUM_COUNTERS_SUPPORTED];
};

class ProRbtDb
{
public:
	PRORBTDB m_DB;
	void InitCounterSession (PRORBTCOUNTERSESSION* pCounterSession);
	void InitProRbtDb ();
	BOOL FreeProRbtDbEntry (int CounterId);
	int GetProRbtDbEntryFromCounterId (int counterid);
	int AcquireProRbtDbEntry(int counterid);
	BOOL FillProRbtDbLine (PRORBTPARAMS* pProRbtLine, int DBEntry);
	BOOL HandleCounterIdEntry(PRORBTCOUNTERSESSION * pCounterSession);
	BOOL HandleProRbtLine(PRORBTPARAMS* pProRbtLine);

};