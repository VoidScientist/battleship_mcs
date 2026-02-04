/**
 *	\file		dial.c
 *	\brief		Fichier implémentation représentant les dialogues applicatifs
 *	\author		ARCELON Louis / MARTEL Mathieu
 *	\date		28 janvier 2026
 *	\version	2.0
 */
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "logging.h"
#include "dial.h"
#include "protocol.h"
#include "datastructs.h"
#include "bataille_navale.h"
#include "logic.h"

/*
*****************************************************************************************
 *	\noop		C O N S T A N T E S
 */
// Codes de status
/** @brief Code de requête de connexion */
#define REQ_CONNECT		101
/** @brief Code d'ack de réception de connexion */
#define ACK_CONNECT		201
/** @brief Code d'erreur de connexion */
#define ERR_CONNECT		301

/** @brief Code de requête de placement */
#define REQ_PLACE		102
/** @brief Code d'ack de réception de placement */
#define ACK_PLACE		202
/** @brief Code d'erreur de placement */
#define ERR_PLACE		302

/** @brief Code de requête de tir */
#define REQ_SHOOT		103
/** @brief Code d'ack de réception de tir */
#define ACK_SHOOT		203

/** @brief Code de changement de tour */
#define ACK_NEXT_TURN	204
/** @brief Code de fin de partie */
#define ACK_END_GAME	205
/** @brief Code de début de partie */
#define ACK_START_GAME	206

// Délais
/** @brief Délai entre l'envoi de messages (10 ms) */
#define DELAY_MESSAGE_US		10000
/** @brief Délai avant broadcast aux adversaires (50 ms) */
#define DELAY_BROADCAST_US		50000
/** @brief Délai après la fin du placement (2 s) */
#define DELAY_PLACEMENT_END_S	2

// Équipes
/** @brief Id de l'équipe A */
#define EQUIPE_A	0
/** @brief Id de l'équipe B */
#define EQUIPE_B	1

// Phases
/** @brief Phase de placement des bateaux */
#define PHASE_PLACEMENT	0
/** @brief Phase de jeu (bataille) */
#define PHASE_JEU		1

// Tailles buffer
/** @brief Taille standard des buffers */
#define BUFFER_SIZE		100
/** @brief Taille des petits buffers */
#define BUFFER_SMALL	20

/*
*****************************************************************************************
 *	\noop		V A R I A B L E S   G L O B A L E S
 */

/** @brief Suivi du joueur courant pour le placement dans chaque équipe */
int tourPlacementJoueur[2] = {0, 0};

/** @brief Flag de récupération des hosts */
int requestHosts;

/** @brief Flag de demande de déconnexion */
volatile sig_atomic_t mustDisconnect = 0;

/** @brief Flag indiquant la fin de partie */
volatile int partieTerminee = 0;


/*
*****************************************************************************************
 *	\noop		D I A L O G U E S   E N R E G I S T R E M E N T   ( N O N   M O D I F I É )
 */

/**
 * @brief Dialogue entre le client et le serveur d'enregistrement
 * 
 * @param params        Paramètres du thread (alloué avec malloc)
 * 
 * @note S'occupe de l'envoi de requêtes et réception de réponses
 */
void dialClt2SrvE(eCltThreadParams_t *params) {

	int 			status;
	rep_t 			response;


	socket_t	 *sockAppel		= params->sockAppel;
	clientInfo_t *infos 		= params->infos;
	clientInfo_t *hosts 		= params->hostBuffer;
	sem_t 		 *semCanClose	= params->semCanClose;
	sem_t 		 *semRequestFin = params->semRequestFin;

	free(params);

	// logMessage("Client: %s, %d, %s, %d\n", DEBUG, infos->name, infos->role, infos->address, infos->port);

	status = enum2status(REQ, CONNECT);
	sendRequest(sockAppel, status, POST, infos, (pFct) clientInfo2str);
	

	rcvResponse(sockAppel, &response);
	if (response.id != enum2status(ACK, CONNECT)) {
		logMessage("[%d] Connexion échouée: %s.\n", DEBUG, response.id, response.data);
		sem_post(semCanClose);
		return;
	} else {
		//logMessage("[%d] Connexion réussie: %s.\n", DEBUG, response.id, response.data);
	}


	while (1) {
		

		if (mustDisconnect) {

			status = enum2status(REQ, CONNECT);
			logMessage("Demande de déconnexion.\n", DEBUG);
			sendRequest(sockAppel, status, DELETE, "", NULL);
			logMessage("Attente d'une réponse.\n", DEBUG);
			rcvResponse(sockAppel, &response);

			if (response.id == enum2status(ACK, CONNECT)) {
			
				logMessage("[%d] Déconnexion: %s\n", DEBUG, response.id, response.data);
				sem_post(semCanClose);
				mustDisconnect = 0;
			
				break;
			
			} else {
			
				logMessage("[%d] Erreur déconnexion: %s\n", DEBUG, response.id, response.data);
				return;
			
			}
			
		}


		if (requestHosts) {

			for (int i = 0; i < MAX_HOSTS_GET; i++) {

				status = enum2status(REQ, CONNECT);
				sendRequest(sockAppel, status, GET, "", NULL);

				rcvResponse(sockAppel, &response);

				if (response.id == enum2status(ACK, CONNECT)) {

					//logMessage("Client reçu.\n", DEBUG);
					str2clientInfo(response.data, &hosts[i]);

				}

				if (response.id == enum2status(ERR, CONNECT)) {

					//logMessage("Plus de clients à recevoir.\n", DEBUG);
					break;

				}


			}

			requestHosts = 0;
			sem_post(semRequestFin);
			

		}

		
	}

}

/**
 * @brief Dialogue entre le serveur d'enregistrement et le client
 * 
 * @param params        Paramètres du thread (alloué avec malloc)
 * 
 * @note S'occupe de l'envoi de réponses et réception de requêtes
 */
void dialSrvE2Clt(eServThreadParams_t *params) {

	int 			running		= 1;
	int 			current	 	= 0;

	int 			id 			= params->id;
	socket_t 		*sockDial 	= params->sockDial;
	clientInfo_t 	*clients 	= params->clientArray;
	clientInfo_t 	*client 	= &clients[id];			// dangereux lorsque serv enr. plein. mais soit.


	int 		clientAmount	= params->clientAmount;


	void (*onDisconnect)(int) 	= params->terminationCallback;
	int  (*canAccept)()			= params->canAccept;


	free(params);
	

	while(running)	// daemon !
	{	
		
		req_t request;		
		rcvRequest(sockDial, &request);
		
		
		switch (request.id) {
			
			case REQ_CONNECT:

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

				if (request.verb == GET) {

					int found = 0;

					for (int i = current; i < clientAmount; i++) {

						if (clients[i].role == HOST && clients[i].status == CONNECTED) {

							status = enum2status(ACK, CONNECT);
							sendResponse(sockDial, status, &clients[i], (pFct) clientInfo2str);

							found   = 1;
							current = i+1;

						}

					}

					if (!found) {
					
						status 	= enum2status(ERR, CONNECT);
						sendResponse(sockDial, status, "", NULL);
						current = 0;
					
					}

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
/*
*****************************************************************************************
 *	\noop		U T I L I T A I R E S   J E U   C L I E N T
 */

/**
 * @brief Traite le placement d'un coéquipier
 * 
 * @param placement     Placement effectué par le coéquipier
 * @param jeu           Partie de jeu en cours
 * @param equipeId      Id de l'équipe
 */
void traiterPlacementCoequipier(Placement *placement, Jeu *jeu, int equipeId) {
	
	Equipe *monEquipe = obtenirMonEquipe(jeu, equipeId);
	
	placer_bateau(&monEquipe->grille, placement->id, placement->longueur, placement->ligne, placement->col, placement->orient);
	
	logMessage("Coéquipier a placé un bateau\n", DEBUG);

}

/**
 * @brief Traite un tir recu de l'adversaire
 * 
 * @param resultat      Résultat du tir adverse
 * @param jeu           Partie de jeu en cours
 * @param equipeId      Id de l'équipe
 */
void traiterTirRecu(Resultat *resultat, Jeu *jeu, int equipeId) {

	Equipe *monEquipe = obtenirMonEquipe(jeu, equipeId);

	if (resultat->touche) {
	
		monEquipe->grille.cases[resultat->ligne][resultat->col] = TOUCHE(resultat->id_coule);
		logMessage("Ennemi vous a touché en (%d,%d) !\n", DEBUG, resultat->ligne, resultat->col);
	
	} else {
	
		monEquipe->grille.cases[resultat->ligne][resultat->col] = 1;
		logMessage("Ennemi a raté en (%d,%d)\n", DEBUG, resultat->ligne, resultat->col);
	
	}

}

/**
 * @brief Traite un signal de tour de placement
 * 
 * @param tour             Tour de jeu recu
 * @param equipeId         Id de l'équipe
 * @param monTourPlacement Indicateur de tour de placement
 * @param semTourPlacement Sémaphore de tour de placement
 */
void traiterTourPlacement(Tour *tour, int equipeId, int *monTourPlacement, sem_t *semTourPlacement) {
	
	if (tour->equipe_id == equipeId) {
	
		*monTourPlacement = 1;
		sem_post(semTourPlacement);
		logMessage("C'est votre tour de placement\n", DEBUG);

		return;

	}

	*monTourPlacement = 0;
	
}
/*
*****************************************************************************************
 *	\noop		D I A L O G U E   C L I E N T   J E U
 */

/**
 * @brief Dialogue entre le client et le serveur de jeu
 * 
 * @param params        Paramètres du thread (alloué avec malloc)
 */
void dialClt2SrvG(gCltThreadParams_t *params) {
	
	rep_t response;

	// Extraction des paramètres

	socket_t 	*sockAppel 			= params->sockAppel;
	Resultat 	*dernierResultat 	= params->dernierResultat;
	Tour 		*tourActuel 		= params->tourActuel;
	Jeu 		*jeu 				= params->jeu;
	sem_t 		*semCanClose 		= params->semCanClose;
	sem_t 		*semPlacementOk 	= params->semPlacementOk;
	sem_t 		*semTirResultat 	= params->semTirResultat;
	sem_t 		*semTourActuel 		= params->semTourActuel;
	sem_t 		*semStartGame 		= params->semStartGame;
	sem_t 		*semTourPlacement 	= params->semTourPlacement;
	int 		*monTourPlacement 	= params->monTourPlacement;
	int 		*equipeId 			= params->equipeId;
	int 		*attendsResultatTir = params->attendsResultatTir;
	
	free(params);

	while (1) {
		
		// Gestion déconnexion
		if (mustDisconnect) {
		
			char buffer[BUFFER_SMALL];
			sprintf(buffer, "%d", *equipeId);
			sendRequest(sockAppel, REQ_CONNECT, DELETE, buffer, NULL);
			sem_post(semCanClose);
			mustDisconnect = 0;
			break;
		
		}

		// Réception message serveur
		rcvResponse(sockAppel, &response);

		// Traitement par code de status
		switch (response.id) {
			
			case ACK_CONNECT:

				logMessage("Connecté au serveur de jeu !\n", DEBUG);
				logMessage("Vous êtes l'équipe %d\n", DEBUG, *equipeId);
			
				break;

			case ACK_PLACE: {
			
				if (strlen(response.data) < 10) {
					// Notre placement validé
					sem_post(semPlacementOk);
			
				} else {
					// Placement coéquipier
					Placement placement;
					str2place(response.data, &placement);
					traiterPlacementCoequipier(&placement, jeu, *equipeId);
			
				}
			
				break;
			}

			case ACK_SHOOT: {
			
				str2resultat(response.data, dernierResultat);

				if (*attendsResultatTir) {
					// Notre tir
					sem_post(semTirResultat);
				} else {
					// Tir reçu
					traiterTirRecu(dernierResultat, jeu, *equipeId);
				}
			
				break;
			
			}

			case ACK_NEXT_TURN: {
			
				str2tour(response.data, tourActuel);

				logMessage(
					"NEXT_TURN reçu: equipe=%d, phase=%d\n"
					, DEBUG
					, tourActuel->equipe_id
					, tourActuel->phase
				);

				if (tourActuel->phase == PHASE_PLACEMENT) {
				
					traiterTourPlacement(tourActuel, *equipeId, monTourPlacement, semTourPlacement);
				
				} else {
					// Phase jeu
					sem_post(semTourActuel);
				
				}
				
				break;
			
			}

			case ACK_START_GAME:
			
				logMessage("Le HOST a lancé la partie !\n", DEBUG);
				sem_post(semStartGame);
				break;

			case ACK_END_GAME: {
			
				int vainqueur;

				sscanf(response.data, "%d", &vainqueur);

				if (vainqueur == *equipeId) {
					logMessage("VICTOIRE !\n", DEBUG);
				} else {
					logMessage("DEFAITE\n", DEBUG);
				}

				partieTerminee = 1;
				break;

			}

			default:

				logMessage("Code réponse non géré: %d\n", WARNING, response.id);
				break;
		
		}
	}
}

/*
*****************************************************************************************
 *	\noop		U T I L I T A I R E S   S E R V E U R   J E U
 */

/**
 * @brief Envoie un message à tous les clients
 * 
 * @param sockets       Tableau des sockets clients
 * @param nb            Nombre de clients
 * @param status        Code de status du message
 * @param data          Données à envoyer
 * @param serializer    Fonction de sérialisation
 */
void envoyerATous(socket_t *sockets, int nb, int status, void *data, pFct serializer) {
	
	for (int i = 0; i < nb; i++) {
	
		sendResponse(&sockets[i], status, data, serializer);
		usleep(DELAY_MESSAGE_US);
	
	}

}

/**
 * @brief Envoie un message à tous les membres d'une équipe
 * 
 * @param sockets       Tableau des sockets clients
 * @param nb            Nombre de clients
 * @param equipeId      Id de l'équipe
 * @param status        Code de status du message
 * @param data          Données à envoyer
 * @param serializer    Fonction de sérialisation
 */
void envoyerAEquipe(socket_t *sockets, int nb, int equipeId, int status, void *data, pFct serializer) {

	for (int i = equipeId; i < nb; i += 2) {
		sendResponse(&sockets[i], status, data, serializer);
		usleep(DELAY_MESSAGE_US);

	}

}

/**
 * @brief Envoie un message aux coéquipiers (pas au joueur lui-même)
 * 
 * @param sockets       Tableau des sockets clients
 * @param nb            Nombre de clients
 * @param equipeId      Id de l'équipe
 * @param numJoueur     Numéro du joueur à exclure
 * @param status        Code de status du message
 * @param data          Données à envoyer
 * @param serializer    Fonction de sérialisation
 */
void envoyerACoequipiers(socket_t *sockets, int nb, int equipeId, int numJoueur, int status, void *data, pFct serializer) {

	for (int i = 0; i < nb; i++) {
		
		int equipeJoueur = i % 2;
		if (equipeJoueur == equipeId && i != numJoueur) {
			sendResponse(&sockets[i], status, data, serializer);
		
		}
	
	}

}

/**
 * @brief Envoie un message à l'équipe adverse
 * 
 * @param sockets       Tableau des sockets clients
 * @param nb            Nombre de clients
 * @param equipeId      Id de l'équipe
 * @param status        Code de status du message
 * @param data          Données à envoyer
 * @param serializer    Fonction de sérialisation
 */
void envoyerAAdversaires(socket_t *sockets, int nb, int equipeId, int status, void *data, pFct serializer) {
	
	for (int i = 0; i < nb; i++) {
	
		int equipeJoueur = i % 2;
		if (equipeJoueur != equipeId) {
			sendResponse(&sockets[i], status, data, serializer);
	
		}
	
	}

}

/**
 * @brief Calcule le prochain joueur pour le placement
 * 
 * @param equipeId      Id de l'équipe actuelle
 * @param nbClients     Nombre de clients connectés
 * 
 * @return Id du prochain joueur
 */
int calculerProchainJoueur(int equipeId, int nbClients) {

	int prochainJoueur = equipeId + 2;

	if (prochainJoueur >= nbClients) {
		prochainJoueur = (equipeId == EQUIPE_A) ? EQUIPE_B : EQUIPE_A;
	}

	return prochainJoueur;
}

/**
 * @brief Envoie NEXT_TURN pour le placement
 * 
 * @param sockets           Tableau des sockets clients
 * @param nb                Nombre de clients
 * @param prochainJoueur    Id du prochain joueur
 */
void envoyerNextTurnPlacement(socket_t *sockets, int nb, int prochainJoueur) {

	Tour tour = {prochainJoueur % 2, 0, PHASE_PLACEMENT};
	envoyerAEquipe(sockets, nb, prochainJoueur % 2, ACK_NEXT_TURN, &tour, (pFct)tour2str);

}

/**
 * @brief Démarre la phase de jeu après placement
 * 
 * @param sockets       Tableau des sockets clients
 * @param nb            Nombre de clients
 */
void demarrerPhaseJeu(socket_t *sockets, int nb) {

	Tour tourJeu = {EQUIPE_A, 0, PHASE_JEU};

	logMessage("Placement terminé. Début de la partie !\n", DEBUG);
	sleep(DELAY_PLACEMENT_END_S);
	
	envoyerATous(sockets, nb, ACK_NEXT_TURN, &tourJeu, (pFct)tour2str);

}

/**
 * @brief Envoie NEXT_TURN pendant la phase de jeu
 * 
 * @param sockets       Tableau des sockets clients
 * @param nb            Nombre de clients
 * @param equipe        Id de l'équipe dont c'est le tour
 */
void envoyerNextTurnJeu(socket_t *sockets, int nb, int equipe) {

	Tour tour = {equipe, 0, PHASE_JEU};
	envoyerATous(sockets, nb, ACK_NEXT_TURN, &tour, (pFct)tour2str);

}

/**
 * @brief Envoie le message de victoire
 * 
 * @param sockets          Tableau des sockets clients
 * @param nb               Nombre de clients
 * @param equipeGagnante   Id de l'équipe gagnante
 * @param jeu              Partie de jeu en cours
 */
void envoyerVictoire(socket_t *sockets, int nb, int equipeGagnante, Jeu *jeu) {

	char vicBuffer[BUFFER_SMALL];

	sprintf(vicBuffer, "%d", equipeGagnante);

	envoyerATous(sockets, nb, ACK_END_GAME, vicBuffer, NULL);
	jeu->fini = 1;

}

/*
*****************************************************************************************
 *	\noop		T R A I T E M E N T S   R E Q U Ê T E S   S E R V E U R
 */

/**
 * @brief Traite une requête de connexion
 * 
 * @param sockDial      Socket du client
 * @param jeu           Partie de jeu en cours
 * @param equipeId      Id de l'équipe
 * @param numeroJoueur  Numéro du joueur
 * @param request       Requête recue
 */
void traiterConnexion(socket_t *sockDial, Jeu *jeu, int equipeId, int numeroJoueur, req_t *request) {
	Joueur joueur;
	str2joueur(request->data, &joueur);

	Equipe *equipe = obtenirMonEquipe(jeu, equipeId);
	ajouter_joueur(equipe, joueur.nom);

	char buffer[BUFFER_SMALL];
	sprintf(buffer, "%d,%d", equipeId, numeroJoueur);

	sendResponse(sockDial, ACK_CONNECT, buffer, NULL);
	logMessage("Joueur %s connecté à l'équipe %d\n", DEBUG, joueur.nom, equipeId);
}

/**
 * @brief Traite une requête de déconnexion
 * 
 * @param sockDial      Socket du client
 * @param running       Flag de boucle du thread
 */
void traiterDeconnexion(socket_t *sockDial, int *running) {

	*running = 0;
	sendResponse(sockDial, ACK_CONNECT, "Déconnexion réussie", NULL);

}

/**
 * @brief Traite une requête de placement
 * 
 * @param sockDial                 Socket du client
 * @param clientsSockets           Tableau des sockets clients
 * @param nbClients                Nombre de clients connectés
 * @param jeu                      Partie de jeu en cours
 * @param equipeId                 Id de l'équipe
 * @param numeroJoueur             Numéro du joueur
 * @param phasePlacementTermine    État du placement par équipe
 * @param request                  Requête recue
 * 
 * @return 1: Succès | 0: Echec
 */
int traiterPlacement(socket_t *sockDial, socket_t *clientsSockets, int nbClients,
                     Jeu *jeu, int equipeId, int numeroJoueur,
                     int *phasePlacementTermine, req_t *request) {

	Placement 	placement;
	int 		prochainJoueur;
	Equipe 		*equipe 		= obtenirMonEquipe(jeu, equipeId);

	str2place(request->data, &placement);
	
	if (!placer_bateau(&equipe->grille, placement.id, placement.longueur,
	                   placement.ligne, placement.col, placement.orient)) {
		
		sendResponse(sockDial, ERR_PLACE, "Position invalide", NULL);
		return 0;
	
	}

	// Confirmer au joueur
	sendResponse(sockDial, ACK_PLACE, "OK", NULL);

	// Broadcaster aux coéquipiers
	envoyerACoequipiers(clientsSockets, nbClients, equipeId, numeroJoueur,
	                   ACK_PLACE, &placement, (pFct)place2str);

	// Gérer le tour suivant
	if (equipe->grille.nb_bateaux < NB_BATEAUX) {
	
		prochainJoueur = calculerProchainJoueur(equipeId, nbClients);
		envoyerNextTurnPlacement(clientsSockets, nbClients, prochainJoueur);
	
	} else {
	
		// Équipe a terminé le placement
		phasePlacementTermine[equipeId] = 1;
		logMessage("Équipe %d a terminé le placement (%d/%d bateaux)\n", DEBUG,
		          equipeId, equipe->grille.nb_bateaux, NB_BATEAUX);

		// Si les deux équipes ont terminé
		if (phasePlacementTermine[EQUIPE_A] && phasePlacementTermine[EQUIPE_B]) {
	
			demarrerPhaseJeu(clientsSockets, nbClients);
	
		}
	
	}

	return 1;
}

/**
 * @brief Traite une requête de tir
 * 
 * @param sockDial         Socket du client
 * @param clientsSockets   Tableau des sockets clients
 * @param nbClients        Nombre de clients connectés
 * @param jeu              Partie de jeu en cours
 * @param request          Requête recue
 */
void traiterTir(socket_t *sockDial, socket_t *clientsSockets, int nbClients,
                Jeu *jeu, req_t *request) {
	Tir tir;
	str2tir(request->data, &tir);


	int 	nextEquipe;

	Equipe 		*attaquant 	= obtenirMonEquipe(jeu, tir.equipe_id);
	Equipe 		*defenseur 	= obtenirMonEquipe(jeu, 1 - tir.equipe_id);

	Resultat 	resultat 	= tirer(&defenseur->grille, attaquant->vue, tir.ligne, tir.col);

	// Envoyer résultat au tireur
	sendResponse(sockDial, ACK_SHOOT, &resultat, (pFct)resultat2str);

	// Broadcaster aux défenseurs
	envoyerAAdversaires(clientsSockets, nbClients, tir.equipe_id,
	                   ACK_SHOOT, &resultat, (pFct)resultat2str);

	usleep(DELAY_BROADCAST_US);

	// Vérifier victoire
	if (victoire(&defenseur->grille)) {

		envoyerVictoire(clientsSockets, nbClients, tir.equipe_id, jeu);
	
	} else {
	
		// Déterminer équipe suivante
		nextEquipe = resultat.touche ? tir.equipe_id : (1 - tir.equipe_id);
		envoyerNextTurnJeu(clientsSockets, nbClients, nextEquipe);
	
	}

}

/*
*****************************************************************************************
 *	\noop		D I A L O G U E   S E R V E U R   J E U
 */

/**
 * @brief Dialogue entre le serveur de jeu et le client
 * 
 * @param params        Paramètres du thread (alloué avec malloc)
 */
void dialSrvG2Clt(gServThreadParams_t *params) {
	
	req_t 			request;

	int 			running 	= 1;

	// Extraction des paramètres

	int 			equipeId 				= params->equipeId;
	int 			numeroJoueur 			= params->numeroJoueur;

	socket_t 		*sockDial 				= params->sockDial;
	Jeu 			*jeu 					= params->jeu;
	socket_t 		*clientsSockets 		= params->clientsSockets;
	
	int 			*nbClientsConnectes 	= params->nbClientsConnectes;
	int 			*phasePlacementTermine 	= params->phasePlacementTermine;
	
	pthread_mutex_t *mutexJeu 				= params->mutexJeu;

	free(params);

	while (running) {

		rcvRequest(sockDial, &request);

		// Traitement par code de requête avec verbe
		switch (request.id) {
		
			case REQ_CONNECT:

				if (request.verb == POST) {
				
					pthread_mutex_lock(mutexJeu);
					traiterConnexion(sockDial, jeu, equipeId, numeroJoueur, &request);
					pthread_mutex_unlock(mutexJeu);
				
				} else if (request.verb == DELETE) {
				
					traiterDeconnexion(sockDial, &running);
				
				}
				
				break;

			case REQ_PLACE:
				
				if (request.verb == POST) {
				
					pthread_mutex_lock(mutexJeu);
					
					traiterPlacement(
						sockDial
						, clientsSockets
						, *nbClientsConnectes
						,jeu
						, equipeId
						, numeroJoueur
						, phasePlacementTermine
						, &request
					);

					pthread_mutex_unlock(mutexJeu);
				
				}

				break;

			case REQ_SHOOT:
				
				if (request.verb == POST) {
				
					pthread_mutex_lock(mutexJeu);
					traiterTir(sockDial, clientsSockets, *nbClientsConnectes,
					          jeu, &request);
					pthread_mutex_unlock(mutexJeu);
				
				}

				break;

			default:
				
				sendResponse(sockDial, enum2status(ERR, getAction(request.id)), "Requête non gérée", NULL);
				
				break;
			
			}
	

	}

	close(sockDial->fd);
	free(sockDial);

}

/**
 * @brief Envoie une requête via un flag et attends une sémaphore
 * 
 * @param reqVar  		Flag de la requête
 * @param semReqAck 	Sémaphore d'attente
 */
void postRequest(int *reqVar, sem_t *semReqAck) {

	*reqVar = 1;
	//logMessage("Requête commencée...\n", DEBUG);
	
	sem_wait(semReqAck);
	
	//logMessage("Requête finie.\n", DEBUG);
	*reqVar = 0;

}