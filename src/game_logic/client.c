/**
 *	\file		client_jeu.c
 *	\brief		Client du jeu de bataille navale
 *	\author		MARTEL Mathieu
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

 /*
 *****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include "include/bataille_navale.h"
#include "include/affichage.h"
#include "include/logic.h"
#include "include/structSerial.h"

#include <dial.h>
#include <datastructs.h>

/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
#define IP_ANY		"127.0.0.1"
#define PORT_SRV	50001

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
pthread_t 		dialServThread;

socket_t 		sockAppel;

sem_t			semCanClose;
sem_t 			semPlacementOk;
sem_t 			semTirResultat;
sem_t			semTourActuel;

Jeu				jeu;
int 			mon_equipe_id;
Resultat 		dernier_resultat;
Tour			tour_actuel;

volatile sig_atomic_t mustDisconnect = 0;
volatile sig_atomic_t partie_terminee = 0;

/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

/**
 *	\fn			void onSignal(int code)
 *	\brief		Gestionnaire de signal SIGINT
 */
void onSignal(int code) {
	mustDisconnect = code == SIGINT;
}

/**
 *	\fn			void onExit()
 *	\brief		Fonction appelée à la sortie du programme
 */
void onExit() {
	int result;
	
	mustDisconnect = 1;
	
	do {
		result = sem_wait(&semCanClose);
	} while (result == -1 && errno == EINTR);
	
	CHECK(shutdown(sockAppel.fd, SHUT_WR),"-- PB shutdown() --");
	
	exit(EXIT_SUCCESS);
}

/**
 *	\fn			void initClient()
 *	\brief		Initialise le client (signaux, sémaphores)
 */
void initClient() {
	struct sigaction sa;
	CHECK(sigemptyset(&sa.sa_mask), "sigemptyset()");
	sa.sa_handler 	= onSignal;
	sa.sa_flags 	= 0;
	CHECK(sigaction(SIGINT, &sa, NULL), "sigaction();");
	
	CHECK(sem_init(&semCanClose, 0, 0), "sem_init()");
	CHECK(sem_init(&semPlacementOk, 0, 0), "sem_init()");
	CHECK(sem_init(&semTirResultat, 0, 0), "sem_init()");
	CHECK(sem_init(&semTourActuel, 0, 0), "sem_init()");
}

/**
 *	\fn			void dialClt2SrvJeu(void *params)
 *	\brief		Thread de dialogue avec le serveur de jeu
 */
void dialClt2SrvJeu(void *params) {
	socket_t *sockAppel = (socket_t *)params;
	
	while (1) {
		
		if (mustDisconnect) {
			char buffer[10];
			sprintf(buffer, "%d", mon_equipe_id);
			sendRequest(sockAppel, 199, DELETE, buffer, NULL);
			
			sem_post(&semCanClose);
			mustDisconnect = 0;
			break;
		}
		
		rep_t response;
		rcvResponse(sockAppel, &response);
		
		switch (response.id) {
			
			case 201:
				printf("Connecté au serveur de jeu !\n");
				printf("Vous êtes l'équipe %d\n", mon_equipe_id);
				break;
			
			case 211:
				sem_post(&semPlacementOk);
				break;
			
			case 221:
				str2resultat(response.data, &dernier_resultat);
				sem_post(&semTirResultat);
				break;
			
			case 231:
				str2tour(response.data, &tour_actuel);
				sem_post(&semTourActuel);
				break;
			
			case 241:
				{
					int vainqueur;
					sscanf(response.data, "%d", &vainqueur);
					
					clear_screen();
					printf("\n============================\n");
					if (vainqueur == mon_equipe_id) {
						printf("    VICTOIRE !\n");
					} else {
						printf("    DEFAITE\n");
					}
					printf("============================\n");
					
					partie_terminee = 1;
				}
				break;
			
			default:
				printf("Code réponse non géré: %d\n", response.id);
				break;
		}
	}
}

/**
 *	\fn			void envoyerPlacement(Placement *placement)
 *	\brief		Envoie un placement de bateau au serveur
 */
void envoyerPlacement(Placement *placement) {
	char buffer[100];
	place2str(placement, buffer);
	
	sendRequest(&sockAppel, 110, POST, buffer, NULL);
	
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
	tir.equipe_id = mon_equipe_id;
	
	tir2str(&tir, buffer);
	
	sendRequest(&sockAppel, 120, POST, buffer, NULL);
	
	sem_wait(&semTirResultat);
	
	return dernier_resultat;
}

/**
 *	\fn			void placer_equipe_reseau(Equipe *equipe)
 *	\brief		Phase de placement avec envoi au serveur
 */
void placer_equipe_reseau(Equipe *equipe) {
	int bateaux_ids[] = {2, 3, 4, 5, 6};
	int bateaux_longueurs[] = {5, 4, 3, 3, 2};
	
	clear_screen();
	printf("\n=== Placement %s ===\n", equipe->nom);
	
	for(int i = 0; i < NB_BATEAUX; i++) {
		int ok = 0;
		while(!ok) {
			clear_screen();
			printf("\n=== Placement %s ===\n", equipe->nom);
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
 *	\fn			void jouer_reseau(Jeu *jeu)
 *	\brief		Boucle de jeu avec communication réseau
 */
void jouer_reseau(Jeu *jeu) {
	clear_screen();
	printf("\n=== Debut de la partie ===\n");
	printf("Appuyez sur Entree...");
	getchar();
	
	while(!partie_terminee) {
		
		sem_wait(&semTourActuel);
		
		if (partie_terminee) break;
		
		if (tour_actuel.equipe_id != mon_equipe_id) {
			printf("\nEn attente du tour adverse...\n");
			continue;
		}
		
		clear_screen();
		Equipe *mon_equipe = mon_equipe_id == 0 ? &jeu->equipeA : &jeu->equipeB;
		
		printf("\n--- C'est votre tour ! ---\n");
		afficher_equipe(mon_equipe);
		afficher_vue(mon_equipe);
		
		int ligne, col;
		lire_coords(&ligne, &col);
		
		Resultat resultat = envoyerTir(ligne, col);
		
		mon_equipe->vue[resultat.ligne][resultat.col] = 
			resultat.touche ? TOUCHE(resultat.id_coule) : 1;
		
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
 *	\fn			void client(char *adrIP, unsigned short port)
 *	\brief		Lance le client de jeu
 */
void client(char *adrIP, unsigned short port) {
	char nom_joueur[50];
	
	initClient();
	init_jeu(&jeu);
	
	printf("Votre nom: ");
	scanf("%49s", nom_joueur);
	getchar();
	
	sockAppel = connecterClt2Srv(adrIP, port);
	
	Joueur joueur;
	joueur.id = 0;
	strcpy(joueur.nom, nom_joueur);
	
	char buffer[100];
	joueur2str(&joueur, buffer);
	sendRequest(&sockAppel, 100, POST, buffer, NULL);
	
	rep_t response;
	rcvResponse(&sockAppel, &response);
	
	if (response.id == 201) {
		sscanf(response.data, "%d", &mon_equipe_id);
		printf("Assigné à l'équipe %d\n", mon_equipe_id);
	} else {
		printf("Erreur de connexion\n");
		return;
	}
	
	Equipe *mon_equipe = mon_equipe_id == 0 ? &jeu.equipeA : &jeu.equipeB;
	ajouter_joueur(mon_equipe, nom_joueur);
	
	pthread_create(&dialServThread, 0, (void*)(void *) dialClt2SrvJeu, &sockAppel);
	
	placer_equipe_reseau(mon_equipe);
	
	jouer_reseau(&jeu);
	
	onExit();
}

/**
 *	\fn			int main(int argc, char** argv)
 *	\brief		Programme principal
 */
int main(int argc, char** argv) {
	progName = argv[0];
	
	if (argc<3) {
		fprintf(stderr,"usage : %s @IP port\n", basename(progName));
		fprintf(stderr,"lancement du client [PID:%d] connecté à [%s:%d]\n", 
				getpid(), IP_ANY, PORT_SRV);
		client(IP_ANY, PORT_SRV);
	}
	else {
		fprintf(stderr,"lancement du client [PID:%d] connecté à [%s:%d]\n",
			getpid(), argv[1], atoi(argv[2]));
		client(argv[1], atoi(argv[2]));
	}
	
	return 0;
}
