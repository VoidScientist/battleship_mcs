/**
 *	\file		dial.c
 *	\brief		Fichier implémentation représentant les dialogues applicatifs
 *	\author		ARCELON Louis
 *	\date		28 janvier 2026
 *	\version	1.0
 */
#include <semaphore.h>
#include <pthread.h>
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
// Suivi du joueur courant pour le placement dans chaque équipe
int tourPlacementJoueur[2] = {0, 0};
int requestHosts;


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
 * \fn 			dialSrv2Clt()
 * \brief       fonction s'occupant du dialogue entre le serveur d'enregistrement et le client
 * 
 * \param		sockDial		structure socket_t contenant les informations
 * 								sur la socket de dialogue avec le client
 * 
 * \note		s'occupe donc de l'envoi de réponses et réception de réponses
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

/**
 * \fn 			dialClt2SrvG()
 * \brief       Dialogue entre le client et le serveur de jeu
 * 
 * \param		params		Paramètres du thread client de jeu
 */
void dialClt2SrvG(gCltThreadParams_t *params) {

    // Déstructuration des paramètres
    socket_t    *sockAppel         = params->sockAppel;
    int         *equipeId          = params->equipeId;
    Resultat    *dernierResultat   = params->dernierResultat;
    Tour        *tourActuel        = params->tourActuel;
    sem_t       *semCanClose       = params->semCanClose;
    sem_t       *semPlacementOk    = params->semPlacementOk;
    sem_t       *semTirResultat    = params->semTirResultat;
    sem_t       *semTourActuel     = params->semTourActuel;
    sem_t       *semStartGame      = params->semStartGame;
    sem_t       *semTourPlacement  = params->semTourPlacement;
    int         *monTourPlacement  = params->monTourPlacement;
    Jeu         *jeu               = params->jeu;
    int         *attendsResultatTir = params->attendsResultatTir;
    volatile sig_atomic_t *partieTerminee = params->partieTerminee;

    free(params);

    while (1) {

        // Gestion de la déconnexion
        if (mustDisconnect) {
            char buffer[10];
            sprintf(buffer, "%d", *equipeId);
            int status = enum2status(REQ, CONNECT);
            sendRequest(sockAppel, status, DELETE, buffer, NULL);

            sem_post(semCanClose);
            mustDisconnect = 0;
            break;
        }

        // Réception d'un message du serveur
		rep_t response;
		rcvResponse(sockAppel, &response);

		int range  = getStatusRange(response.id);
		int action = getAction(response.id);

		logMessage("[DEBUG CLIENT] Message reçu: code=%d, range=%d, action=%d, data='%s'\n", 
				   DEBUG, response.id, range, action, response.data);

        // Connexion initiale
        if (range == ACK && action == CONNECT) {
            logMessage("Connecté au serveur de jeu !\n", DEBUG);
            logMessage("Vous êtes l'équipe %d\n", DEBUG, *equipeId);
            logMessage("[DEBUG] Socket FD utilisée: %d\n", DEBUG, sockAppel->fd);
        }
        // Placement d'un bateau par un coéquipier
        else if (range == ACK && action == PLACE) {
            if (strlen(response.data) < 10) {
                // Notre placement a été validé
                sem_post(semPlacementOk);
            } else {
                // Placement d'un coéquipier
                Placement placement;
                str2place(response.data, &placement);

                Equipe *monEquipe = (*equipeId == 0) ? &jeu->equipeA : &jeu->equipeB;

                placer_bateau(&monEquipe->grille,
                              placement.id, placement.longueur,
                              placement.ligne, placement.col,
                              placement.orient);

                logMessage("Coéquipier a placé un bateau\n", DEBUG);
            }
        }
        // Résultat d'un tir
		else if (range == ACK && action == SHOOT) {
			str2resultat(response.data, dernierResultat);
			
			if (*attendsResultatTir) {
				// C'est notre tir
				sem_post(semTirResultat);
			} else {
				// C'est un tir reçu
				Equipe *monEquipe = (*equipeId == 0) ? &jeu->equipeA : &jeu->equipeB;
				
				if (dernierResultat->touche) {
				    monEquipe->grille.cases[dernierResultat->ligne][dernierResultat->col] = 
				        TOUCHE(dernierResultat->id_coule);
				    logMessage("Ennemi vous a touché en (%d,%d) !\n", DEBUG, 
				              dernierResultat->ligne, dernierResultat->col);
				} else {
				    monEquipe->grille.cases[dernierResultat->ligne][dernierResultat->col] = 1;
				    logMessage("Ennemi a raté en (%d,%d)\n", DEBUG, 
				              dernierResultat->ligne, dernierResultat->col);
				}
			}
		}
		// Passage au tour suivant
		else if (range == ACK && action == NEXT_TURN) {
			str2tour(response.data, tourActuel);
			
			logMessage("[DEBUG] NEXT_TURN reçu: equipe=%d, phase=%d\n", DEBUG, 
				       tourActuel->equipe_id, tourActuel->phase);
			
			if (tourActuel->phase == 0) {
				// Phase placement
				if (tourActuel->equipe_id == *equipeId) {
				    *monTourPlacement = 1;
				    sem_post(semTourPlacement);
				    logMessage("C'est votre tour de placement\n", DEBUG);
				} else {
				    *monTourPlacement = 0;
				}
			} else {
				// Phase jeu
				logMessage("[DEBUG] Phase jeu détectée, sem_post(semTourActuel)\n", DEBUG);
				sem_post(semTourActuel);
			}
		}
		// Début de la partie
		else if (range == ACK && action == START_GAME) {
			logMessage("Le HOST a lancé la partie !\n", DEBUG);
			sem_post(semStartGame);
		}
        // Fin de la partie
        else if (range == ACK && action == END_GAME) {
            int vainqueur;
            sscanf(response.data, "%d", &vainqueur);

            if (vainqueur == *equipeId) {
                logMessage("VICTOIRE !\n", DEBUG);
            } else {
                logMessage("DEFAITE\n", DEBUG);
            }

            *partieTerminee = 1;
        }
        // Message inconnu
        else {
            logMessage("Code réponse non géré: %d\n", WARNING, response.id);
        }
    }
}

/**
 * \fn 			dialSrvG2Clt()
 * \brief       Dialogue entre le serveur de jeu et le client
 * 
 * \param		params		Paramètres du thread serveur de jeu
 */
void dialSrvG2Clt(gServThreadParams_t *params) {
	
	int 			equipeId 				= params->equipeId;
	int 			numeroJoueur			= params->numeroJoueur;
	socket_t 		*sockDial 				= params->sockDial;
	Jeu 			*jeu 					= params->jeu;
	socket_t 		*clientsSockets 		= params->clientsSockets;
	int 			*nbClientsConnectes 	= params->nbClientsConnectes;
	int 			*phasePlacementTermine 	= params->phasePlacementTermine;
	pthread_mutex_t *mutexJeu 				= params->mutexJeu;
	
	free(params);
	
	int running = 1;
	
	while(running) {
		
		req_t request;
		rcvRequest(sockDial, &request);
		
		int range = getStatusRange(request.id);
		int action = getAction(request.id);
		int status;
		
		if (range == REQ && action == CONNECT && request.verb == POST) {
			
			Joueur joueur;
			str2joueur(request.data, &joueur);
			
			pthread_mutex_lock(mutexJeu);
			Equipe *equipe = equipeId == 0 ? &jeu->equipeA : &jeu->equipeB;
			ajouter_joueur(equipe, joueur.nom);
			pthread_mutex_unlock(mutexJeu);
			
			char buffer[20];
			sprintf(buffer, "%d,%d", equipeId, numeroJoueur);

			status = enum2status(ACK, CONNECT);
			sendResponse(sockDial, status, buffer, NULL);
			
			logMessage("Joueur %s connecté à l'équipe %d\n", DEBUG, joueur.nom, equipeId);
		}
		
		else if (range == REQ && action == CONNECT && request.verb == DELETE) {
			
			running = 0;
			
			status = enum2status(ACK, CONNECT);
			sendResponse(sockDial, status, "Déconnexion réussie", NULL);
		}
		
		else if (range == REQ && action == PLACE && request.verb == POST) {
	
			Placement placement;
			str2place(request.data, &placement);
			
			pthread_mutex_lock(mutexJeu);
			Equipe *equipe = equipeId == 0 ? &jeu->equipeA : &jeu->equipeB;
			int ok = placer_bateau(&equipe->grille, placement.id, placement.longueur,
				                   placement.ligne, placement.col, placement.orient);
			
			if (ok) {
				status = enum2status(ACK, PLACE);
				sendResponse(sockDial, status, "OK", NULL);
				
				// BROADCASTER LE PLACEMENT AUX COÉQUIPIERS
				char placementBuffer[100];
				place2str(&placement, placementBuffer);
				
				for (int i = 0; i < *nbClientsConnectes; i++) {
					int equipeJoueur = i % 2;
					if (equipeJoueur == equipeId && i != numeroJoueur) {
						sendResponse(&clientsSockets[i], status, placementBuffer, NULL);
					}
				}
				
				// Passer au joueur suivant (ordre: 0,2,4... puis 1,3,5...)
				int prochainJoueur = equipeId + 2;  // Même équipe
				
				if (prochainJoueur >= *nbClientsConnectes) {
					// Plus de joueur dans cette équipe, passer à l'autre équipe
					if (equipeId == 0) {
						prochainJoueur = 1;  // Premier joueur équipe 1
					} else {
						prochainJoueur = 0;  // Retour équipe 0 (ne devrait pas arriver)
					}
				}
				
				// Envoyer NEXT_TURN seulement si cette équipe N'A PAS terminé le placement
				if (equipe->grille.nb_bateaux < NB_BATEAUX) {
					Tour tour = {prochainJoueur % 2, 0, 0};  // phase 0 = placement
					char tourBuffer[100];
					tour2str(&tour, tourBuffer);

					int tourStatus = enum2status(ACK, NEXT_TURN);
					for (int i = prochainJoueur % 2; i < *nbClientsConnectes; i += 2) {
						sendResponse(&clientsSockets[i], tourStatus, tourBuffer, NULL);
					}
				}
				
				// Vérifier si cette équipe a terminé le placement
				if (equipe->grille.nb_bateaux >= NB_BATEAUX) {
					phasePlacementTermine[equipeId] = 1;
					
					logMessage("Équipe %d a terminé le placement (%d/%d bateaux)\n", DEBUG, 
						      equipeId, equipe->grille.nb_bateaux, NB_BATEAUX);
					
					// Si LES DEUX équipes ont terminé
					if (phasePlacementTermine[0] && phasePlacementTermine[1]) {
						logMessage("Placement terminé. Début de la partie !\n", DEBUG);
						sleep(2);
						
						// Envoyer le premier tour de JEU (phase = 1)
						Tour tourJeu = {0, 0, 1};  // Équipe 0 commence, phase jeu
						char tourJeuBuffer[100];
						tour2str(&tourJeu, tourJeuBuffer);
						
						int tourJeuStatus = enum2status(ACK, NEXT_TURN);
						for(int i = 0; i < *nbClientsConnectes; i++) {
							sendResponse(&clientsSockets[i], tourJeuStatus, tourJeuBuffer, NULL);
						}
					}
				}
				
				pthread_mutex_unlock(mutexJeu);
			} else {
				pthread_mutex_unlock(mutexJeu);
				status = enum2status(ERR, PLACE);
				sendResponse(sockDial, status, "Position invalide", NULL);
			}
		}
		
		else if (range == REQ && action == SHOOT && request.verb == POST) {
			
			Tir tir;
			str2tir(request.data, &tir);
			
			pthread_mutex_lock(mutexJeu);
			
			Equipe *attaquant = tir.equipe_id == 0 ? &jeu->equipeA : &jeu->equipeB;
			Equipe *defenseur = tir.equipe_id == 0 ? &jeu->equipeB : &jeu->equipeA;
			
			Resultat resultat = tirer(&defenseur->grille, attaquant->vue, tir.ligne, tir.col);
			
			char resBuffer[100];
			resultat2str(&resultat, resBuffer);
			
			status = enum2status(ACK, SHOOT);
			sendResponse(sockDial, status, resBuffer, NULL);

			// BROADCASTER aux défenseurs
			for(int i = 0; i < *nbClientsConnectes; i++) {
				int equipeJoueur = i % 2;
				if (equipeJoueur != tir.equipe_id) {  // Équipe adverse
					sendResponse(&clientsSockets[i], status, resBuffer, NULL);
				}
			}
			
			usleep(50000);
			
			if (victoire(&defenseur->grille)) {
				char vicBuffer[10];
				sprintf(vicBuffer, "%d", tir.equipe_id);
				
				int endStatus = enum2status(ACK, END_GAME);
				for(int i = 0; i < *nbClientsConnectes; i++) {
					sendResponse(&clientsSockets[i], endStatus, vicBuffer, NULL);
				}
				jeu->fini = 1;
			 } else {
				int nextEquipe = resultat.touche ? tir.equipe_id : (1 - tir.equipe_id);
				
				logMessage("[DEBUG SERVEUR] Tir de equipe %d, touche=%d, nextEquipe=%d\n", 
						   DEBUG, tir.equipe_id, resultat.touche, nextEquipe);  // ← AJOUTER
				
				Tour tour = {nextEquipe, 0, 1};
				char tourBuffer[100];
				tour2str(&tour, tourBuffer);
				
				int tourStatus = enum2status(ACK, NEXT_TURN);
				
				logMessage("[DEBUG SERVEUR] Envoi NEXT_TURN equipe %d à %d clients\n", 
						   DEBUG, nextEquipe, *nbClientsConnectes);  // ← AJOUTER
				
				for(int i = 0; i < *nbClientsConnectes; i++) {
					logMessage("[DEBUG SERVEUR] -> Client %d\n", DEBUG, i);  // ← AJOUTER
					sendResponse(&clientsSockets[i], tourStatus, tourBuffer, NULL);
					usleep(10000);  // ← 10ms entre chaque envoi
				}
			}
			
			pthread_mutex_unlock(mutexJeu);
		}
		
		else {
			status = enum2status(ERR, GAME);
			sendResponse(sockDial, status, "Requête non gérée", NULL);
		}
	}
	
	close(sockDial->fd);
	free(sockDial);
}


/**
 * \brief      Envoie une requête via un flag et attends une sémaphore.
 *
 * \param      reqVar     Flag de la requête
 * \param      semReqAck  Sémaphore d'attente
 */
void postRequest(int *reqVar, sem_t *semReqAck) {

	*reqVar = 1;
	//logMessage("Requête commencée...\n", DEBUG);
	
	sem_wait(semReqAck);
	
	//logMessage("Requête finie.\n", DEBUG);
	*reqVar = 0;

}
