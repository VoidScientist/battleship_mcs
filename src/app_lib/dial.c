/**
 *	\file		dial.h
 *	\brief		Fichier en-tête représentant les dialogues applicatifs
 *	\author		ARCELON Louis
 *	\date		28 janvier 2026
 *	\version	1.0
 */
#include <semaphore.h>
#include <string.h>
#include "logging.h"
#include "dial.h"
#include "protocol.h"
#include "datastructs.h"
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   M A C R O S
 */
/**
 *	\def		PAUSE(msg)
 *	\brief		Macro-fonction qui affiche msg et attend une entrée clavier  
 */
#define PAUSE(msg)	printf("%s [Appuyez sur entrée pour continuer]", msg); getchar();



volatile sig_atomic_t mustDisconnect = 0;


/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */
/**
 * \fn 			dialCltE2Srv()
 * \brief       fonction s'occupant du dialogue entre le client et le serveur d'enregistrement
 * 
 * \param		sockAppel		structure socket_t contenant le descripteur
 * 								de fichier de la socket d'appel
 * 
 * \note 		s'occupe donc de l'envoi de requêtes et réception de réponses
 */
void dialClt2SrvE(eCltThreadParams_t *params) {

	int 			status;
	rep_t 			response;


	socket_t	 *sockAppel		= params->sockAppel;
	clientInfo_t *infos 		= params->infos;
	sem_t 		 *semCanClose	= params->semCanClose;

	free(params);

	// logMessage("Client: %s, %d, %s, %d\n", DEBUG, infos->name, infos->role, infos->address, infos->port);


	status = enum2status(REQ, CONNECT);
	sendRequest(sockAppel, status, POST, infos, (pFct) clientInfo2str);
	

	rcvResponse(sockAppel, &response);
	if (response.id != enum2status(ACK, CONNECT)) {
		logMessage("[%d] Failed to connect: %s.\n", DEBUG, response.id, response.data);
		sem_post(semCanClose);
		return;
	}


	while (1) {
		

		if (mustDisconnect) {

			status = enum2status(REQ, CONNECT);
			sendRequest(sockAppel, status, DELETE, "", NULL);
			rcvResponse(sockAppel, &response);

			if (response.id == enum2status(ACK, CONNECT)) {
			
				logMessage("[%d] %s\n", DEBUG, response.id, response.data);
				sem_post(semCanClose);
				mustDisconnect = 0;
			
				break;
			
			} else {
			
				logMessage("[%d] %s\n", DEBUG, response.id, response.data);
				return;
			
			}
			
		}

		
	}

}
/**
 * \fn 			dialSrv2Clt()
 * \brief       fonction s'occupant du dialogue entre le serveur d'enregistrement et le client
 * 
 * \param		sockDial		structure socket_t contenant les informations
 * 								sur la socket de dialogue avec le client
 * 
 * \note		s'occupe donc de l'envoi de réponses et réception de réponses
 */
void dialSrvE2Clt(eServThreadParams_t *params) {

	int running				= 1;

	int 			id 			= params->id;
	socket_t 		*sockDial 	= params->sockDial;
	clientInfo_t 	*clients 	= params->clientArray;
	clientInfo_t 	*client 	= &clients[id];			// dangereux lorsque serv enr. plein. mais soit.

	void (*onDisconnect)(int) 	= params->terminationCallback;
	int  (*canAccept)()			= params->canAccept;


	free(params);
	

	while(running)	// daemon !
	{	
		
		req_t request;		
		rcvRequest(sockDial, &request);
		
		
		switch (request.id) {
			
			case 101:

				int status;
				
				if (request.verb == POST) {

					if (!canAccept()) {
						status = enum2status(ERR, CONNECT);
						sendResponse(sockDial, status, "Serveur d'enregistrement plein.", NULL);
						client->status = DISCONNECTED;
						running = 0;
						break;
					}
					
					str2clientInfo(request.data, client);
					client->status = CONNECTED;

					// logMessage("Client connecté: %s, %d, %s, %d\n", DEBUG, client->name, client->status, client->address, client->port);

					status = enum2status(ACK, CONNECT);
					sendResponse(sockDial, status, "Connexion réussie", NULL);
					break;

				}

				if (request.verb == DELETE) {

					running = 0;

					client->status = DISCONNECTED;

					status = enum2status(ACK, CONNECT);
					sendResponse(sockDial, status, "Déconnexion réussie", NULL);
					onDisconnect(id);
					break;

				}

				break;


			default:
				action_t act = getAction(request.id);
				status = enum2status(ERR, act);
				sendResponse(sockDial, status, "Code de status non géré", NULL);
				break;
			
		}
		
	}

	// Fermer la socket de dialogue
	close(sockDial->fd);

	// supprimer la socket_t du heap
	free(sockDial);

}