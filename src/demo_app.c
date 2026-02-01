/**
 *	\file		demo_app.c
 *	\brief		Client de l'application
 *	\author		ARCELON Louis
 *	\date		1 février 2026
 *	\version	1.0
 */
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include <dial.h>
#include <datastructs.h>
#include <interface.h>
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
#define PORT_SRV	50000
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
pthread_t 		dialServE;

socket_t 		sockAppel;	// socket d'appel

sem_t			semCanClose;
sem_t 			semRequestFin;

clientInfo_t 	self;
clientInfo_t	hosts[MAX_HOSTS_GET];
/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */


void onSignal(int code) {

	mustDisconnect = code == SIGINT;

}


void onExit() {

	int result;


	mustDisconnect = 1;

	// s'assure qu'on ait bien pu fermer la connexion
	// avant de fermer le client
	do {
		result = sem_wait(&semCanClose);
	} while (result == -1 && errno == EINTR);

	// Fermer la socket d'appel
	CHECK(shutdown(sockAppel.fd, SHUT_WR),"-- PB shutdown() --");


	exit(EXIT_SUCCESS);

}


void onDisplayHosts() {

	postRequest(&requestHosts, &semRequestFin);
	displayHosts(hosts, MAX_HOSTS_GET);

}



void initClient() {

	struct sigaction sa;
	CHECK(sigemptyset(&sa.sa_mask), "sigemptyset()");
	sa.sa_handler 	= onSignal;
	sa.sa_flags 	= 0;
	CHECK(sigaction(SIGINT, &sa, NULL), "sigaction();");


	CHECK(sem_init(&semCanClose, 0, 0), "sem_init()");
	CHECK(sem_init(&semRequestFin, 0, 0), "sem_init()");

	// initialise les hôtes avec des valeurs pour éviter
	// de lire n'importe quoi.
	for (int i = 0; i < MAX_HOSTS_GET; i++) {
		createClientInfo(&hosts[i], "", 0, "", 0);
		hosts[i].status 	= DISCONNECTED;
	}

}


/**
 *	\fn			void client (char *adrIP, int port)
 *	\brief		lance un client STREAM connecté à l'adresse applicative adrIP:port 
 *	\param 		adrIP : adresse IP du serveur à connecter
 *	\param 		port : port du serveur à connecter
 */
void client(char *adrIP, unsigned short port) {

	char 				userIP[USER_BUFFER_SIZE];
	short				userPort;
	eCltThreadParams_t	*params;
	playerMenuParams_t	menuParams;
	

	initClient();

	getSrvEAddress(adrIP, port, userIP, &userPort);

	setupUserInfos(&self);

	// Créer une connexion avec le serveur
	sockAppel = connecterClt2Srv (userIP, userPort);


	params 					= malloc(sizeof(eCltThreadParams_t));
	params->sockAppel 		= &sockAppel;
	params->infos 			= &self;
	params->hostBuffer		= hosts;
	params->semCanClose		= &semCanClose;
	params->semRequestFin 	= &semRequestFin;

	pthread_create(&dialServE, 0, (void*)(void *) dialClt2SrvE, params);
	

	menuParams.showHosts	= onDisplayHosts;
	menuParams.exitProgram	= onExit;

	displayPlayerMenu(&menuParams);




	onExit();

	
}

/**
 *	\fn				int main(int argc, char** argv)
 *	\brief			Programme principal d'un serveur STREAM ou d'un client STREAM
 *					avec un protocole simple : envoi d'un message/réception d'un message
 *	\note			La compilation se fait avec -DCLIENT pour générer un client et -DSERVER pour un serveur
 *	\param 			argc : nombre d'aguments de la ligne de commande
 *	\param 			argv : arguments de la commande en ligne
 *					Cas d'un serveur : adresse IP du serveur à metrre en écoute & port d'écoute
 *					Cas d'un client : adresse IP du serveur à connecter & port d'écoute du serveur
 */
int main(int argc, char** argv) {
	progName = argv[0];


	if (argc<3) {
		fprintf(stderr,"usage : %s @IP port\n", basename(progName));
		 /*exit(-1);*/ 
		fprintf(stderr,"lancement du client [PID:%d] connecté à l'adresse applicative [%s:%d]\n", 
				getpid(), IP_ANY, PORT_SRV);
		client(IP_ANY, PORT_SRV);
	}
	else {
		fprintf(stderr,"lancement du client [PID:%d] connecté à l'adresse applicative [%s:%d]\n",
			getpid(), argv[1], atoi(argv[2]));
		client(argv[1], atoi(argv[2]));
	}


	return 0;
}
