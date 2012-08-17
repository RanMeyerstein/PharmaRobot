#ifndef __CONSISCOMM_H
#define __CONSISCOMM_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

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
