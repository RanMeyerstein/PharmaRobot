#ifndef __CONSISCOMM_H
#define __CONSISCOMM_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#define MAX_CONSIS_MESSAGE_SIZE 1024

__declspec(align(8)) 

/**************** DISPENSE DIALOGUE *************/
/************** A type Start Requested by PMS (this application) ***********/
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

/************** a type Start Answered by CONSIS ***********/
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


/**************** STOCK DIALOGUE *************/
/************** B type Start  Requested by PMS (this application) ***********/
struct BConsisStockRequest{

	char RecordType;
	char DemandingCounterUnitId[3];
	char PZN[7];
	char ArticleId[30];
};
/************** B type End ***********/

/************** b type Start Answered by CONSIS ***********/
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


/**************** PRODUCT INFORMATION DIALOGUE *************/
/************** p type - Requested by CONSIS ***********/
struct pConsisRequestMessage{

	char RecordType;                //'p'
	char DemandingCounterUnitId[3];
	char PZN[7];                    //Always zeroes
	char EAN[13];                   //Blank - Not relevant for IL
	char ArticleId[30];             //Our Barcode
};
/************** p type End ***********/

/************** P type - Answered by PMS (this application) ***********/
struct PConsisPmsResponseMessage{

	char RecordType;                //'P'
	char DemandingCounterUnitId[3];
	char PZN[7];                    //Always zeroes
	char ArticleName[40];           //Extracted from Yarpa DB
	char AbrDosageForm[3];          //Zeroed
	char PackageUnit[10];           //Zeroed
	char EAN[13];                   //Blank - Not relevant for IL
	char ExpiryDate[6];             //DDMMYY
	char ArticleId[30];             //Our Barcode
};
/************** P type End ***********/


/**************** STOCK INFORMATION DIALOGUE *************/
/************** i type - Requested by CONSIS ***********/
struct iConsisRequestMessage{

	char RecordType;                //'i'
	char OrderNumber[8];
	char DemandingCounterUnitId[3];
	char DeliveryNoteNumber[12];    //Text format
	char PZN[7];                    //Always zeroes
	char EAN[13];                   //Blank - Not relevant for IL
	char Quantity[5];
	char ExpiryDate[6];             //DDMMYY
	char OrderState[2];             /*'00' Receipt of goods     '01' Returns              '02' Start of new supply 
									  '03' End of supply        '04' New article (return) '05' New article (goods receipt) 
									  '06' Ready                '07' Cancelled */
	char ArticleId[30];             //Our Barcode
	char Barcode[20];               //Zeroed, we use article id
};
/************** i type End ***********/

/************** I type - Answered by PMS (this application) ***********/
struct IConsisPmsResponseMessage{

	char RecordType; 	//'I'
	char OrderNumber[8];
	char DemandingCounterUnitId[3];
	char PZN[7];                    //Always zeroes
	char Quantity[5];
	char ExpiryDate[6];             //DDMMYY
	char State[2];                  /*'00' article may be stored '01' article may not be stored 
									  '02' article must be stored with expiry date */
	char Text[4];                   // zeroed assuming no text - 3 characters + data + \0 Line < 70 characters Height < 18 characters
	char ArticleId[30];             //Our ID
};
/************** I type End ***********/

class ConsisComm{

public:
	ConsisComm();
	~ ConsisComm();

	bool ConnectionStarted;
	bool DllsLoaded;
	int ConnectToConsis(char* clientName, CListBox * dlglistBox, CButton * remotebutton); 
	int SendStockQuery(char* MessageContent);
	int SendDispnseCommand(char* MessageContent);
	BOOL SendConsisMessage(char* MessageContent, size_t BufferSize);	
	BOOL ReceiveConsisMessage(char* ReceiveBuffer, int * messageLength, int timeout);

private:

	char clinetname[256];
	CListBox * m_dlglistBox;
	struct addrinfo *result;
	HINSTANCE hinstLibAce,hinstLibCD;

};

#endif //ifndef __CONSISCOMM_H
