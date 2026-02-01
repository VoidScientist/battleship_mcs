/**
 *	\file		interface.c
 *	\brief		fichier implémentation utilisé pour interagir avec stdin
 *				et faire un affichage dans le terminal
 *	\author		ARCELON Louis
 *	\date		1 février 2026
 *	\version	1.0
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <semaphore.h>

#include <session.h>

#include "dial.h"
#include "interface.h"


void setupUserInfos(clientInfo_t *infos) {

    int                 result;

    char                pseudo[PSEUDO_SIZE]	= "";
    userRole_t          role 				= -1;
    unsigned short      port 				= 0;
    
    printf("\nConfiguration du profil utilisateur:\n");
    
    do {
        printf("Nom d'utilisateur (3-10 chars): ");
        result = retrieveInput("%s", pseudo);

        switch (result) {

        	case FGETS_ERROR: 
        		printf("\nErreur de lecture. Arrêt.\n");
        		exit(EXIT_FAILURE);

        	case USE_DEFAULT:
        		printf("Un nom est requis!\n");
        		continue;

        	case VSSCANF_ERROR:
        		printf("Format invalide. Réessayez.\n");
        		continue;

        }

    } while (strlen(pseudo) < 3 || strlen(pseudo) > 10);
    
    
    
    do {
        printf("\nRole (0: PLAYER, 1: HOST): ");
        result = retrieveInput("%d", &role);

        switch (result) {

        	case FGETS_ERROR: 
        		printf("\nErreur de lecture. Arrêt.\n");
        		exit(EXIT_FAILURE);

        	case USE_DEFAULT:
        		printf("Un rôle est requis!\n");
        		continue;

        	case VSSCANF_ERROR:
        		printf("Format invalide. Entrez 0 ou 1.\n");
        		continue;

        }
        
        
        if (role != PLAYER && role != HOST) {

            printf("Rôle invalide. Entrez 0 (PLAYER) ou 1 (HOST).\n");

        }

    } while (role != PLAYER && role != HOST);
    
    
    if (role == HOST) {
        do {
            printf("\nPort (2000-20000): ");
            result = retrieveInput("%hu", &port);

            switch (result) {

	        	case FGETS_ERROR: 
	        		printf("\nErreur de lecture. Arrêt.\n");
	        		exit(EXIT_FAILURE);

	        	case USE_DEFAULT:
	        		printf("Port par défaut choisi: %d\n", DEFAULT_PORT);
	                port = DEFAULT_PORT;
	        		continue;

	        	case VSSCANF_ERROR:
	        		printf("Format invalide. Entrez un nombre.\n");
	        		continue;

       		 }

	        if (port < 2000 || port > 20000) {
	            printf("Le port doit être entre 2000 et 20000.\n");
	        }
        
        } while (port < 2000 || port > 20000);
        

    } 
    
    strncpy(infos->name, pseudo, PSEUDO_SIZE);
    getIpAddress(infos->address);

    infos->role 	= role;
    infos->port 	= port;
    
}



void getSrvEAddress(char* adrIP, unsigned short port, char *userIP, short *userPort) {
    int result;
    
    printf("\nConnexion au serveur d'enregistrement:\n");

    printf("Adresse Applicative (%s:%d): ", adrIP, port);
    
    result = retrieveInput("%[^:]:%hd", userIP, userPort);
    
    if (result != STEP_SUCCESS) {
        printf("Utilisation des valeurs par défaut...\n");
        strcpy(userIP, adrIP);
        *userPort = port;
        return;
    }
    
    printf("Connexion à %s:%d\n", userIP, *userPort);
}




int retrieveInput(char *fmt, ...) {

	char 		buffer[INPUT_BUFFER_SIZE];
	int 		result;
	va_list		args;


	va_start(args, fmt);

	result = saferFgets(buffer, INPUT_BUFFER_SIZE);

	if (result != STEP_SUCCESS) {
		va_end(args);
		return result;
	}

	result = saferVsscanf(buffer, fmt, args);

	va_end(args);

	if (result != STEP_SUCCESS) return result;

	return STEP_SUCCESS;

}



int calculateExpectedFromFmt(char *fmt, int size) {

	int 	amount	= 0;


	if (fmt[size - 1] != '\0') return EXPECT_ERROR;


	for (int i = 0; i < size - 1; i++) {
		
		if (fmt[i] == '%') {

			if (fmt[i+1] == '%') i++;
			
			else 			amount++;
		
		}
	}

	return amount;

}



int saferVsscanf(char *buff, char *fmt, va_list args) {

	int 	expected, matched;


	expected 	= calculateExpectedFromFmt(fmt, strlen(fmt)+1);
	matched 	= vsscanf(buff, fmt, args);

	if (matched != expected) return VSSCANF_ERROR;


	return STEP_SUCCESS;

}





int saferFgets(char *buffer, int size) {

	char 	*readResult;


	readResult = fgets(buffer, size, stdin);


	if (readResult == NULL) {
		return FGETS_ERROR;
	}

	if (buffer[0] == '\n') {
		return USE_DEFAULT;
	}


	return STEP_SUCCESS;

}



void displayHosts(clientInfo_t *hosts, int amount) {

	if (getHostsAmount(hosts, amount) == 0) return;

	printf("\n╔===[Hôtes disponibles]===╗\n");
	printf("║                         ║\n");

	for (int i = 0; i < amount; i++) {

		if (hosts[i].role == HOST) {
			printf("║    [ %d ]   %-10s   ║\n", i+1, hosts[i].name);
			printf("║                         ║\n");
		}

	}

	printf("╚=========================╝ \n");


}



void displayPlayerMenu(playerMenuParams_t params) {

	int 		running 	= 1;

	menuState_t state 		= MAIN_MENU;


	while (running) {

		switch (state) {

			case MAIN_MENU: displayMainMenu(params, &state); break;
			case JOIN_MENU: displayJoinMenu(params, &state); break;

		}
		

	}

}



void displayMainMenu(playerMenuParams_t params, menuState_t *state) {

	int 	result;
	int 	action  = 0;

	callback	exitProgram = params.exitProgram;

	printf("\n");
	printf("╔=======[BATTLESHIP]=======╗\n");
	printf("║                          ║\n");
	printf("║     [1]    Rejoindre     ║\n");
	printf("║                          ║\n");
	printf("║     [2]    Quitter       ║\n");
	printf("║                          ║\n");
	printf("╚==========================╝\n");

	
	do {

		printf("\nAction: ");
		result = retrieveInput("%d", &action);

		if (result == USE_DEFAULT) {

			continue;

		}

		if (result != STEP_SUCCESS) {

			exitProgram();
			return;

		}

		switch (action) {

			case 1:
				*state = JOIN_MENU;
				break;

			case 2:
				exitProgram();
				return;
		}

	} while (action != 1 && action != 2);

}



void displayJoinMenu(playerMenuParams_t params, menuState_t *state) {

	int 	result;
	int 	maxAct;
	int 	action  = 0;

	callback	exitProgram = params.exitProgram;
	callback	showHosts 	= params.showHosts;

	showHosts();

	maxAct = getHostsAmount(params.hosts, MAX_HOSTS_GET);

	if (maxAct == 0) {

		printf("\nPas d'hôte à rejoindre.\n");

		*state = MAIN_MENU;
		return;

	}

	do {

		printf("\nAction (1-%d): ", maxAct);
		result = retrieveInput("%d", &action);

		if (result == USE_DEFAULT) {

			continue;

		}

		if (result != STEP_SUCCESS) {

			exitProgram();
			return;

		}

		switch (action) {

			default:
				*state = MAIN_MENU;
				break;
		}

	} while (action <= 0 || action > maxAct);

}