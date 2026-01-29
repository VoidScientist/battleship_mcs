/**
 *	\file		dial.h
 *	\brief		Fichier en-tête représentant les dialogues applicatifs
 *	\author		ARCELON Louis
 *	\date		28 janvier 2026
 *	\version	1.0
 */
#include "logging.h"
#include "dial.h"
#include "protocol.h"
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

	while (1) {

		int status = enum2status(REQ, CONNECT);
		sendRequest(sockAppel, status, POST, "HELLO", NULL);

		rcvResponse(sockAppel, &response);

		switch (getStatusRange(response.id)) {


			case ERR:
				logMessage("An error occurred: %s\n", ERROR, response.data);
				break;


		}
		
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
		rcvRequest(sockDial, &request);
		
		
		switch (request.id) {
				
			default:
				action_t act = getAction(request.id);
				int status = enum2status(ERR, act);
				sendResponse(sockDial, status, "Unable to connect", NULL);
				break;
			
		}
		
	}

}