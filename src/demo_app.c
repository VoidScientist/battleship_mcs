/**
 *	\file		demo_app.c
 *	\brief		Exemple d'utilisation de la librairie libinet.a
 *	\author		ARCELON Louis
 *	\date		28 janvier 2026
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

#define USER_BUFFER_SIZE 100
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

clientInfo_t 	self;
/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

void onSignal(int code) {

	mustDisconnect = code == SIGINT;

}


void initClient() {

	struct sigaction sa;
	CHECK(sigemptyset(&sa.sa_mask), "sigemptyset()");
	sa.sa_handler 	= onSignal;
	sa.sa_flags 	= 0;
	CHECK(sigaction(SIGINT, &sa, NULL), "sigaction();");


	CHECK(sem_init(&semCanClose, 0, 0), "sem_init()"); 

}


void setupUserInfos() {

	char 		pseudo[PSEUDO_SIZE];
	userRole_t 	role;
	char *		fgetsResult;
	int 		scanfResult;

	printf("\nConfiguration du profil utilisateur:\n");
	
	do {

		printf("Nom d'utilisateur (3-10 chars): ");
		fgetsResult = fgets(pseudo, PSEUDO_SIZE, stdin);
		sscanf(pseudo, "%[^\n]\n", pseudo);
		
		if (fgetsResult == NULL) {
			printf("\nDon't CTRL+D please...\n");
			clearerr(stdin);
		}

	} while (strlen(pseudo) < 3);

	strcpy(self.name, pseudo);

	do {
		printf("Role (0: PLAYER, 1: HOST): ");
		scanfResult = scanf("%d", &role);

		if (scanfResult == EOF) {
			printf("\nDon't CTRL+D please...\n");
			clearerr(stdin);
		}

	} while (role != PLAYER && role != HOST);

	if (role == HOST) {

		strcpy(self.address, "0.0.0.0");
		self.port = 4000;

	}

	self.role = HOST;

}


void getSrvEAddress(char* adrIP, unsigned short port, char *userIP, short *userPort) {

	char 	buffer[USER_BUFFER_SIZE];
	char	*fgetsResult;
	int 	matched;

	printf("\nConnexion au serveur d'écoute:\n");
	printf("Adresse Applicative (%s:%d): ", adrIP, port);
	

	fgetsResult = fgets(buffer, USER_BUFFER_SIZE, stdin);

	if (fgetsResult == NULL) {
		printf("\nErreur de lecture, utilisation des valeurs par défaut...\n");
		strcpy(userIP, adrIP);
		*userPort = port;
		return;
	}

	if (buffer[0] == '\n') {
		printf("Utilisation des valeurs par défaut...\n");
		strcpy(userIP, adrIP);
		*userPort = port;
		return;
	}

	matched = sscanf(buffer, "%[^:]:%hd\n", userIP, userPort);

	if (matched != 2) {
		printf("Mauvais format, utilisation des valeurs par défaut...\n");
		strcpy(userIP, adrIP);
		*userPort = port;
		return;
	}

}

/**
 *	\fn			void client (char *adrIP, int port)
 *	\brief		lance un client STREAM connecté à l'adresse applicative adrIP:port 
 *	\param 		adrIP : adresse IP du serveur à connecter
 *	\param 		port : port du serveur à connecter
 */
void client (char *adrIP, unsigned short port) {

	char 				userIP[USER_BUFFER_SIZE];
	short				userPort;
	eCltThreadParams_t	*params;

	initClient();

	getSrvEAddress(adrIP, port, userIP, &userPort);

	setupUserInfos();

	// Créer une connexion avec le serveur
	sockAppel = connecterClt2Srv (userIP, userPort);




	params 					= malloc(sizeof(params));
	params->sockAppel 		= &sockAppel;
	params->infos 			= &self;
	params->semCanClose		= &semCanClose;

	pthread_create(&dialServE, 0, (void*)(void *) dialClt2SrvE, params);










	if (sem_wait(&semCanClose) == -1 && errno != EINTR) {
		perror("sem_wait()");
		exit(EXIT_FAILURE);
	}

	// Fermer la socket d'appel
	CHECK(shutdown(sockAppel.fd, SHUT_WR),"-- PB shutdown() --");

	
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
