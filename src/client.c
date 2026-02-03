/**
 *	\file		client.c
 *	\brief		Code du client pour le jeu
 *	\author		MARTEL Mathieu / ARCELON Louis
 *	\version	1.0
 */

/*
*****************************************************************************************
 *	\noop		I N C L U D E S 
 */
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include "bataille_navale.h"
#include "affichage.h"
#include "logic.h"
#include "structSerial.h"

#include <dial.h>
#include <datastructs.h>
#include <repReq.h>
#include <protocol.h>
#include <session.h>
#include <interface.h>

/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
#define IP_SERV_ENR		"0.0.0.0"
#define PORT_SERV_ENR	50000
#define PORT_JEU		50001
#define MAX_JOUEURS     10

/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   M A C R O S
 */
#define CHECK(sts, msg) if ((sts)==-1) {perror(msg); exit(-1);}

/*
*****************************************************************************************
 *	\noop		D E C L A R A T I O N   DES   V A R I A B L E S    G L O B A L E S
 */
char 			*progName;

// Serveur d'enregistrement
socket_t 		sockEnregistrement;
clientInfo_t 	self;
/**
 * @brief       liste d'hôtes maintenue par le client 
 */
clientInfo_t	hosts[MAX_HOSTS_GET];
sem_t			semCanClose;
sem_t 			semRequestFin;
pthread_t 		threadEnregistrement;

// Serveur de jeu (pour HOST)
Jeu				jeuServeur;
socket_t		clientsSockets[MAX_JOUEURS];
int				nbClientsConnectes = 0;
int				phasePlacementTermine[2] = {0, 0};
pthread_mutex_t	mutexJeu = PTHREAD_MUTEX_INITIALIZER;
socket_t 		sockEcouteJeu;
pthread_t 		threadEcouteJeu;

// Client de jeu
socket_t 		sockJeu;
pthread_t 		threadDialJeu;
Jeu				jeu;
int 			monEquipeId;
int 			monIndexJoueur;
Resultat 		dernierResultat;
Tour			tourActuel;
volatile sig_atomic_t partieTerminee = 0;
sem_t 			semPlacementOk;
sem_t 			semTirResultat;
sem_t			semTourActuel;
sem_t			semStartGame;
int				monTourPlacement;
sem_t			semTourPlacement;
int             attendsResultatTir = 0;

/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

/**
 * @brief      fonction de gestion des signaux
 *
 * @param[in]  code  code du signal
 */
void onSignal(int code) {
	mustDisconnect = code == SIGINT;
}
/**
 * @brief      fonction à appeler pour fermer le client proprement
 */
void onExit() {
	int result;
	
	mustDisconnect = 1;
	
	do {
		result = sem_wait(&semCanClose);
	} while (result == -1 && errno == EINTR);
	
	CHECK(shutdown(sockEnregistrement.fd, SHUT_WR),"-- PB shutdown enregistrement --");
	
	if (sockJeu.fd > 0) {
		CHECK(shutdown(sockJeu.fd, SHUT_WR),"-- PB shutdown jeu --");
	}
	
	exit(EXIT_SUCCESS);
}
/**
 * @brief      initialisation du client (sémaphores, signaux etc...)
 */
void initClient() {
	struct sigaction sa;
	CHECK(sigemptyset(&sa.sa_mask), "sigemptyset()");
	sa.sa_handler 	= onSignal;
	sa.sa_flags 	= 0;
	CHECK(sigaction(SIGINT, &sa, NULL), "sigaction()");
	
	CHECK(sem_init(&semCanClose, 0, 0), "sem_init()");
	CHECK(sem_init(&semRequestFin, 0, 0), "sem_init()");
	CHECK(sem_init(&semPlacementOk, 0, 0), "sem_init()");
	CHECK(sem_init(&semTirResultat, 0, 0), "sem_init()");
	CHECK(sem_init(&semTourActuel, 0, 0), "sem_init()");
	CHECK(sem_init(&semStartGame, 0, 0), "sem_init()");
	CHECK(sem_init(&semTourPlacement, 0, 0), "sem_init()");
	
	monTourPlacement = 0;
	monIndexJoueur = -1;
	
	for (int i = 0; i < MAX_HOSTS_GET; i++) {
		createClientInfo(&hosts[i], "", 0, "", 0);
		hosts[i].status = DISCONNECTED;
	}
}

/**
 *	\fn			void envoyerPlacement(Placement *placement)
 *	\brief		Envoie un placement de bateau au serveur
 */
void envoyerPlacement(Placement *placement) {
	char buffer[100];
	place2str(placement, buffer);
	
	int status = enum2status(REQ, PLACE);
	sendRequest(&sockJeu, status, POST, buffer, NULL);
	
	sem_wait(&semPlacementOk);
}

/**
 *	\fn			Resultat envoyerTir(int ligne, int col)
 *	\brief		Envoie un tir au serveur et attend le résultat
 */
Resultat envoyerTir(int ligne, int col) {
    char buffer[100];
    Tir tir;
    tir.ligne = ligne;
    tir.col = col;
    tir.equipe_id = monEquipeId;
    
    tir2str(&tir, buffer);
    
    attendsResultatTir = 1;
    
    int status = enum2status(REQ, SHOOT);
    sendRequest(&sockJeu, status, POST, buffer, NULL);
    
    sem_wait(&semTirResultat);
    
    attendsResultatTir = 0;
    
    return dernierResultat;
}

/**
 *	\fn			void placerEquipeReseau(Equipe *equipe)
 *	\brief		Phase de placement avec envoi au serveur
 */
void placerEquipeReseau(Equipe *equipe) {
	int bateaux_ids[] = {2, 3, 4, 5, 6};
	int bateaux_longueurs[] = {5, 4, 3, 3, 2};
	
	for(int i = 0; i < NB_BATEAUX; i++) {
		
		printf("En attente de votre tour de placement...\n");
		sem_wait(&semTourPlacement);
		
		int ok = 0;
		while(!ok) {
			clear_screen();
			printf("\n=== Placement %s ===\n", equipe->nom);
			printf("\n=== C'EST VOTRE TOUR ! ===\n");
			printf("\nBateau %d/%d (longueur %d)\n", i+1, NB_BATEAUX, bateaux_longueurs[i]);
			afficher_equipe(equipe);
			
			int ligne, col;
			char orient_char;
			lire_bateau(&ligne, &col, &orient_char);
			
			Orientation orient = (orient_char == 'H') ? HORIZONTAL : VERTICAL;
			
			if(placer_bateau(&equipe->grille, bateaux_ids[i], bateaux_longueurs[i], ligne, col, orient)) {
				Placement placement;
				placement.id = bateaux_ids[i];
				placement.longueur = bateaux_longueurs[i];
				placement.ligne = ligne;
				placement.col = col;
				placement.orient = orient;
				
				envoyerPlacement(&placement);
				
				printf("Bateau place avec succes !\n");
				
				monTourPlacement = 0;
				
				ok = 1;
			} else {
				printf("Position invalide\n");
				printf("Appuyez sur Entree...");
				getchar();
			}
		}
	}
	
	clear_screen();
	printf("\n=== Placement termine ===\n");
	afficher_equipe(equipe);
	printf("\nEn attente des autres joueurs...\n");
}

/**
 *	\fn			void jouerReseau(Jeu *jeu)
 *	\brief		Boucle de jeu avec communication réseau
 */
void jouerReseau(Jeu *jeu) {
    clear_screen();
    printf("\n=== Debut de la partie ===\n");
    
    while(!partieTerminee) {
        
        sem_wait(&semTourActuel);  // Attendre un tour
        
        if (partieTerminee) break;
        
        // Tant que ce n'est pas notre tour, attendre le PROCHAIN signal
        while (tourActuel.equipe_id != monEquipeId && !partieTerminee) {
            printf("\nEn attente du tour adverse...\n");
            sem_wait(&semTourActuel);  // Attendre le PROCHAIN tour
        }
        
        if (partieTerminee) break;
        
        // Vérifier si on est le joueur principal
        int indexPremierJoueur = monEquipeId;
        
        if (monIndexJoueur != indexPremierJoueur) {
            printf("\nC'est au tour de votre coéquipier principal...\n");
            continue;
        }
        
        // C'EST NOTRE TOUR !
        clear_screen();
        Equipe *monEquipe = monEquipeId == 0 ? &jeu->equipeA : &jeu->equipeB;
        
        printf("\n--- C'est votre tour ! ---\n");
        afficher_equipe(monEquipe);
        afficher_vue(monEquipe);
        
        int ligne, col;
        lire_coords(&ligne, &col);
        
        Resultat resultat = envoyerTir(ligne, col);
        
        monEquipe->vue[resultat.ligne][resultat.col] = 
            resultat.touche ? TOUCHE(resultat.id_coule) : 1;
            
        clear_screen();
        afficher_equipe(monEquipe);
        afficher_vue(monEquipe);
        
        if (resultat.touche) {
            if (resultat.coule) {
                printf("\nTouché-Coulé !\n");
            } else {
                printf("\nTouché !\n");
            }
        } else {
            printf("\nLoupé.\n");
        }
        
        printf("Appuyez sur Entree...");
        getchar();
    }
}

/**
 *	\fn			void *threadServeurJeu(void *params)
 *	\brief		Thread d'écoute pour le serveur de jeu (MODE HOST)
 */
void *threadServeurJeu(void *params) {
	
	init_jeu(&jeuServeur);
	
	printf("Serveur de jeu en ecoute sur le port %d\n", PORT_JEU);
	
	while (nbClientsConnectes < MAX_JOUEURS) {
		socket_t *sockDial = malloc(sizeof(socket_t));
		*sockDial = accepterClt(sockEcouteJeu);
		
		int equipeId;
		int numeroJoueur;
		pthread_mutex_lock(&mutexJeu);
		clientsSockets[nbClientsConnectes] = *sockDial;
		equipeId = nbClientsConnectes % 2;
		numeroJoueur = nbClientsConnectes;
		nbClientsConnectes++;
		pthread_mutex_unlock(&mutexJeu);
		
		printf("Joueur %d connecte (equipe %d)\n", numeroJoueur + 1, equipeId);
		
		gServThreadParams_t *params = malloc(sizeof(gServThreadParams_t));
		params->equipeId 				= equipeId;
		params->numeroJoueur			= numeroJoueur;
		params->sockDial 				= sockDial;
		params->jeu 					= &jeuServeur;
		params->clientsSockets 			= clientsSockets;
		params->nbClientsConnectes 		= &nbClientsConnectes;
		params->phasePlacementTermine 	= phasePlacementTermine;
		params->mutexJeu 				= &mutexJeu;
		
		pthread_t thread;
		pthread_create(&thread, 0, (void*)(void*) dialSrvG2Clt, params);
		pthread_detach(thread);
	}
	
	printf("%d joueurs connectes. La partie peut commencer !\n", MAX_JOUEURS);
	
	return NULL;
}

/**
 *	\fn			void lancerClientJeu(char *ip, int port)
 *	\brief		Connecte le client au serveur de jeu et lance la partie
 */
void lancerClientJeu(char *ip, int port) {
	
	init_jeu(&jeu);
	
	printf("\nConnexion au serveur de jeu %s:%d...\n", ip, port);
	sockJeu = connecterClt2Srv(ip, port);
	
	Joueur joueur;
	joueur.id = 0;
	strcpy(joueur.nom, self.name);
	
	char buffer[100];
	joueur2str(&joueur, buffer);
	
	int status = enum2status(REQ, CONNECT);
	sendRequest(&sockJeu, status, POST, buffer, NULL);
	
	rep_t response;
	rcvResponse(&sockJeu, &response);
	
	if (getStatusRange(response.id) == ACK && getAction(response.id) == CONNECT) {
		// Format de réponse: "equipeId,indexJoueur"
		char *token = strtok(response.data, ",");
		if (token) {
			monEquipeId = atoi(token);
			token = strtok(NULL, ",");
			if (token) {
				monIndexJoueur = atoi(token);
			}
		}
		printf("Assigne a l'equipe %d (joueur %d)\n", monEquipeId, monIndexJoueur);
	} else {
		printf("Erreur de connexion au serveur de jeu\n");
		return;
	}
	
	Equipe *monEquipe = monEquipeId == 0 ? &jeu.equipeA : &jeu.equipeB;
	ajouter_joueur(monEquipe, self.name);
	
	gCltThreadParams_t *params = malloc(sizeof(gCltThreadParams_t));
	params->sockAppel 			= &sockJeu;
	params->equipeId 			= &monEquipeId;
	params->dernierResultat 	= &dernierResultat;
	params->tourActuel 			= &tourActuel;
	params->semCanClose 		= &semCanClose;
	params->semPlacementOk 		= &semPlacementOk;
	params->semTirResultat 		= &semTirResultat;
	params->semTourActuel 		= &semTourActuel;
	params->partieTerminee 		= &partieTerminee;
	params->semStartGame		= &semStartGame;
	params->monTourPlacement	= &monTourPlacement;
	params->semTourPlacement	= &semTourPlacement;
	params->jeu					= &jeu;
	params->attendsResultatTir  = &attendsResultatTir;
	
	pthread_create(&threadDialJeu, 0, (void*)(void *) dialClt2SrvG, params);

	printf("\nEn attente que le HOST lance la partie...\n");
	sem_wait(&semStartGame);

	printf("\nLa partie commence !\n");
	sleep(1);

	placerEquipeReseau(monEquipe);
	
	jouerReseau(&jeu);
}

/**
 *	\fn			void lancerModeHost()
 *	\brief		Mode HOST - Lance un serveur de jeu et joue
 */
void lancerModeHost() {
	
	printf("\n=== MODE HOST ===\n");
	
	printf("Creation du serveur de jeu sur le port %d...\n", PORT_JEU);
	sockEcouteJeu = creerSocketEcoute("0.0.0.0", PORT_JEU);
	
	pthread_create(&threadEcouteJeu, 0, threadServeurJeu, NULL);
	pthread_detach(threadEcouteJeu);
	
	printf("En attente de stabilisation du serveur...\n");
	sleep(1);
	
	printf("\n===========================================\n");
	printf("  SERVEUR DE JEU CREE AVEC SUCCES !\n");
	printf("===========================================\n");
	printf("Votre IP : %s\n", self.address);
	printf("Votre port : %d\n", self.port);
	printf("\nLes autres joueurs peuvent maintenant\n");
	printf("se connecter a votre partie.\n");
	printf("\n===========================================\n");
	printf("Appuyez sur Entree quand vous etes pret\n");
	printf("a rejoindre votre propre serveur...\n");
	printf("===========================================\n");
	getchar();
	
	// SE CONNECTER D'ABORD
	printf("\nConnexion a votre propre serveur...\n");
	
	init_jeu(&jeu);
	sockJeu = connecterClt2Srv("127.0.0.1", PORT_JEU);
	
	Joueur joueur;
	joueur.id = 0;
	strcpy(joueur.nom, self.name);
	
	char buffer[100];
	joueur2str(&joueur, buffer);
	
	int status = enum2status(REQ, CONNECT);
	sendRequest(&sockJeu, status, POST, buffer, NULL);
	
	rep_t response;
	rcvResponse(&sockJeu, &response);
	
	if (getStatusRange(response.id) == ACK && getAction(response.id) == CONNECT) {
		// Format de réponse: "equipeId,indexJoueur"
		char *token = strtok(response.data, ",");
		if (token) {
			monEquipeId = atoi(token);
			token = strtok(NULL, ",");
			if (token) {
				monIndexJoueur = atoi(token);
			}
		}
		printf("Assigne a l'equipe %d (joueur %d)\n", monEquipeId, monIndexJoueur);
	} else {
		printf("Erreur de connexion au serveur de jeu\n");
		return;
	}
	
	Equipe *monEquipe = monEquipeId == 0 ? &jeu.equipeA : &jeu.equipeB;
	ajouter_joueur(monEquipe, self.name);
	
	gCltThreadParams_t *params  = malloc(sizeof(gCltThreadParams_t));
	params->sockAppel 			= &sockJeu;
	params->equipeId 			= &monEquipeId;
	params->dernierResultat 	= &dernierResultat;
	params->tourActuel 			= &tourActuel;
	params->semCanClose 		= &semCanClose;
	params->semPlacementOk 		= &semPlacementOk;
	params->semTirResultat 		= &semTirResultat;
	params->semTourActuel 		= &semTourActuel;
	params->partieTerminee 		= &partieTerminee;
	params->semStartGame		= &semStartGame;
	params->monTourPlacement	= &monTourPlacement;
	params->semTourPlacement	= &semTourPlacement;
	params->jeu					= &jeu;
	params->attendsResultatTir  = &attendsResultatTir;
	
	pthread_create(&threadDialJeu, 0, (void*)(void *) dialClt2SrvG, params);
	
	// MAINTENANT ATTENDRE LES AUTRES JOUEURS
	printf("\nAttendez que les joueurs rejoignent...\n");
	printf("Nombre de joueurs connectes: %d", nbClientsConnectes);
	fflush(stdout);
	
	int dernierNb = nbClientsConnectes;
	while (1) {
		if (nbClientsConnectes != dernierNb) {
			printf("\rNombre de joueurs connectes: %d", nbClientsConnectes);
			fflush(stdout);
			dernierNb = nbClientsConnectes;
		}
		
		if (nbClientsConnectes >= 2) {
			printf("\n\nAppuyez sur Entree pour lancer la partie...\n");
			getchar();
			
			pthread_mutex_lock(&mutexJeu);

			// START_GAME à tout le monde
			status = enum2status(ACK, START_GAME);
			printf("[DEBUG SERVEUR] Envoi START_GAME à %d clients\n", nbClientsConnectes);
			for (int i = 0; i < nbClientsConnectes; i++) {
				sendResponse(&clientsSockets[i], status, "GO", NULL);
				usleep(10000);  // ← 10ms entre chaque envoi
			}

			sleep(1);  // ← 1 SECONDE pour que tous traitent START_GAME

			// NEXT_TURN aux DEUX équipes
			int tourStatus = enum2status(ACK, NEXT_TURN);
			char tourBuffer[100];

			// Équipe 0
			Tour tourEquipe0 = {0, 0, 0};
			tour2str(&tourEquipe0, tourBuffer);
			printf("[DEBUG SERVEUR] Envoi NEXT_TURN équipe 0 (phase 0)\n");
			for (int i = 0; i < nbClientsConnectes; i += 2) {
				printf("[DEBUG SERVEUR] -> Joueur %d (FD=%d)\n", i, clientsSockets[i].fd);  // ← AJOUTER FD
				sendResponse(&clientsSockets[i], tourStatus, tourBuffer, NULL);
				usleep(10000);  // ← 10ms entre chaque envoi
			}

			// Équipe 1
			Tour tourEquipe1 = {1, 0, 0};
			tour2str(&tourEquipe1, tourBuffer);
			printf("[DEBUG SERVEUR] Envoi NEXT_TURN équipe 1 (phase 0)\n");
			for (int i = 1; i < nbClientsConnectes; i += 2) {
				printf("[DEBUG SERVEUR] -> Joueur %d (FD=%d)\n", i, clientsSockets[i].fd);  // ← AJOUTER FD
				sendResponse(&clientsSockets[i], tourStatus, tourBuffer, NULL);
				usleep(10000);  // ← 10ms entre chaque envoi
			}

			pthread_mutex_unlock(&mutexJeu);
			usleep(100000);
			break;
		}
		
		sleep(1);
	}
	
	printf("\nLa partie commence !\n");
	sleep(1);
	
	placerEquipeReseau(monEquipe);
	jouerReseau(&jeu);
}

/**
 *	\fn			void lancerModePlayer()
 *	\brief		Mode PLAYER - Cherche un HOST et se connecte
 */
void lancerModePlayer() {
	
	printf("\n=== MODE PLAYER ===\n");
	
	self.role = PLAYER;
	
	for (int i = 0; i < MAX_HOSTS_GET; i++) {
		createClientInfo(&hosts[i], "", 0, "", 0);
		hosts[i].status = DISCONNECTED;
	}
	
	printf("Recuperation de la liste des parties disponibles...\n");
	
	postRequest(&requestHosts, &semRequestFin);
	
	int nbHosts = 0;
	for (int i = 0; i < MAX_HOSTS_GET; i++) {
		if (hosts[i].role == HOST && strlen(hosts[i].name) > 0) {
			nbHosts++;
		}
	}
	
	if (nbHosts == 0) {
		printf("\nAucune partie disponible.\n");
		printf("Attendez qu'un joueur cree une partie (MODE HOST).\n");
		return;
	}
	
	printf("\n=== Parties disponibles ===\n");
	int idx = 0;
	for (int i = 0; i < MAX_HOSTS_GET; i++) {
		if (hosts[i].role == HOST && strlen(hosts[i].name) > 0) {
			printf("%d. %s (%s:%d)\n", idx+1, hosts[i].name, hosts[i].address, hosts[i].port);
			idx++;
		}
	}
	
	printf("\nChoisir une partie (1-%d): ", nbHosts);
	int choix;
	scanf("%d", &choix);
	getchar();
	
	if (choix < 1 || choix > nbHosts) {
		printf("Choix invalide\n");
		return;
	}
	
	idx = 0;
	clientInfo_t *host = NULL;
	for (int i = 0; i < MAX_HOSTS_GET; i++) {
		if (hosts[i].role == HOST && strlen(hosts[i].name) > 0) {
			idx++;
			if (idx == choix) {
				host = &hosts[i];
				break;
			}
		}
	}
	
	if (host == NULL) {
		printf("Erreur: HOST non trouve\n");
		return;
	}
	
	printf("\nConnexion a la partie de %s...\n", host->name);
	lancerClientJeu(host->address, host->port);
}

/**
 *	\fn			void client(char *adrIP, unsigned short port)
 *	\brief		Lance le client
 */
void client(char *adrIP, unsigned short port) {
	
	char userIP[INPUT_BUFFER_SIZE];
	short userPort;
	int choix;
	
	initClient();
	
	getSrvEAddress(adrIP, port, userIP, &userPort);
	
	printf("\nConfiguration du profil utilisateur:\n");
	printf("Nom d'utilisateur (3-10 chars): ");
	scanf("%10s", self.name);
	getchar();
	
	printf("\n=== BATAILLE NAVALE ===\n");
	printf("1. Creer une partie (HOST)\n");
	printf("2. Rejoindre une partie (PLAYER)\n");
	printf("Choix: ");
	scanf("%d", &choix);
	getchar();
	
	if (choix == 1) {
		self.role = HOST;
		self.port = PORT_JEU;
		getIpAddress(self.address);
		self.status = CONNECTING;
	} else {
		self.role = PLAYER;
		self.port = 0;
		getIpAddress(self.address);
		self.status = CONNECTING;
	}
	
	printf("\nConnexion au serveur d'enregistrement...\n");
	sockEnregistrement = connecterClt2Srv(userIP, userPort);
	
	eCltThreadParams_t *params = malloc(sizeof(eCltThreadParams_t));
	params->sockAppel 		= &sockEnregistrement;
	params->infos 			= &self;
	params->hostBuffer		= hosts;
	params->semCanClose		= &semCanClose;
	params->semRequestFin 	= &semRequestFin;
	
	pthread_create(&threadEnregistrement, 0, (void*)(void *) dialClt2SrvE, params);
	
	sleep(1);
	
	if (choix == 1) {
		lancerModeHost();
	} else {
		lancerModePlayer();
	}
	
	printf("\n=== Fin de la partie ===\n");
	printf("Appuyez sur Entree pour quitter...");
	getchar();
	
	onExit();
}

/**
 *	\fn			int main(int argc, char** argv)
 *	\brief		Programme principal
 */
int main(int argc, char** argv) {
	progName = argv[0];
	
	if (argc < 3) {
		fprintf(stderr, "usage : %s @IP port\n", basename(progName));
		fprintf(stderr, "lancement du  [PID:%d] connecte a [%s:%d]\n", 
				getpid(), IP_SERV_ENR, PORT_SERV_ENR);
		client(IP_SERV_ENR, PORT_SERV_ENR);
	}
	else {
		fprintf(stderr, "lancement du client [PID:%d] connecte a [%s:%d]\n",
			getpid(), argv[1], atoi(argv[2]));
		client(argv[1], atoi(argv[2]));
	}
	
	return 0;
}
