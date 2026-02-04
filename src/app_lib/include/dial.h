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


/**
 * @brief Paramètres du thread serveur de jeu pour dialogue avec un client
 */
typedef struct {
	int 				equipeId;				/**< Id de l'équipe du joueur */
	int 				numeroJoueur;			/**< Numéro du joueur */
	socket_t 			*sockDial;				/**< Socket de dialogue avec le client */
	Jeu 				*jeu;					/**< Pointeur vers l'état du jeu */
	socket_t 			*clientsSockets;		/**< Tableau des sockets clients */
	int 				*nbClientsConnectes;	/**< Nombre de clients connectés */
	int 				*phasePlacementTermine;	/**< Etat du placement par équipe */
	pthread_mutex_t 	*mutexJeu;				/**< Mutex de protection du jeu */
} gServThreadParams_t;

/**
 * @brief Paramètres du thread client de jeu pour dialogue avec le serveur
 */
typedef struct {
	socket_t 				*sockAppel;				/**< Socket de connexion au serveur */
	int 					*equipeId;				/**< Id de l'équipe */
	Resultat 				*dernierResultat;		/**< Résultat du dernier tir reçu */
	Tour 					*tourActuel;			/**< Tour actuel de la partie */
	sem_t 					*semCanClose;			/**< Sémaphore de fermeture propre */
	sem_t 					*semPlacementOk;		/**< Sémaphore de validation placement */
	sem_t 					*semTirResultat;		/**< Sémaphore de résultat de tir */
	sem_t 					*semTourActuel;			/**< Sémaphore de changement de tour */
	sem_t					*semStartGame;			/**< Sémaphore de début de partie */
	sem_t					*semTourPlacement;		/**< Sémaphore de tour de placement */
	int						*monTourPlacement;		/**< Indicateur de tour de placement */
	Jeu						*jeu;					/**< Pointeur vers l'état du jeu */
	int                     *attendsResultatTir;	/**< Indicateur d'attente de résultat */
} gCltThreadParams_t;

/**
 * @brief 	flag de déconnexion, fais pour être appelé dans un traitement de signal.
 */
extern volatile sig_atomic_t mustDisconnect;
/**
 * @brief 	flag de récupération des hôtes. {CONNECT GET}
 */
extern int requestHosts;
extern volatile int partieTerminee;
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

void envoyerATous(socket_t *sockets, int nb, int status, void *data, pFct serializer);

void envoyerAEquipe(socket_t *sockets, int nb, int equipeId, int status, void *data, pFct serializer);

void envoyerACoequipiers(socket_t *sockets, int nb, int equipeId, int numJoueur, int status, void *data, pFct serializer);

void envoyerAAdversaires(socket_t *sockets, int nb, int equipeId, int status, void *data, pFct serializer);

int calculerProchainJoueur(int equipeId, int nbClients);

void envoyerNextTurnPlacement(socket_t *sockets, int nb, int prochainJoueur);

void demarrerPhaseJeu(socket_t *sockets, int nb);

void envoyerNextTurnJeu(socket_t *sockets, int nb, int equipe);

void envoyerVictoire(socket_t *sockets, int nb, int equipeGagnante, Jeu *jeu);

void traiterConnexion(socket_t *sockDial, Jeu *jeu, int equipeId, int numeroJoueur, req_t *request);

void traiterDeconnexion(socket_t *sockDial, int *running);

int traiterPlacement(socket_t *sockDial, socket_t *clientsSockets, int nbClients, Jeu *jeu, int equipeId, int numeroJoueur, int *phasePlacementTermine, req_t *request);

void traiterTir(socket_t *sockDial, socket_t *clientsSockets, int nbClients, Jeu *jeu, req_t *request);

#endif /* DIAL_H */
