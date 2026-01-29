/**
 *	\file		dial.h
 *	\brief		Fichier en-tête représentant les dialogues applicatifs
 *	\author		ARCELON Louis
 *	\date		28 janvier 2026
 *	\version	1.0
 */
#include "logging.h"
#include "dial.h"
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
#define ASK 100
#define OK 200
#define NOK 300

#define CONNECT 0
#define DISCONNECT 1
#define SHOOT 2
#define REVEAL 3
#define START 4
#define END 5
#define NEXT 6
#define PLACE 7
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   M A C R O S
 */
/**
 *	\def		PAUSE(msg)
 *	\brief		Macro-fonction qui affiche msg et attend une entrée clavier  
 */
#define PAUSE(msg)	printf("%s [Appuyez sur entrée pour continuer]", msg); getchar();
/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */
/**
 * \fn 			dialClt2Srv()
 * \brief       fonction s'occupant du dialogue entre le client et le serveur
 * 
 * \param		sockAppel		structure socket_t contenant le descripteur
 * 								de fichier de la socket d'appel
 * 
 * \note 		s'occupe donc de l'envoi de requêtes et réception de réponses
 */
void dialClt2Srv(socket_t *sockAppel) {

	rep_t response;
	req_t request;

	while (1) {

		sendRequest(sockAppel, ASK, CONNECT, "HELLO", NULL);

		rcvResponse(sockAppel, &response);
		
	}

}
/**
 * \fn 			dialSrv2Clt()
 * \brief       fonction s'occupant du dialogue entre le serveur est le client
 * 
 * \param		sockDial		structure socket_t contenant les informations
 * 								sur la socket de dialogue avec le client
 * 
 * \note		s'occupe donc de l'envoi de réponses et réception de réponses
 */
void dialSrv2Clt(socket_t *sockDial) {

	while(1)	// daemon !
	{	
		
		req_t request;
		rep_t response;
		
		rcvRequest(sockDial, &request);
		
		
		switch (request.id) {
			
			case 100:
				response = creerReponse(OK, "ACCEPTED", NULL);
				break;
				
			case 101:
				response = creerReponse(NOK, "REFUSED", NULL);
				break;
				
			default:
				response = creerReponse(NOK, "INVALID", NULL);
				break;
			
		}
		
		envoyer(sockDial, (generic) &response, (pFct) rep2str);
	}

}