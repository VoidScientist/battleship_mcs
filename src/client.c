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
#define BIND_ALL			"0.0.0.0"
#define PORT_SERV_ENR		50000
#define PORT_JEU			50001

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

/*
*****************************************************************************************
 *	\noop		M A C R O S
 */
#define CHECK(sts, msg) if ((sts)==-1) {perror(msg); exit(-1);}

/*
*****************************************************************************************
 *	\noop		V A R I A B L E S   G L O B A L E S
 */
char 			*progName;

// Serveur d'enregistrement
socket_t 		sockEnregistrement;
clientInfo_t 	self;
clientInfo_t	hosts[MAX_HOSTS_GET];
sem_t			semCanClose;
sem_t 			semRequestFin;

// Serveur de jeu (HOST)
Jeu				jeuServeur;
socket_t		clientsSockets[MAX_JOUEURS];
int				nbClientsConnectes = 0;
int				phasePlacementTermine[2] = {0, 0};
pthread_mutex_t	mutexJeu = PTHREAD_MUTEX_INITIALIZER;
socket_t 		sockEcouteJeu;

// Client de jeu
socket_t 		sockJeu;
pthread_t 		threadDialJeu;
Jeu				jeu;
int 			monEquipeId;
int 			monIndexJoueur;
Resultat 		dernierResultat;
Tour			tourActuel;
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
 *	\fn			void afficherInfosPartie()
 *	\brief		Affiche les informations de la partie en cours
 */
void afficherInfosPartie(clientInfo_t *client) {
	printf("\n===========================================\n");
	printf("  SERVEUR DE JEU CREE AVEC SUCCES !\n");
	printf("===========================================\n");
	printf("Votre IP : %s\n", client->address);
	printf("Votre port : %d\n", client->port);
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
 *	\fn			void initClient()
 *	\brief		Initialise le client complet
 */
void initClient() {
	
	struct sigaction sa;
	CHECK(sigemptyset(&sa.sa_mask), "sigemptyset()");
	sa.sa_handler = onSignal;
	sa.sa_flags   = 0;
	CHECK(sigaction(SIGINT, &sa, NULL), "sigaction()");

	CHECK(sem_init(&semCanClose, 0, 0), "sem_init()");
	CHECK(sem_init(&semRequestFin, 0, 0), "sem_init()");
	CHECK(sem_init(&semPlacementOk, 0, 0), "sem_init()");
	CHECK(sem_init(&semTirResultat, 0, 0), "sem_init()");
	CHECK(sem_init(&semTourActuel, 0, 0), "sem_init()");
	CHECK(sem_init(&semStartGame, 0, 0), "sem_init()");
	CHECK(sem_init(&semTourPlacement, 0, 0), "sem_init()");
	
	monTourPlacement = 0;
	monIndexJoueur 	 = -1;
	
	resetClientInfoArray(hosts, MAX_HOSTS_GET);
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

/**
 * @brief     nettoyage, requêtes et affichage des hôtes
 */
void onDisplayHosts() {

	resetClientInfoArray(hosts, MAX_HOSTS_GET);

	postRequest(&requestHosts, &semRequestFin);
	
	displayHosts(hosts, MAX_HOSTS_GET);

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
	sendRequest(&sockJeu, status, POST, placement, (pFct) place2str);
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
	
	envoyerAEquipe(clientsSockets, nbClientsConnectes, EQUIPE_A, tourStatus, &tourEquipeA, (pFct) tour2str);
	envoyerAEquipe(clientsSockets, nbClientsConnectes, EQUIPE_B, tourStatus, &tourEquipeB, (pFct) tour2str);

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
	
	
	if (placer_bateau(&equipe->grille, bateaux_ids[numBateau], bateaux_longueurs[numBateau], ligne, col, orient)) {
		
		placement->id 			= bateaux_ids[numBateau];
		placement->longueur 	= bateaux_longueurs[numBateau];
		placement->ligne 		= ligne;
		placement->col 			= col;
		placement->orient 		= orient;
		
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

	int 		ligne, col;
	Resultat 	resultat;

	clear_screen();
	printf("\n--- C'est votre tour ! ---\n");
	afficher_equipe(equipe);
	afficher_vue(equipe);
	
	
	lire_coords(&ligne, &col);
	
	resultat = envoyerTir(ligne, col);
	
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
		
		if (monIndexJoueur != monEquipeId) {
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

	pthread_t thread;
	int equipeId, numeroJoueur;

	init_jeu(&jeuServeur);
	printf("Serveur de jeu en ecoute sur le port %d\n", PORT_JEU);
	
	while (nbClientsConnectes < MAX_JOUEURS) {
		socket_t *sockDial = malloc(sizeof(socket_t));
		*sockDial = accepterClt(sockEcouteJeu);
		
		
		pthread_mutex_lock(&mutexJeu);
		
			clientsSockets[nbClientsConnectes] 	= *sockDial;
			equipeId 							= nbClientsConnectes % 2;
			numeroJoueur 						= nbClientsConnectes++;
		
		pthread_mutex_unlock(&mutexJeu);
		
		printf("Joueur %d connecte (equipe %d)\n", numeroJoueur + 1, equipeId);
		
		gServThreadParams_t *threadParams 	= malloc(sizeof(gServThreadParams_t));
		threadParams->equipeId 				= equipeId;
		threadParams->numeroJoueur 			= numeroJoueur;
		threadParams->sockDial 				= sockDial;
		threadParams->jeu 					= &jeuServeur;
		threadParams->clientsSockets 		= clientsSockets;
		threadParams->nbClientsConnectes 	= &nbClientsConnectes;
		threadParams->phasePlacementTermine = phasePlacementTermine;
		threadParams->mutexJeu 				= &mutexJeu;
		
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

	pthread_t 	threadEcouteJeu;

	printf("Creation du serveur de jeu sur le port %d...\n", PORT_JEU);
	sockEcouteJeu = creerSocketEcoute(BIND_ALL, self.port);
	
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
	printf("Nombre de joueurs connectes: %d\n", nbClientsConnectes);
	
	int dernierNb = nbClientsConnectes;
	while (nbClientsConnectes < MIN_JOUEURS) {
		if (nbClientsConnectes != dernierNb) {
			printf("\rNombre de joueurs connectes: %d\n", nbClientsConnectes);
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
 *	\fn			int connecterAuServeurJeu(char *ip, int port)
 *	\brief		Établit la connexion au serveur de jeu
 */
int connecterAuServeurJeu(char *ip, int port) {

	gCltThreadParams_t 	*params;
	int 				status;
	rep_t				response;
	Equipe 				*monEquipe;
	Joueur 				joueur 		= {.id = 0};

	init_jeu(&jeu);
	
	printf("\nConnexion au serveur de jeu %s:%d...\n", ip, port);
	sockJeu = connecterClt2Srv(ip, port);
	
	strcpy(joueur.nom, self.name);
	
	status = enum2status(REQ, CONNECT);
	sendRequest(&sockJeu, status, POST, &joueur, (pFct)joueur2str);
	
	rcvResponse(&sockJeu, &response);
	
	if (getStatusRange(response.id) != ACK || getAction(response.id) != CONNECT) {
		printf("Erreur de connexion au serveur de jeu\n");
		return 0;
	}
	
	sscanf(response.data, "%d,%d", &monEquipeId, &monIndexJoueur);
	
	monEquipe = obtenirMonEquipe(&jeu, monEquipeId);
	ajouter_joueur(monEquipe, self.name);
	
	params 						= malloc(sizeof(gCltThreadParams_t));
	params->sockAppel 			= &sockJeu;
	params->equipeId 			= &monEquipeId;
	params->dernierResultat 	= &dernierResultat;
	params->tourActuel 			= &tourActuel;
	params->semCanClose 		= &semCanClose;
	params->semPlacementOk 		= &semPlacementOk;
	params->semTirResultat 		= &semTirResultat;
	params->semTourActuel 		= &semTourActuel;
	params->semStartGame 		= &semStartGame;
	params->monTourPlacement 	= &monTourPlacement;
	params->semTourPlacement 	= &semTourPlacement;
	params->jeu 				= &jeu;
	params->attendsResultatTir 	= &attendsResultatTir;

	pthread_create(&threadDialJeu, 0, (void*)(void*)dialClt2SrvG, params);
	
	return 1;
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

	Equipe *monEquipe;

	if (!connecterAuServeurJeu(ip, port)) return;
	
	printf("\nEn attente que le HOST lance la partie...\n");
	sem_wait(&semStartGame);
	
	printf("\nLa partie commence !\n");
	sleep(DELAY_START_GAME);
	
	monEquipe = obtenirMonEquipe(&jeu, monEquipeId);
	placerEquipeReseau(monEquipe);
	jouerReseau(&jeu);
}

/**
 *	\fn			void lancerModeHost()
 *	\brief		Mode HOST - Crée et héberge une partie
 */
void lancerModeHost() {

	Equipe *monEquipe;

	printf("\n=== MODE HOST ===\n");
	
	creerServeurJeu();
	afficherInfosPartie(&self);
	
	printf("Appuyez sur Entree quand vous etes pret\n");
	printf("a rejoindre votre propre serveur...\n");
	printf("===========================================\n");
	getchar();
	
	printf("\nConnexion a votre propre serveur...\n");
	if (!connecterAuServeurJeu(self.address, self.port)) return;
	
	attendreJoueurs();
	envoyerSignauxDemarrage();
	
	printf("\nLa partie commence !\n");
	sleep(DELAY_START_GAME);
	
	monEquipe = obtenirMonEquipe(&jeu, monEquipeId);
	placerEquipeReseau(monEquipe);
	jouerReseau(&jeu);
}

/*
*****************************************************************************************
 *	\noop		F O N C T I O N   P R I N C I P A L E
 */

/**
 *	\fn			void client(char *adrIP, unsigned short port)
 *	\brief		Point d'entrée principal du client
 */
void client(char *adrIP, unsigned short port) {

	char 				userIP[INPUT_BUFFER_SIZE];
	short 				userPort;
	pthread_t 			threadEnregistrement;
	playerMenuParams_t	menuParams;
	clientInfo_t 		chosenHost;


	initClient();

	getSrvEAddress(adrIP, port, userIP, &userPort);
	
	setupUserInfos(&self);
		
	sockEnregistrement = connecterClt2Srv(userIP, userPort);
	
	eCltThreadParams_t *params 	= malloc(sizeof(eCltThreadParams_t));
	params->sockAppel 			= &sockEnregistrement;
	params->infos 				= &self;
	params->hostBuffer 			= hosts;
	params->semCanClose 		= &semCanClose;
	params->semRequestFin 		= &semRequestFin;
	
	pthread_create(&threadEnregistrement, 0, (void*)(void*)dialClt2SrvE, params);
	pthread_detach(threadEnregistrement);
	
	menuParams.showHosts	= onDisplayHosts;
	menuParams.exitProgram	= onExit;
	menuParams.hosts 		= hosts;
	menuParams.chosenHost   = &chosenHost;

	if (self.role == HOST) {

		lancerModeHost();

	} else {

		displayPlayerMenu(menuParams);


		lancerClientJeu(chosenHost.address, chosenHost.port);
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
		        getpid(), BIND_ALL, PORT_SERV_ENR);
		client(BIND_ALL, PORT_SERV_ENR);
	} else {
		fprintf(stderr, "lancement du client [PID:%d] connecte a [%s:%d]\n",
		        getpid(), argv[1], atoi(argv[2]));
		client(argv[1], atoi(argv[2]));
	}
	
	return 0;
}
