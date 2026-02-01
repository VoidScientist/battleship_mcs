/**
 *	\file		dial.h
 *	\brief		Fichier en-tête représentant les dialogues applicatifs
 *	\author		ARCELON Louis
 *	\date		28 janvier 2026
 *	\version	1.0
 */
#ifndef DIAL_H
#define DIAL_H
/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include <semaphore.h>
#include <signal.h>
#include "data.h"
#include "repReq.h"
#include "datastructs.h"


#define MAX_HOSTS_GET 10


typedef struct {

	int 			id;
	socket_t 		*sockDial;
	clientInfo_t 	*clientArray;
	int 			clientAmount;
	void 			(*terminationCallback)(int);
	int 			(*canAccept)();

} eServThreadParams_t;


typedef struct {

	socket_t 		*sockAppel;
	clientInfo_t	*infos;
	clientInfo_t	*hostBuffer;
	sem_t 			*semCanClose;
	sem_t 			*semRequestFin;

} eCltThreadParams_t;



extern volatile sig_atomic_t mustDisconnect;

extern int requestHosts;
/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
/**
 * \fn 			dialCltE2SrvE()
 * \brief       fonction s'occupant du dialogue entre le client et le serveur d'enregistrement
 * 
 * \param		sockAppel		structure socket_t contenant le descripteur
 * 								de fichier de la socket d'appel
 * 
 * \note 		s'occupe donc de l'envoi de requêtes et réception de réponses
 */
void dialClt2SrvE(eCltThreadParams_t *params);
/**
 * \fn 			dialSrvE2Clt()
 * \brief       fonction s'occupant du dialogue entre le serveur d'enregistrement et le client
 * 
 * \param		*params		structure eServThreadParams contenant les paramètres
 * 								pour le dialogue. Doit être alloué avec malloc()
 * 
 * \note		s'occupe donc de l'envoi de réponses et réception de réponses
 */
void dialSrvE2Clt(eServThreadParams_t *params);

/**
 * \fn 			dialSrv2Clt()
 * \brief       fonction s'occupant du dialogue entre le serveur d'enregistrement et le client
 * 
 * \param		*params		structure eServThreadParams contenant les paramètres
 * 								pour le dialogue. Doit être alloué avec malloc()
 * 
 * \note		s'occupe donc de l'envoi de réponses et réception de réponses
 */
void handleResponseSrvE2Clt();


/**
 * \brief      Envoie une requête via un flag et attends une sémaphore.
 *
 * \param      reqVar     Flag de la requête
 * \param      semReqAck  Sémaphore d'attente
 */
void postRequest(int *reqVar, sem_t *semReqAck);


#endif /* DIAL_H */