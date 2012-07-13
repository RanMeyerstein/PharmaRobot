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
	int ConnectToConsis(char* clientName, CListBox * dlglistBox); 
	int SendStockQuery(char* MessageContent);

private:

	char clinetname[256];
	CListBox * m_dlglistBox;
	struct addrinfo *result;
	HINSTANCE hinstLibAce,hinstLibCD;

};

#endif //ifndef __CONSISCOMM_H
