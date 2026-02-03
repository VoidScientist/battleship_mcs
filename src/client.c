/**
 *	\file		client.c
 *	\brief		Code du client pour le jeu
 *	\author		MARTEL Mathieu / ARCELON Louis
 *	\version	2.0
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
 *	\noop		C O N S T A N T E S
 */
// Réseau
#define IP_SERV_ENR			"0.0.0.0"
#define PORT_SERV_ENR		50000
#define PORT_JEU			50001
#define LOCALHOST			"127.0.0.1"
#define BIND_ALL			"0.0.0.0"

// Limites
#define MAX_JOUEURS			10
#define MIN_JOUEURS			2
#define NB_TYPES_BATEAUX	5

// Délais (microsecondes)
#define DELAY_MESSAGE_US	10000
#define DELAY_STABILIZE_US	100000

// Délais (secondes)
#define DELAY_START_GAME	1
#define DELAY_PLACEMENT		2
#define DELAY_SERVER_START	1

// Tailles buffer
#define BUFFER_SIZE			100
#define BUFFER_SMALL		20

// Équipes
#define EQUIPE_A			0
#define EQUIPE_B			1

/*
*****************************************************************************************
 *	\noop		M A C R O S
 */
#define CHECK(sts, msg) if ((sts)==-1) {perror(msg); exit(-1);}

/*
*****************************************************************************************
 *	\noop		V A R I A B L E S   G L O B A L E S
 */
char *progName;

// Serveur d'enregistrement
socket_t 		sockEnregistrement;
clientInfo_t 	self;
clientInfo_t	hosts[MAX_HOSTS_GET];
sem_t			semCanClose;
sem_t 			semRequestFin;
pthread_t 		threadEnregistrement;

// Serveur de jeu (HOST)
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
 *	\noop		F O N C T I O N S   U T I L I T A I R E S
 */

/**
 *	\fn			Equipe* obtenirMonEquipe(Jeu *jeu, int equipeId)
 *	\brief		Retourne l'équipe correspondant à l'ID
 */
Equipe* obtenirMonEquipe(Jeu *jeu, int equipeId) {
	return (equipeId == EQUIPE_A) ? &jeu->equipeA : &jeu->equipeB;
}

/**
 *	\fn			int estJoueurPrincipal()
 *	\brief		Vérifie si c'est le joueur principal de l'équipe
 */
int estJoueurPrincipal() {
	return monIndexJoueur == monEquipeId;
}

/**
 *	\fn			void afficherInfosPartie()
 *	\brief		Affiche les informations de la partie en cours
 */
void afficherInfosPartie() {
	printf("\n===========================================\n");
	printf("  SERVEUR DE JEU CREE AVEC SUCCES !\n");
	printf("===========================================\n");
	printf("Votre IP : %s\n", self.address);
	printf("Votre port : %d\n", self.port);
	printf("\nLes autres joueurs peuvent maintenant\n");
	printf("se connecter a votre partie.\n");
	printf("\n===========================================\n");
}

/*
*****************************************************************************************
 *	\noop		F O N C T I O N S   D ' I N I T I A L I S A T I O N
 */

/**
 *	\fn			void onSignal(int code)
 *	\brief		Gestionnaire de signal SIGINT
 */
void onSignal(int code) {

	mustDisconnect = code == SIGINT;

}

/**
 *	\fn			void initSemaphores()
 *	\brief		Initialise tous les sémaphores
 */
void initSemaphores() {
	CHECK(sem_init(&semCanClose, 0, 0), "sem_init()");
	CHECK(sem_init(&semRequestFin, 0, 0), "sem_init()");
	CHECK(sem_init(&semPlacementOk, 0, 0), "sem_init()");
	CHECK(sem_init(&semTirResultat, 0, 0), "sem_init()");
	CHECK(sem_init(&semTourActuel, 0, 0), "sem_init()");
	CHECK(sem_init(&semStartGame, 0, 0), "sem_init()");
	CHECK(sem_init(&semTourPlacement, 0, 0), "sem_init()");
}

/**
 *	\fn			void initSignaux()
 *	\brief		Initialise la gestion des signaux
 */
void initSignaux() {
	struct sigaction sa;
	CHECK(sigemptyset(&sa.sa_mask), "sigemptyset()");
	sa.sa_handler = onSignal;
	sa.sa_flags = 0;
	CHECK(sigaction(SIGINT, &sa, NULL), "sigaction()");
}

/**
 *	\fn			void initHostsBuffer()
 *	\brief		Initialise le buffer des hosts
 */
void initHostsBuffer() {
	for (int i = 0; i < MAX_HOSTS_GET; i++) {
		createClientInfo(&hosts[i], "", 0, "", 0);
		hosts[i].status = DISCONNECTED;
	}
}

/**
 *	\fn			void initClient()
 *	\brief		Initialise le client complet
 */
void initClient() {
	initSignaux();
	initSemaphores();
	
	monTourPlacement = 0;
	monIndexJoueur = -1;
	
	initHostsBuffer();
}

/**
 *	\fn			void onExit()
 *	\brief		Fonction de nettoyage à la sortie
 */
void onExit() {

	int result;
	
	mustDisconnect = 1;
	
	do {
		result = sem_wait(&semCanClose);
	} while (result == -1 && errno == EINTR);
	
	CHECK(shutdown(sockEnregistrement.fd, SHUT_WR), "shutdown enregistrement");
	
	if (sockJeu.fd > 0) {
		CHECK(shutdown(sockJeu.fd, SHUT_WR), "shutdown jeu");
	}
	
	exit(EXIT_SUCCESS);
}

/*
*****************************************************************************************
 *	\noop		F O N C T I O N S   R É S E A U
 */

/**
 *	\fn			void envoyerPlacement(Placement *placement)
 *	\brief		Envoie un placement au serveur
 */
void envoyerPlacement(Placement *placement) {
	int status = enum2status(REQ, PLACE);
	sendRequest(&sockJeu, status, POST, placement, (pFct)place2str);
	sem_wait(&semPlacementOk);
}

/**
 *	\fn			Resultat envoyerTir(int ligne, int col)
 *	\brief		Envoie un tir au serveur et attend le résultat
 */
Resultat envoyerTir(int ligne, int col) {
	
	int status = enum2status(REQ, SHOOT);
	Tir tir = {

		.ligne 		= ligne,
		.col 		= col,
		.equipe_id 	= monEquipeId
	
	};
	
	attendsResultatTir = 1;
	
	sendRequest(&sockJeu, status, POST, &tir, (pFct)tir2str);
	
	sem_wait(&semTirResultat);
	
	attendsResultatTir = 0;
	
	return dernierResultat;
}

/**
 *	\fn			void envoyerMessageEquipe(int equipeId, int tourStatus, Tour *tour)
 *	\brief		Envoie un message à tous les membres d'une équipe
 */
void envoyerMessageEquipe(int equipeId, int tourStatus, Tour *tour) {

	for (int i = equipeId; i < nbClientsConnectes; i += 2) {
	
		sendResponse(&clientsSockets[i], tourStatus, tour, (pFct)tour2str);
		usleep(DELAY_MESSAGE_US);
	
	}

}

/**
 *	\fn			void envoyerSignalStartGame()
 *	\brief		Envoie le signal START_GAME à tous les clients
 */
void envoyerSignalStartGame() {
	
	int status = enum2status(ACK, START_GAME);
	
	for (int i = 0; i < nbClientsConnectes; i++) {
		sendResponse(&clientsSockets[i], status, "GO", NULL);
		usleep(DELAY_MESSAGE_US);
	}
	
	sleep(DELAY_START_GAME);
}

/**
 *	\fn			void envoyerSignauxPlacement()
 *	\brief		Envoie les signaux de début de placement aux deux équipes
 */
void envoyerSignauxPlacement() {
	
	int tourStatus = enum2status(ACK, NEXT_TURN);
	
	Tour tourEquipeA = {EQUIPE_A, 0, 0};
	Tour tourEquipeB = {EQUIPE_B, 0, 0};
	
	envoyerMessageEquipe(EQUIPE_A, tourStatus, &tourEquipeA);
	envoyerMessageEquipe(EQUIPE_B, tourStatus, &tourEquipeB);

}

/**
 *	\fn			void envoyerSignauxDemarrage()
 *	\brief		Envoie tous les signaux de démarrage de partie
 */
void envoyerSignauxDemarrage() {

	pthread_mutex_lock(&mutexJeu);
	envoyerSignalStartGame();
	envoyerSignauxPlacement();
	pthread_mutex_unlock(&mutexJeu);

}

/*
*****************************************************************************************
 *	\noop		F O N C T I O N S   D E   J E U
 */

/**
 *	\fn			int lirePlacement(Equipe *equipe, int numBateau, Placement *placement)
 *	\brief		Lit et valide un placement de bateau
 */
int lirePlacement(Equipe *equipe, int numBateau, Placement *placement) {

	Orientation orient;
	int 	bateaux_ids[] 		= {2, 3, 4, 5, 6};
	int 	bateaux_longueurs[] = {5, 4, 3, 3, 2};
	int 	ligne, col;
	char 	orient_char;
	
	clear_screen();
	printf("\n=== Placement %s ===\n", equipe->nom);
	printf("\n=== C'EST VOTRE TOUR ! ===\n");
	printf("\nBateau %d/%d (longueur %d)\n", numBateau + 1, NB_BATEAUX, bateaux_longueurs[numBateau]);

	afficher_equipe(equipe);
	
	orient = (orient_char == 'H') ? HORIZONTAL : VERTICAL;
	lire_bateau(&ligne, &col, &orient_char);
	
	
	if (placer_bateau(&equipe->grille, bateaux_ids[numBateau], 
	                   bateaux_longueurs[numBateau], ligne, col, orient)) {
		placement->id = bateaux_ids[numBateau];
		placement->longueur = bateaux_longueurs[numBateau];
		placement->ligne = ligne;
		placement->col = col;
		placement->orient = orient;
		return 1;
	}
	
	return 0;
}

/**
 *	\fn			void placerUnBateau(Equipe *equipe, int numBateau)
 *	\brief		Gère le placement d'un bateau
 */
void placerUnBateau(Equipe *equipe, int numBateau) {
	
	Placement placement;
	int ok = 0;
	
	printf("En attente de votre tour de placement...\n");
	sem_wait(&semTourPlacement);
	
	while (!ok) {
		if (lirePlacement(equipe, numBateau, &placement)) {
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

/**
 *	\fn			void placerEquipeReseau(Equipe *equipe)
 *	\brief		Phase de placement complète
 */
void placerEquipeReseau(Equipe *equipe) {
	for (int i = 0; i < NB_BATEAUX; i++) {
		placerUnBateau(equipe, i);
	}
	
	clear_screen();
	printf("\n=== Placement termine ===\n");
	afficher_equipe(equipe);
	printf("\nEn attente des autres joueurs...\n");
}

/**
 *	\fn			void attendreSonTour()
 *	\brief		Attend que ce soit le tour de l'équipe
 */
void attendreSonTour() {
	while (tourActuel.equipe_id != monEquipeId && !partieTerminee) {
		printf("\nEn attente du tour adverse...\n");
		sem_wait(&semTourActuel);
	}
}

/**
 *	\fn			void afficherResultatTir(Resultat resultat)
 *	\brief		Affiche le résultat du tir
 */
void afficherResultatTir(Resultat resultat) {
	if (resultat.touche) {
		printf(resultat.coule ? "\nTouché-Coulé !\n" : "\nTouché !\n");
	} else {
		printf("\nLoupé.\n");
	}
}

/**
 *	\fn			void executerTour(Equipe *equipe)
 *	\brief		Exécute un tour de jeu complet
 */
void executerTour(Equipe *equipe) {
	clear_screen();
	printf("\n--- C'est votre tour ! ---\n");
	afficher_equipe(equipe);
	afficher_vue(equipe);
	
	int ligne, col;
	lire_coords(&ligne, &col);
	
	Resultat resultat = envoyerTir(ligne, col);
	
	// Mettre à jour la vue
	equipe->vue[resultat.ligne][resultat.col] = 
		resultat.touche ? TOUCHE(resultat.id_coule) : 1;
	
	// Réafficher avec le résultat
	clear_screen();
	printf("\n--- Résultat du tir ! ---\n");
	afficher_equipe(equipe);
	afficher_vue(equipe);
	
	afficherResultatTir(resultat);
	
	printf("Appuyez sur Entree...");
	getchar();
}

/**
 *	\fn			void jouerReseau(Jeu *jeu)
 *	\brief		Boucle principale de jeu
 */
void jouerReseau(Jeu *jeu) {
	clear_screen();
	printf("\n=== Debut de la partie ===\n");
	
	while (!partieTerminee) {
		sem_wait(&semTourActuel);
		
		if (partieTerminee) break;
		
		attendreSonTour();
		
		if (partieTerminee) break;
		
		if (!estJoueurPrincipal()) {
			printf("\nC'est au tour de votre coéquipier principal...\n");
			continue;
		}
		
		executerTour(obtenirMonEquipe(jeu, monEquipeId));
	}
}

/*
*****************************************************************************************
 *	\noop		F O N C T I O N S   S E R V E U R
 */

/**
 *	\fn			void *threadServeurJeu(void *params)
 *	\brief		Thread d'écoute du serveur de jeu
 */
void *threadServeurJeu(void *params) {
	
	init_jeu(&jeuServeur);
	printf("Serveur de jeu en ecoute sur le port %d\n", PORT_JEU);
	
	while (nbClientsConnectes < MAX_JOUEURS) {
		socket_t *sockDial = malloc(sizeof(socket_t));
		*sockDial = accepterClt(sockEcouteJeu);
		
		pthread_mutex_lock(&mutexJeu);
		clientsSockets[nbClientsConnectes] = *sockDial;
		int equipeId = nbClientsConnectes % 2;
		int numeroJoueur = nbClientsConnectes;
		nbClientsConnectes++;
		pthread_mutex_unlock(&mutexJeu);
		
		printf("Joueur %d connecte (equipe %d)\n", numeroJoueur + 1, equipeId);
		
		gServThreadParams_t *threadParams = malloc(sizeof(gServThreadParams_t));
		threadParams->equipeId = equipeId;
		threadParams->numeroJoueur = numeroJoueur;
		threadParams->sockDial = sockDial;
		threadParams->jeu = &jeuServeur;
		threadParams->clientsSockets = clientsSockets;
		threadParams->nbClientsConnectes = &nbClientsConnectes;
		threadParams->phasePlacementTermine = phasePlacementTermine;
		threadParams->mutexJeu = &mutexJeu;
		
		pthread_t thread;
		pthread_create(&thread, 0, (void*)(void*)dialSrvG2Clt, threadParams);
		pthread_detach(thread);
	}
	
	printf("%d joueurs connectes. La partie peut commencer !\n", MAX_JOUEURS);
	return NULL;
}

/**
 *	\fn			void creerServeurJeu()
 *	\brief		Crée et démarre le serveur de jeu
 */
void creerServeurJeu() {
	printf("Creation du serveur de jeu sur le port %d...\n", PORT_JEU);
	sockEcouteJeu = creerSocketEcoute(BIND_ALL, PORT_JEU);
	
	pthread_create(&threadEcouteJeu, 0, threadServeurJeu, NULL);
	pthread_detach(threadEcouteJeu);
	
	printf("En attente de stabilisation du serveur...\n");
	sleep(DELAY_SERVER_START);
}

/**
 *	\fn			void attendreJoueurs()
 *	\brief		Attend que suffisamment de joueurs se connectent
 */
void attendreJoueurs() {
	printf("\nAttendez que les joueurs rejoignent...\n");
	printf("Nombre de joueurs connectes: %d", nbClientsConnectes);
	fflush(stdout);
	
	int dernierNb = nbClientsConnectes;
	while (nbClientsConnectes < MIN_JOUEURS) {
		if (nbClientsConnectes != dernierNb) {
			printf("\rNombre de joueurs connectes: %d", nbClientsConnectes);
			fflush(stdout);
			dernierNb = nbClientsConnectes;
		}
		sleep(1);
	}
	
	printf("\n\nAppuyez sur Entree pour lancer la partie...\n");
	getchar();
}

/*
*****************************************************************************************
 *	\noop		F O N C T I O N S   C O N N E X I O N
 */

/**
 *	\fn			gCltThreadParams_t* creerParamsThreadClient()
 *	\brief		Crée les paramètres du thread client
 */
gCltThreadParams_t* creerParamsThreadClient() {
	gCltThreadParams_t *params = malloc(sizeof(gCltThreadParams_t));
	params->sockAppel = &sockJeu;
	params->equipeId = &monEquipeId;
	params->dernierResultat = &dernierResultat;
	params->tourActuel = &tourActuel;
	params->semCanClose = &semCanClose;
	params->semPlacementOk = &semPlacementOk;
	params->semTirResultat = &semTirResultat;
	params->semTourActuel = &semTourActuel;
	params->partieTerminee = &partieTerminee;
	params->semStartGame = &semStartGame;
	params->monTourPlacement = &monTourPlacement;
	params->semTourPlacement = &semTourPlacement;
	params->jeu = &jeu;
	params->attendsResultatTir = &attendsResultatTir;
	return params;
}

/**
 *	\fn			void recevoirAssignationEquipe(rep_t *response)
 *	\brief		Traite l'assignation d'équipe du serveur
 */
void recevoirAssignationEquipe(rep_t *response) {
	char *token = strtok(response->data, ",");
	if (token) {
		monEquipeId = atoi(token);
		token = strtok(NULL, ",");
		if (token) {
			monIndexJoueur = atoi(token);
		}
	}
	printf("Assigne a l'equipe %d (joueur %d)\n", monEquipeId, monIndexJoueur);
}

/**
 *	\fn			int connecterAuServeurJeu(char *ip, int port)
 *	\brief		Établit la connexion au serveur de jeu
 */
int connecterAuServeurJeu(char *ip, int port) {
	init_jeu(&jeu);
	
	printf("\nConnexion au serveur de jeu %s:%d...\n", ip, port);
	sockJeu = connecterClt2Srv(ip, port);
	
	Joueur joueur = {.id = 0};
	strcpy(joueur.nom, self.name);
	
	int status = enum2status(REQ, CONNECT);
	sendRequest(&sockJeu, status, POST, &joueur, (pFct)joueur2str);
	
	rep_t response;
	rcvResponse(&sockJeu, &response);
	
	if (getStatusRange(response.id) != ACK || getAction(response.id) != CONNECT) {
		printf("Erreur de connexion au serveur de jeu\n");
		return 0;
	}
	
	recevoirAssignationEquipe(&response);
	
	Equipe *monEquipe = obtenirMonEquipe(&jeu, monEquipeId);
	ajouter_joueur(monEquipe, self.name);
	
	gCltThreadParams_t *params = creerParamsThreadClient();
	pthread_create(&threadDialJeu, 0, (void*)(void*)dialClt2SrvG, params);
	
	return 1;
}

/**
 *	\fn			void demarrerThreadEnregistrement()
 *	\brief		Démarre le thread de dialogue avec le serveur d'enregistrement
 */
void demarrerThreadEnregistrement(char *ip, short port) {
	sockEnregistrement = connecterClt2Srv(ip, port);
	
	eCltThreadParams_t *params = malloc(sizeof(eCltThreadParams_t));
	params->sockAppel = &sockEnregistrement;
	params->infos = &self;
	params->hostBuffer = hosts;
	params->semCanClose = &semCanClose;
	params->semRequestFin = &semRequestFin;
	
	pthread_create(&threadEnregistrement, 0, (void*)(void*)dialClt2SrvE, params);
	sleep(DELAY_SERVER_START);
}

/*
*****************************************************************************************
 *	\noop		F O N C T I O N S   M O D E S   D E   J E U
 */

/**
 *	\fn			void lancerClientJeu(char *ip, int port)
 *	\brief		Lance le client de jeu et la partie
 */
void lancerClientJeu(char *ip, int port) {
	if (!connecterAuServeurJeu(ip, port)) return;
	
	printf("\nEn attente que le HOST lance la partie...\n");
	sem_wait(&semStartGame);
	
	printf("\nLa partie commence !\n");
	sleep(DELAY_START_GAME);
	
	Equipe *monEquipe = obtenirMonEquipe(&jeu, monEquipeId);
	placerEquipeReseau(monEquipe);
	jouerReseau(&jeu);
}

/**
 *	\fn			void lancerModeHost()
 *	\brief		Mode HOST - Crée et héberge une partie
 */
void lancerModeHost() {
	printf("\n=== MODE HOST ===\n");
	
	creerServeurJeu();
	afficherInfosPartie();
	
	printf("Appuyez sur Entree quand vous etes pret\n");
	printf("a rejoindre votre propre serveur...\n");
	printf("===========================================\n");
	getchar();
	
	printf("\nConnexion a votre propre serveur...\n");
	if (!connecterAuServeurJeu(LOCALHOST, PORT_JEU)) return;
	
	attendreJoueurs();
	envoyerSignauxDemarrage();
	
	printf("\nLa partie commence !\n");
	sleep(DELAY_START_GAME);
	
	Equipe *monEquipe = obtenirMonEquipe(&jeu, monEquipeId);
	placerEquipeReseau(monEquipe);
	jouerReseau(&jeu);
}

/**
 *	\fn			clientInfo_t* selectionnerHost(int nbHosts)
 *	\brief		Permet à l'utilisateur de sélectionner un host
 */
clientInfo_t* selectionnerHost(int nbHosts) {
	printf("\nChoisir une partie (1-%d): ", nbHosts);
	int choix;
	scanf("%d", &choix);
	getchar();
	
	if (choix < 1 || choix > nbHosts) {
		printf("Choix invalide\n");
		return NULL;
	}
	
	int idx = 0;
	for (int i = 0; i < MAX_HOSTS_GET; i++) {
		if (hosts[i].role == HOST && strlen(hosts[i].name) > 0) {
			idx++;
			if (idx == choix) {
				return &hosts[i];
			}
		}
	}
	
	return NULL;
}

/**
 *	\fn			int afficherHostsDisponibles()
 *	\brief		Affiche les hosts disponibles et retourne leur nombre
 */
int afficherHostsDisponibles() {
	int nbHosts = 0;
	
	printf("\n=== Parties disponibles ===\n");
	for (int i = 0; i < MAX_HOSTS_GET; i++) {
		if (hosts[i].role == HOST && strlen(hosts[i].name) > 0) {
			printf("%d. %s (%s:%d)\n", nbHosts + 1, hosts[i].name, 
			       hosts[i].address, hosts[i].port);
			nbHosts++;
		}
	}
	
	return nbHosts;
}

/**
 *	\fn			void lancerModePlayer()
 *	\brief		Mode PLAYER - Rejoint une partie existante
 */
void lancerModePlayer() {
	printf("\n=== MODE PLAYER ===\n");
	
	self.role = PLAYER;
	initHostsBuffer();
	
	printf("Recuperation de la liste des parties disponibles...\n");
	postRequest(&requestHosts, &semRequestFin);
	
	int nbHosts = afficherHostsDisponibles();
	
	if (nbHosts == 0) {
		printf("\nAucune partie disponible.\n");
		printf("Attendez qu'un joueur cree une partie (MODE HOST).\n");
		return;
	}
	

	// ça on a pas => à greffer à ce que j'ai
	clientInfo_t *host = selectionnerHost(nbHosts);
	if (!host) {
		printf("Erreur: HOST non trouve\n");
		return;
	}
	
	printf("\nConnexion a la partie de %s...\n", host->name);
	lancerClientJeu(host->address, host->port);
}

/*
*****************************************************************************************
 *	\noop		F O N C T I O N   P R I N C I P A L E
 */

/**
 *	\fn			void configurerClient()
 *	\brief		Configure les informations du client
 */
void configurerClient(int choixMode) {
	printf("\nConfiguration du profil utilisateur:\n");
	printf("Nom d'utilisateur (3-10 chars): ");
	scanf("%10s", self.name);
	getchar();
	
	if (choixMode == 1) {
		self.role = HOST;
		self.port = PORT_JEU;
		strcpy(self.address, LOCALHOST);
	} else {
		self.role = PLAYER;
		self.port = 0;
		strcpy(self.address, "");
	}
	
	self.status = CONNECTED;
}

/**
 *	\fn			int afficherMenuPrincipal()
 *	\brief		Affiche le menu et retourne le choix
 */
int afficherMenuPrincipal() {
	int choix;
	
	printf("\n=== BATAILLE NAVALE ===\n");
	printf("1. Creer une partie (HOST)\n");
	printf("2. Rejoindre une partie (PLAYER)\n");
	printf("Choix: ");
	scanf("%d", &choix);
	getchar();
	
	return choix;
}

/**
 *	\fn			void client(char *adrIP, unsigned short port)
 *	\brief		Point d'entrée principal du client
 */
void client(char *adrIP, unsigned short port) {
	char userIP[INPUT_BUFFER_SIZE];
	short userPort;
	
	initClient();
	getSrvEAddress(adrIP, port, userIP, &userPort);
	
	int choix = afficherMenuPrincipal();
	configurerClient(choix);
	
	printf("\nConnexion au serveur d'enregistrement...\n");
	demarrerThreadEnregistrement(userIP, userPort);
	
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
 *	\brief		Point d'entrée du programme
 */
int main(int argc, char** argv) {
	progName = argv[0];
	
	if (argc < 3) {
		fprintf(stderr, "usage : %s @IP port\n", basename(progName));
		fprintf(stderr, "lancement du [PID:%d] connecte a [%s:%d]\n", 
		        getpid(), IP_SERV_ENR, PORT_SERV_ENR);
		client(IP_SERV_ENR, PORT_SERV_ENR);
	} else {
		fprintf(stderr, "lancement du client [PID:%d] connecte a [%s:%d]\n",
		        getpid(), argv[1], atoi(argv[2]));
		client(argv[1], atoi(argv[2]));
	}
	
	return 0;
}
