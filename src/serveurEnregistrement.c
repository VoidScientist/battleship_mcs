/**
 *	\file		serveurEnregistrement.c
 *	\brief		serveur d'enregistrement.
 *	\author		ARCELON Louis
 *	\date		31 janvier 2026
 *	\version	1.0
 */
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <time.h>

#include <libgen.h>
#include <dial.h>
#include <datastructs.h>
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
/**
 *	\def		IP_ANY
 *	\brief		Adresse IP par défaut du serveur
 */
#define IP_ANY		"0.0.0.0"
/**
 *	\def		PORT_SRV
 *	\brief		Numéro de port par défaut du serveur
 */
#define PORT_SRV			50000
#define MAX_CLIENTS 		64
#define DISPLAY_SLEEP 		0.1e9

#define DISPLAY_HEADER_FMT 	"| %-15s | %-15s | %-15s | %-15s | %-5s |\n"
#define DISPLAY_FMT 		"| %-15s | %-15s | %-15s | %-15s | %-5d |\n"
#define DISPLAY_SEP 		"+-------------------------------------------------------------------------------+\n"
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   M A C R O S
 */
/**
 *	\def		CHECK(sts, msg)
 *	\brief		Macro-fonction qui vérifie que sts est égal -1 (cas d'erreur : sts==-1) 
 *				En cas d'erreur, il y a affichage du message adéquat et fin d'exécution  
 */
#define CHECK(sts, msg) if ((sts)==-1) {perror(msg); exit(-1);}
/**
 *	\def		PAUSE(msg)
 *	\brief		Macro-fonction qui affiche msg et attend une entrée clavier  
 */
#define PAUSE(msg)	printf("%s [Appuyez sur entrée pour continuer]", msg); getchar();
/*
*****************************************************************************************
 *	\noop		D E C L A R A T I O N   DES   V A R I A B L E S    G L O B A L E S
 */
 /**
 *	\var		progName
 *	\brief		Nom de l'exécutable : libnet nécessite cette variable qui pointe sur argv[0]
 */
char 			*progName;

socket_t 		sockEcoute;		// socket d'écoute de demande de connexion d'un client

clientInfo_t 	clients[MAX_CLIENTS];
int 			currentClient = 0;

pthread_t 		displayThread;
int 			startDisplay = 0;

volatile sig_atomic_t stopServer = 0;

/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */


void bye() {

	// Fermer la socket d'écoute
	CHECK(close(sockEcoute.fd), "-- PB close() --");

	printf("Goodbye.\n");

}


void onSignal(int code) {

	stopServer = code == SIGINT;

}


void initServer() {

	atexit(bye);

	struct sigaction sa;
	CHECK(sigemptyset(&sa.sa_mask), "sigemptyset()");
	sa.sa_handler 	= onSignal;
	sa.sa_flags 	= 0;
	CHECK(sigaction(SIGINT, &sa, NULL), "sigaction();");

}


int canAccept() {

	return currentClient < MAX_CLIENTS;

}


void updateCurrentClient(int *current) {

	for (int i = 0; i < MAX_CLIENTS; i++) {

		if (clients[i].status == DISCONNECTED) {

			*current = i;
			return;

		}

	}

	*current = MAX_CLIENTS + 1;

}


void disconnectClient(int id) {

	updateCurrentClient(&currentClient);

}


void displayClient() {

	struct timespec ts;
	ts.tv_sec 	= 0;
	ts.tv_nsec  = DISPLAY_SLEEP;

	while (1) {

		if (!startDisplay) {

			nanosleep(&ts, NULL);
			continue;

		}

		printf(DISPLAY_SEP);
		printf(DISPLAY_HEADER_FMT, "NAME", "STATUS", "ROLE", "ADDRESS", "PORT");
		printf(DISPLAY_SEP);

		for (int i = 0; i < MAX_CLIENTS; i++) {

			char *status;
			char *role;

			switch (clients[i].status) {

				case DISCONNECTED: 	continue;
				case CONNECTING: 	status 		= "CONNECTING"; break;
				case CONNECTED: 	status 		= "CONNECTED"; 	break;

			}

			switch (clients[i].role) {

				case PLAYER: 		role 	= "PLAYER"; break;
				case HOST: 			role 	= "HOST"; 	break;

			}

			printf(DISPLAY_FMT, clients[i].name, status, role, clients[i].address, clients[i].port);

		}

		printf(DISPLAY_SEP);
	
		nanosleep(&ts, NULL);

		system("clear");

	}
	

}


/**
 *	\fn				void serveur (char *adrIP, int port)
 *	\brief			lance un serveur STREAM en écoute sur l'adresse applicative adrIP:port
 *	\param 			adrIP : adresse IP du serveur à metrre en écoute
 *	\param 			port : port d'écoute
 */
void serveur (char *adrIP, int port) {

	initServer();

	pthread_create(&displayThread, 0, (void*)(void*) displayClient, NULL);
	pthread_detach(displayThread);
	
	// sockEcoute est une variable externe
	sockEcoute = creerSocketEcoute(adrIP, port);

	while (1) {

		pthread_t 			thread;
		eServThreadParams_t *params;
		socket_t 			*sockDial;	// socket de dialogue avec un client	

		sockDial 	= malloc(sizeof(socket_t));
		params 		= malloc(sizeof(eServThreadParams_t));

		// Accepter une connexion
		*sockDial 	= accepterClt(sockEcoute);

		startDisplay				= 1;

		params->id 					= currentClient;
		params->sockDial 			= sockDial;
		params->clientArray 		= clients;
		params->clientAmount		= MAX_CLIENTS;
		params->terminationCallback = disconnectClient;
		params->canAccept			= canAccept;

		if (canAccept())
			clients[currentClient].status = CONNECTING;

		pthread_create(&thread, 0, (void*)(void*) dialSrvE2Clt, params);
		pthread_detach(thread);

		updateCurrentClient(&currentClient);

	}


}


int main(int argc, char **argv) {

	progName = argv[0];

	if (argc<3) {
		fprintf(stderr, "usage: %s @IP port\n", basename(progName));
		/*exit(-1);*/
		fprintf(stderr,"lancement du serveur [PID:%d] sur l'adresse applicative [%s:%d]\n",
			getpid(), IP_ANY, PORT_SRV);
		serveur(IP_ANY, PORT_SRV);
	}
	else {
		fprintf(stderr,"lancement du serveur [PID:%d] sur l'adresse applicative [%s:%d]\n",
			getpid(), argv[1], atoi(argv[2]));
		serveur(argv[1], atoi(argv[2]));
	}

}