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
#include "bataille_navale.h"
#include "logic.h"
#include "structSerial.h"


/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
/**
 * @brief      maximum d'hôtes récupérables dans une commande CONNECT GET 
 */
#define MAX_HOSTS_GET 10
/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */
/**
 * @brief     	structure contenant les paramètres requis au dialogue du serveur 
 * 				d'enregistrement vers un client.
 */
typedef struct {

	/** id du thread, permet de stocker les infos client au bon endroit */
	int 			id; 			
	/** pointeur vers une structure socket_t pour le dialogue */
	socket_t 		*sockDial;
	/** tableau de clients pour envoi et modification (seulement clientArray[id] */
	clientInfo_t 	*clientArray;
	/** nombre de clients (max) */
	int 			clientAmount;
	/** callback de terminaison de thread */
	void 			(*terminationCallback)(int);
	/** fonction pour vérifier si le thread peut accepter l'utilisateur */
	int 			(*canAccept)();

} eServThreadParams_t;
/**
 * @brief      structure de paramètres de dialogue client vers serveur enregistrement.
 */
typedef struct {

	/** socket d'appel du client */
	socket_t 		*sockAppel;
	/** pointeur vers les infos du client */
	clientInfo_t	*infos;
	/** tableau d'hôtes joignables du client */
	clientInfo_t	*hostBuffer;
	/** sémaphore permettant d'autoriser le client à se terminer */
	sem_t 			*semCanClose;
	/** sémaphore signalant la fin d'une requête */
	sem_t 			*semRequestFin;

} eCltThreadParams_t;


typedef struct {
	int 				equipeId;
	int 				numeroJoueur;
	socket_t 			*sockDial;
	Jeu 				*jeu;
	socket_t 			*clientsSockets;
	int 				*nbClientsConnectes;
	int 				*phasePlacementTermine;
	pthread_mutex_t 	*mutexJeu;
} gServThreadParams_t;


typedef struct {
	socket_t 				*sockAppel;
	int 					*equipeId;
	Resultat 				*dernierResultat;
	Tour 					*tourActuel;
	sem_t 					*semCanClose;
	sem_t 					*semPlacementOk;
	sem_t 					*semTirResultat;
	sem_t 					*semTourActuel;
	sem_t					*semStartGame;
	sem_t					*semTourPlacement;
	int						*monTourPlacement;
	Jeu						*jeu;
	int                     *attendsResultatTir;
	volatile sig_atomic_t 	*partieTerminee;
} gCltThreadParams_t;

/**
 * @brief 	flag de déconnexion, fais pour être appelé dans un traitement de signal.
 */
extern volatile sig_atomic_t mustDisconnect;
/**
 * @brief 	flag de récupération des hôtes. {CONNECT GET}
 */
extern int requestHosts;
/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
/**
 * \brief       fonction s'occupant du dialogue entre le client et le serveur d'enregistrement
 * 
 * \param		params   		eCltThreadParams_t contenant les paramètres pour
 * 								le dialogue. Doit être alloué avec `malloc()`
 * 
 * \note 		s'occupe donc de l'envoi de requêtes et réception de réponses
 */
void dialClt2SrvE(eCltThreadParams_t *params);
/**
 * \brief       fonction s'occupant du dialogue entre le serveur d'enregistrement et le client
 * 
 * \param		*params		structure eServThreadParams contenant les paramètres
 * 								pour le dialogue. Doit être alloué avec `malloc()`
 * 
 * \note		s'occupe donc de l'envoi de réponses et réception de réponses
 */
void dialSrvE2Clt(eServThreadParams_t *params);
/**
 * \fn 			dialClt2SrvG()
 * \brief       Dialogue entre le client et le serveur de jeu
 * 
 * \param		params		Paramètres du thread client de jeu
 */
void dialClt2SrvG(gCltThreadParams_t *params);

/**
 * \fn 			dialSrvG2Clt()
 * \brief       Dialogue entre le serveur de jeu et le client
 * 
 * \param		params		Paramètres du thread serveur de jeu
 */
void dialSrvG2Clt(gServThreadParams_t *params);

/**
 * \brief      Envoie une requête via un flag et attends une sémaphore.
 *
 * \param      reqVar     Flag de la requête
 * \param      semReqAck  Sémaphore d'attente
 */
void postRequest(int *reqVar, sem_t *semReqAck);


#endif /* DIAL_H */
