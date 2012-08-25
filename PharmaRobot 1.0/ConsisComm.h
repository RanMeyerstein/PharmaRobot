#ifndef __CONSISCOMM_H
#define __CONSISCOMM_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>


__declspec(align(8)) 

/************** A type Start ***********/
struct AConsisReplyHeader{

	char RecordType;
	char OrderNumber[8];
	char DemandingCounterUnitId[3];
	char DispenserStation[3];
	char Priority;
	char NumberOfArticles[2];
};

struct AConsisReplyDispensedOcc{

	char PZN[7];
	char DispensedQuantity[5];
	char Flag;
	char ArticleId[30];
};
/************** A type End ***********/

/************** a type Start ***********/
struct aConsisReplyHeader{

	char RecordType;
	char OrderNumber[8];
	char DemandingCounterUnitId[3];
	char DispenserStation[3];
	char OrderState[2];
	char NumberOfArticles[2];
};

struct aConsisReplyDispensedOcc{

	char PZN[7];
	char DispensedQuantity[5];
	char ArticleId[30];
};
/************** a type End ***********/


/************** B type Start ***********/
struct BConsisStockRequest{

	char RecordType;
	char DemandingCounterUnitId[3];
	char PZN[7];
	char ArticleId[30];
};
/************** B type End ***********/

/************** b type Start ***********/
struct bConsisReplyHeader{

	char RecordType;
	char DemandingCounterUnitId[3];
	char PZN[7];
	char TotalQuantity[5];
	char NumStockLocations[2];
};

struct bConsisReplyStockLocations{

	char StockLocation[10];
	char Capacity[5];
	char PartialStockAtLocation[5];
	char Expirydate[6];
};

struct bConsisReplyFooter{

	char ArticleId[30];
};
/************** b type End ***********/

class ConsisComm{

public:
	ConsisComm();
	~ ConsisComm();

	bool ConnectionStarted;
	bool DllsLoaded;
	int ConnectToConsis(char* clientName, CListBox * dlglistBox); 
	int SendStockQuery(char* MessageContent);
	int SendDispnseCommand(char* MessageContent);
	BOOL SendConsisMessage(char* MessageContent, size_t BufferSize);	
	BOOL ReceiveConsisMessage(char* ReceiveBuffer, int * messageLength);

private:

	char clinetname[256];
	CListBox * m_dlglistBox;
	struct addrinfo *result;
	HINSTANCE hinstLibAce,hinstLibCD;

};

#endif //ifndef __CONSISCOMM_H
