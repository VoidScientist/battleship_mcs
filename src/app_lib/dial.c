/**
 *	\file		dial.c
 *	\brief		Fichier implémentation représentant les dialogues applicatifs
 *	\author		ARCELON Louis
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
#include "../game_logic/include/bataille_navale.h"
#include "../game_logic/include/logic.h"

/*
*****************************************************************************************
 *	\noop		C O N S T A N T E S
 */
// Codes de status
#define REQ_CONNECT		101
#define ACK_CONNECT		201
#define ERR_CONNECT		301

#define REQ_PLACE		105
#define ACK_PLACE		205
#define ERR_PLACE		305

#define REQ_SHOOT		106
#define ACK_SHOOT		206

#define ACK_NEXT_TURN	207
#define ACK_END_GAME	208
#define ACK_START_GAME	209

// Délais
#define DELAY_MESSAGE_US		10000
#define DELAY_BROADCAST_US		50000
#define DELAY_PLACEMENT_END_S	2

// Équipes
#define EQUIPE_A	0
#define EQUIPE_B	1

// Phases
#define PHASE_PLACEMENT	0
#define PHASE_JEU		1

// Tailles buffer
#define BUFFER_SIZE		100
#define BUFFER_SMALL	20

/*
*****************************************************************************************
 *	\noop		V A R I A B L E S   G L O B A L E S
 */
volatile sig_atomic_t mustDisconnect = 0;
int tourPlacementJoueur[2] = {0, 0};
int requestHosts;

/*
*****************************************************************************************
 *	\noop		D I A L O G U E S   E N R E G I S T R E M E N T   ( N O N   M O D I F I É )
 */

void dialClt2SrvE(eCltThreadParams_t *params) {
	int status;
	rep_t response;

	socket_t *sockAppel = params->sockAppel;
	clientInfo_t *infos = params->infos;
	clientInfo_t *hosts = params->hostBuffer;
	sem_t *semCanClose = params->semCanClose;
	sem_t *semRequestFin = params->semRequestFin;

	free(params);

	status = enum2status(REQ, CONNECT);
	sendRequest(sockAppel, status, POST, infos, (pFct)clientInfo2str);

	rcvResponse(sockAppel, &response);
	if (response.id != enum2status(ACK, CONNECT)) {
		logMessage("[%d] Connexion échouée: %s.\n", DEBUG, response.id, response.data);
		sem_post(semCanClose);
		return;
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
					str2clientInfo(response.data, &hosts[i]);
				}

				if (response.id == enum2status(ERR, CONNECT)) {
					break;
				}
			}

			requestHosts = 0;
			sem_post(semRequestFin);
		}
	}
}

void dialSrvE2Clt(eServThreadParams_t *params) {
	int running = 1;
	int current = 0;

	int id = params->id;
	socket_t *sockDial = params->sockDial;
	clientInfo_t *clients = params->clientArray;
	clientInfo_t *client = &clients[id];

	int clientAmount = params->clientAmount;

	void (*onDisconnect)(int) = params->terminationCallback;
	int (*canAccept)() = params->canAccept;

	free(params);

	while (running) {
		req_t request;
		rcvRequest(sockDial, &request);

		switch (request.id) {
		case 101: {
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
						sendResponse(sockDial, status, &clients[i], (pFct)clientInfo2str);
						found = 1;
						current = i + 1;
					}
				}

				if (!found) {
					status = enum2status(ERR, CONNECT);
					sendResponse(sockDial, status, "", NULL);
					current = 0;
				}

				break;
			}

			break;
		}

		default: {
			action_t act = getAction(request.id);
			int status = enum2status(ERR, act);
			sendResponse(sockDial, status, "Code de status non géré", NULL);
			break;
		}
		}
	}

	close(sockDial->fd);
	free(sockDial);
}

/*
*****************************************************************************************
 *	\noop		U T I L I T A I R E S   J E U   C L I E N T
 */

/**
 *	\fn			Equipe* obtenirEquipe(Jeu *jeu, int equipeId)
 *	\brief		Retourne l'équipe correspondant à l'ID
 */
Equipe* obtenirEquipe(Jeu *jeu, int equipeId) {
	return (equipeId == EQUIPE_A) ? &jeu->equipeA : &jeu->equipeB;
}

/**
 *	\fn			void traiterPlacementCoequipier(Placement *placement, Jeu *jeu, int equipeId)
 *	\brief		Traite le placement d'un coéquipier
 */
void traiterPlacementCoequipier(Placement *placement, Jeu *jeu, int equipeId) {
	Equipe *monEquipe = obtenirEquipe(jeu, equipeId);
	placer_bateau(&monEquipe->grille, placement->id, placement->longueur,
	              placement->ligne, placement->col, placement->orient);
	logMessage("Coéquipier a placé un bateau\n", DEBUG);
}

/**
 *	\fn			void traiterTirRecu(Resultat *resultat, Jeu *jeu, int equipeId)
 *	\brief		Traite un tir reçu de l'adversaire
 */
void traiterTirRecu(Resultat *resultat, Jeu *jeu, int equipeId) {
	Equipe *monEquipe = obtenirEquipe(jeu, equipeId);

	if (resultat->touche) {
		monEquipe->grille.cases[resultat->ligne][resultat->col] = TOUCHE(resultat->id_coule);
		logMessage("Ennemi vous a touché en (%d,%d) !\n", DEBUG,
		          resultat->ligne, resultat->col);
	} else {
		monEquipe->grille.cases[resultat->ligne][resultat->col] = 1;
		logMessage("Ennemi a raté en (%d,%d)\n", DEBUG,
		          resultat->ligne, resultat->col);
	}
}

/**
 *	\fn			void traiterTourPlacement(Tour *tour, int equipeId, int *monTourPlacement, sem_t *semTourPlacement)
 *	\brief		Traite un signal de tour de placement
 */
void traiterTourPlacement(Tour *tour, int equipeId, int *monTourPlacement, sem_t *semTourPlacement) {
	if (tour->equipe_id == equipeId) {
		*monTourPlacement = 1;
		sem_post(semTourPlacement);
		logMessage("C'est votre tour de placement\n", DEBUG);
	} else {
		*monTourPlacement = 0;
	}
}

/*
*****************************************************************************************
 *	\noop		D I A L O G U E   C L I E N T   J E U
 */

/**
 *	\fn			void dialClt2SrvG(gCltThreadParams_t *params)
 *	\brief		Dialogue entre le client et le serveur de jeu
 */
void dialClt2SrvG(gCltThreadParams_t *params) {
	// Extraction des paramètres
	socket_t *sockAppel = params->sockAppel;
	int *equipeId = params->equipeId;
	Resultat *dernierResultat = params->dernierResultat;
	Tour *tourActuel = params->tourActuel;
	sem_t *semCanClose = params->semCanClose;
	sem_t *semPlacementOk = params->semPlacementOk;
	sem_t *semTirResultat = params->semTirResultat;
	sem_t *semTourActuel = params->semTourActuel;
	sem_t *semStartGame = params->semStartGame;
	sem_t *semTourPlacement = params->semTourPlacement;
	int *monTourPlacement = params->monTourPlacement;
	Jeu *jeu = params->jeu;
	int *attendsResultatTir = params->attendsResultatTir;
	volatile sig_atomic_t *partieTerminee = params->partieTerminee;

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
		rep_t response;
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
			logMessage("NEXT_TURN reçu: equipe=%d, phase=%d\n", DEBUG,
			          tourActuel->equipe_id, tourActuel->phase);

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

			*partieTerminee = 1;
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
 *	\fn			void envoyerATous(socket_t *sockets, int nb, int status, void *data, pFct serializer)
 *	\brief		Envoie un message à tous les clients
 */
void envoyerATous(socket_t *sockets, int nb, int status, void *data, pFct serializer) {
	for (int i = 0; i < nb; i++) {
		sendResponse(&sockets[i], status, data, serializer);
		usleep(DELAY_MESSAGE_US);
	}
}

/**
 *	\fn			void envoyerAEquipe(socket_t *sockets, int nb, int equipeId, int status, void *data, pFct serializer)
 *	\brief		Envoie un message à tous les membres d'une équipe
 */
void envoyerAEquipe(socket_t *sockets, int nb, int equipeId, int status, void *data, pFct serializer) {
	for (int i = equipeId; i < nb; i += 2) {
		sendResponse(&sockets[i], status, data, serializer);
		usleep(DELAY_MESSAGE_US);
	}
}

/**
 *	\fn			void envoyerACoequipiers(socket_t *sockets, int nb, int equipeId, int numJoueur, int status, void *data, pFct serializer)
 *	\brief		Envoie un message aux coéquipiers (pas au joueur lui-même)
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
 *	\fn			void envoyerAAdversaires(socket_t *sockets, int nb, int equipeId, int status, void *data, pFct serializer)
 *	\brief		Envoie un message à l'équipe adverse
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
 *	\fn			int calculerProchainJoueur(int equipeId, int nbClients)
 *	\brief		Calcule le prochain joueur pour le placement
 */
int calculerProchainJoueur(int equipeId, int nbClients) {
	int prochainJoueur = equipeId + 2;

	if (prochainJoueur >= nbClients) {
		prochainJoueur = (equipeId == EQUIPE_A) ? EQUIPE_B : EQUIPE_A;
	}

	return prochainJoueur;
}

/**
 *	\fn			void envoyerNextTurnPlacement(socket_t *sockets, int nb, int prochainJoueur)
 *	\brief		Envoie NEXT_TURN pour le placement
 */
void envoyerNextTurnPlacement(socket_t *sockets, int nb, int prochainJoueur) {
	Tour tour = {prochainJoueur % 2, 0, PHASE_PLACEMENT};
	envoyerAEquipe(sockets, nb, prochainJoueur % 2, ACK_NEXT_TURN, &tour, (pFct)tour2str);
}

/**
 *	\fn			void demarrerPhaseJeu(socket_t *sockets, int nb)
 *	\brief		Démarre la phase de jeu après placement
 */
void demarrerPhaseJeu(socket_t *sockets, int nb) {
	logMessage("Placement terminé. Début de la partie !\n", DEBUG);
	sleep(DELAY_PLACEMENT_END_S);

	Tour tourJeu = {EQUIPE_A, 0, PHASE_JEU};
	envoyerATous(sockets, nb, ACK_NEXT_TURN, &tourJeu, (pFct)tour2str);
}

/**
 *	\fn			void envoyerNextTurnJeu(socket_t *sockets, int nb, int equipe)
 *	\brief		Envoie NEXT_TURN pendant la phase de jeu
 */
void envoyerNextTurnJeu(socket_t *sockets, int nb, int equipe) {
	Tour tour = {equipe, 0, PHASE_JEU};
	envoyerATous(sockets, nb, ACK_NEXT_TURN, &tour, (pFct)tour2str);
}

/**
 *	\fn			void envoyerVictoire(socket_t *sockets, int nb, int equipeGagnante, Jeu *jeu)
 *	\brief		Envoie le message de victoire
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
 *	\fn			void traiterConnexion(socket_t *sockDial, Jeu *jeu, int equipeId, int numeroJoueur, req_t *request)
 *	\brief		Traite une requête de connexion
 */
void traiterConnexion(socket_t *sockDial, Jeu *jeu, int equipeId, int numeroJoueur, req_t *request) {
	Joueur joueur;
	str2joueur(request->data, &joueur);

	Equipe *equipe = obtenirEquipe(jeu, equipeId);
	ajouter_joueur(equipe, joueur.nom);

	char buffer[BUFFER_SMALL];
	sprintf(buffer, "%d,%d", equipeId, numeroJoueur);

	sendResponse(sockDial, ACK_CONNECT, buffer, NULL);
	logMessage("Joueur %s connecté à l'équipe %d\n", DEBUG, joueur.nom, equipeId);
}

/**
 *	\fn			void traiterDeconnexion(socket_t *sockDial, int *running)
 *	\brief		Traite une requête de déconnexion
 */
void traiterDeconnexion(socket_t *sockDial, int *running) {
	*running = 0;
	sendResponse(sockDial, ACK_CONNECT, "Déconnexion réussie", NULL);
}

/**
 *	\fn			int traiterPlacement(socket_t *sockDial, socket_t *clientsSockets, int nbClients, 
 *	                                  Jeu *jeu, int equipeId, int numeroJoueur, 
 *	                                  int *phasePlacementTermine, req_t *request)
 *	\brief		Traite une requête de placement
 */
int traiterPlacement(socket_t *sockDial, socket_t *clientsSockets, int nbClients,
                     Jeu *jeu, int equipeId, int numeroJoueur,
                     int *phasePlacementTermine, req_t *request) {
	Placement placement;
	str2place(request->data, &placement);

	Equipe *equipe = obtenirEquipe(jeu, equipeId);

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
		int prochainJoueur = calculerProchainJoueur(equipeId, nbClients);
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
 *	\fn			void traiterTir(socket_t *sockDial, socket_t *clientsSockets, int nbClients,
 *	                             Jeu *jeu, req_t *request)
 *	\brief		Traite une requête de tir
 */
void traiterTir(socket_t *sockDial, socket_t *clientsSockets, int nbClients,
                Jeu *jeu, req_t *request) {
	Tir tir;
	str2tir(request->data, &tir);

	Equipe *attaquant = obtenirEquipe(jeu, tir.equipe_id);
	Equipe *defenseur = obtenirEquipe(jeu, 1 - tir.equipe_id);

	Resultat resultat = tirer(&defenseur->grille, attaquant->vue, tir.ligne, tir.col);

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
		int nextEquipe = resultat.touche ? tir.equipe_id : (1 - tir.equipe_id);
		envoyerNextTurnJeu(clientsSockets, nbClients, nextEquipe);
	}
}

/*
*****************************************************************************************
 *	\noop		D I A L O G U E   S E R V E U R   J E U
 */

/**
 *	\fn			void dialSrvG2Clt(gServThreadParams_t *params)
 *	\brief		Dialogue entre le serveur de jeu et le client
 */
void dialSrvG2Clt(gServThreadParams_t *params) {
	// Extraction des paramètres
	int equipeId = params->equipeId;
	int numeroJoueur = params->numeroJoueur;
	socket_t *sockDial = params->sockDial;
	Jeu *jeu = params->jeu;
	socket_t *clientsSockets = params->clientsSockets;
	int *nbClientsConnectes = params->nbClientsConnectes;
	int *phasePlacementTermine = params->phasePlacementTermine;
	pthread_mutex_t *mutexJeu = params->mutexJeu;

	free(params);

	int running = 1;

	while (running) {
		req_t request;
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
				traiterPlacement(sockDial, clientsSockets, *nbClientsConnectes,
				                jeu, equipeId, numeroJoueur,
				                phasePlacementTermine, &request);
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
			sendResponse(sockDial, enum2status(ERR, GAME), "Requête non gérée", NULL);
			break;
		}
	}

	close(sockDial->fd);
	free(sockDial);
}

/**
 *	\fn			void postRequest(int *reqVar, sem_t *semReqAck)
 *	\brief		Envoie une requête via un flag et attend une sémaphore
 */
void postRequest(int *reqVar, sem_t *semReqAck) {
	*reqVar = 1;
	sem_wait(semReqAck);
	*reqVar = 0;
}
