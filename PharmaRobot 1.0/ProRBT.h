#ifndef __PRORBT_H
#define __PRORBT_H

#include "stdafx.h"
#include "PharmaRobot 1.0.h"
#include "PharmaRobot 1.0Dlg.h"
#include "ConsisComm.h"
#include <afx.h>



#define MAXIMAL_NUM_LINES_SUPPORTED 101
#define MAXIMAL_NUM_COUNTERS_SUPPORTED 18
#define ENTRY_NOT_FOUND -1

typedef enum QUERYRESPONSE
{
	Q_ERROR = 0,
	Q_NOACK,
	Q_SENDACK
};

struct PRORBTPARAMS
{
	_TCHAR Header[1],Barcode[14], Qty[4], SessionId[17], LineNum[5], TotalLines[5], Directive[2], CounterUnit[4], Dispenser[3];
};

#define ACK_MESSAGE_SIZE 980

struct PRORBTPARAMSACK
{
	_TCHAR Header[1],Type[1], Message[ACK_MESSAGE_SIZE];
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
	ProRbtDb() {
		InitProRbtDb();
	};

	PRORBTDB m_DB;

	void InitCounterSession (PRORBTCOUNTERSESSION* pCounterSession);
	void InitProRbtDb ();
	BOOL FreeProRbtDbEntry (int CounterId);
	int GetProRbtDbEntryFromCounterId (int counterid);
	int AcquireProRbtDbEntry(int counterid);
	BOOL FillProRbtDbLine (PRORBTPARAMS* pProRbtLine, int DBEntry);
	QUERYRESPONSE HandleCounterIdEntry(PRORBTCOUNTERSESSION * pCounterSession, CPharmaRobot10Dlg* pdialog);
	QUERYRESPONSE HandleProRbtLine(PRORBTPARAMS* pProRbtLine, CPharmaRobot10Dlg* pdialog);

};

#endif //__PRORBT_H
