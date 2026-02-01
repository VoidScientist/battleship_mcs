/**
 *	\file		session.c
 *	\brief		Spécification de la couche Session
 *	\author		ARCELON Louis
 *	\date		6 janvier 2026
 *	\version	1.0
 */
#include "session.h"
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   M A C R O S
 */
/**
 *	\def		CHECK(sts, msg)
 *	\brief		Macro-fonction qui vérifie que sts est égal -1 (cas d'erreur : sts==-1) 
 *				En cas d'erreur, il y a affichage du message adéquat et fin d'exécution  
 */
#define CHECK(sts, msg) if ((sts)==-1) {perror(msg); exit(-1);}
/**
 *	\def		PAUSE(msg)
 *	\brief		Macro-fonction qui affiche msg et attend une entrée clavier  
 */
#define PAUSE(msg)	printf("%s [Appuyez sur entrée pour continuer]", msg); getchar();
/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

void adr2struct(struct sockaddr_in *addr, char *adrIP, short port) {
	
#ifdef DEBUG_ENABLED
	logMessage("Création de l'addresse: [%s:%d].\n", DEBUG, adrIP, port);
#endif

	addr->sin_family = PF_INET;
	addr->sin_port = htons (port);
	addr->sin_addr.s_addr = inet_addr(adrIP);
	memset(&addr->sin_zero, 0, 8);

}


socket_t creerSocket(int mode) {
	
	socket_t newSocket = {0};
	
	CHECK(newSocket.fd=socket(PF_INET, mode, 0), "Can't create");
	newSocket.mode = mode;

#ifdef DEBUG_ENABLED
	logMessage("Création de la socket N°%d de famille [PF_INET] et mode [%d].\n", DEBUG, newSocket.fd, newSocket.mode);
#endif

	return newSocket;
	
}


socket_t creerSocketAdr (int mode, char *adrIP, short port) {
	
	socket_t newSocket = creerSocket(mode);

	adr2struct(&newSocket.addrLoc, adrIP, port);

	// permet de relancer une socket d'écoute à la même addresse
	// directement après fin de programme.
	int opt = 1;
	setsockopt(newSocket.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	CHECK(
		bind(newSocket.fd
		, (struct sockaddr *) &newSocket.addrLoc
		, sizeof(newSocket.addrLoc))
		, "Can't bind"
	);

#ifdef DEBUG_ENABLED
	logMessage("Création de la socket: [%s:%d].\n", DEBUG, adrIP, port);
#endif

	return newSocket;
	
}


socket_t creerSocketEcoute (char *adrIP, short port) {
	
	socket_t socketEcoute = creerSocketAdr(SOCK_STREAM, adrIP, port);
	
	CHECK(listen(socketEcoute.fd, 5), "Can't listen");

#ifdef DEBUG_ENABLED
	logMessage("Création de la socket d'écoute: [%s:%d].\n", DEBUG, adrIP, port);
#endif
	
	return socketEcoute;
	
}


socket_t accepterClt (const socket_t sockEcoute) {
	
	socket_t sockDialogue = creerSocket(SOCK_STREAM);
	socklen_t sockLen = sizeof(struct sockaddr_in);
	
	CHECK(
		sockDialogue.fd = accept(
			sockEcoute.fd
			, (struct sockaddr *) &sockDialogue.addrDst
			, &sockLen
		)
		, "Can't accept"
	);
	
#ifdef DEBUG_ENABLED
	logMessage(
		"Création de la socket dialogue n°%d et d'adresse: [%s:%d].\n"
		, DEBUG
		, sockDialogue.fd
		, inet_ntoa(sockDialogue.addrDst.sin_addr)
		, ntohs(sockDialogue.addrDst.sin_port)
	);
#endif
	
	sockDialogue.addrLoc = sockEcoute.addrLoc;
	
	return sockDialogue;
}


socket_t connecterClt2Srv (char *adrIP, short port) {
	
	socket_t sockAppel = creerSocket(SOCK_STREAM);
	socklen_t sockLen = sizeof(struct sockaddr_in);
	
	adr2struct(&sockAppel.addrDst, adrIP, port);
	
	CHECK(
		connect(sockAppel.fd
			, (struct sockaddr *) &sockAppel.addrDst
			, sockLen
		)
		, "Can't connect"
	);
	
	CHECK(
		getsockname(
			sockAppel.fd
			, (struct sockaddr *) &sockAppel.addrLoc
			, &sockLen
		)
		, "Can't get sock name"
	);
	
#ifdef DEBUG_ENABLED
	logMessage(
		"- Création de la socket d'appel n°%d et d'adresse: [%s:%d].\n"
		"- Connectée à [%s:%d]\n"
		, DEBUG
		, sockAppel.fd
		, inet_ntoa(sockAppel.addrLoc.sin_addr)
		, ntohs(sockAppel.addrLoc.sin_port)
		, inet_ntoa(sockAppel.addrDst.sin_addr)
		, ntohs(sockAppel.addrDst.sin_port)
	);
#endif
	
	return sockAppel;
	
	
}


void getIpAddress(char *ipBuffer) {

	char hostbuffer[256];
    struct hostent *host_entry;
    int hostname;

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));

    host_entry = gethostbyname(hostbuffer);

    strcpy(
    	ipBuffer
    	, inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]))
    );

}