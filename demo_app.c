/**
 *	\file		demo_app.c
 *	\brief		Exemple d'utilisation de la librairie libinet.a
 *	\author		ARCELON Louis
 *	\date		28 janvier 2026
 *	\version	1.0
 */
#include <libgen.h>
#include <logging.h>
#include <data.h>
#include <repReq.h>
#include <dial.h>
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
char *progName;
/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */
/**
 *	\fn			void client (char *adrIP, int port)
 *	\brief		lance un client STREAM connecté à l'adresse applicative adrIP:port 
 *	\param 		adrIP : adresse IP du serveur à connecter
 *	\param 		port : port du serveur à connecter
 */
void client (char *adrIP, int port) {
	socket_t sockAppel;	// socket d'appel

	printf("Demande de connexion: ");

	// Créer une connexion avec le serveur
	sockAppel = connecterClt2Srv (adrIP, port);

	dialClt2Srv(&sockAppel);

	PAUSE("Fin du client");
	// Fermer la socket d'appel
	CHECK(shutdown(sockAppel.fd, SHUT_WR),"-- PB shutdown() --");

	
}
/**
 *	\fn				void serveur (char *adrIP, int port)
 *	\brief			lance un serveur STREAM en écoute sur l'adresse applicative adrIP:port
 *	\param 			adrIP : adresse IP du serveur à metrre en écoute
 *	\param 			port : port d'écoute
 */
void serveur (char *adrIP, int port) {
	socket_t sockEcoute;	// socket d'écoute de demande de connexion d'un client
	socket_t sockDial;		// socket de dialogue avec un client
	buffer_t buff;
	
	// sockEcoute est une variable externe
	sockEcoute = creerSocketEcoute(adrIP, port);

	// Accepter une connexion
	sockDial = accepterClt(sockEcoute);
	
	dialSrv2Clt(&sockDial);

	// Fermer la socket d'écoute
	CHECK(close(sockDial.fd),"-- PB close() --");
	// Fermer la socket d'écoute
	CHECK(close(sockEcoute.fd),"-- PB close() --");
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

#ifdef CLIENT
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
#endif

#ifdef SERVER
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
#endif	
	return 0;
}
