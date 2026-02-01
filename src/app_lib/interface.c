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
#include <session.h>
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
            
            if (result == FGETS_ERROR) {
                printf("\nErreur de lecture. Réessayez.\n");
                continue;
            }
            if (result == USE_DEFAULT) {
                printf("Un port est requis!\n");
                continue;
            }
            if (result == VSSCANF_ERROR) {
                printf("Format invalide. Entrez un nombre.\n");
                continue;
            }
            
            if (port < 2000 || port > 20000) {
                printf("Le port doit être entre 2000 et 20000.\n");
            }
        } while (port < 2000 || port > 20000);
        

    } 
    
    
    strcpy(infos->name, pseudo);
    getIpAddress(infos->address);

    infos->role 	= role;
    infos->port 	= port;
    
}



void getSrvEAddress(char* adrIP, unsigned short port, char *userIP, short *userPort) {
    int result;
    
    printf("\nConnexion au serveur d'écoute:\n");

    printf("Adresse Applicative (%s:%d): ", adrIP, port);
    
    result = retrieveInput("%[^:]:%hd", userIP, userPort);
    
    if (result == USE_DEFAULT || result == FGETS_ERROR || result == VSSCANF_ERROR) {
        printf("Utilisation des valeurs par défaut...\n");
        strcpy(userIP, adrIP);
        *userPort = port;
        return;
    }
    
    printf("Connexion à %s:%d\n", userIP, *userPort);
}



void displayHosts(clientInfo_t *hosts, int amount) {


	printf("\nCurrent hosts:\n");

	for (int i = 0; i < amount; i++) {

		if (hosts[i].role == HOST)
			printf("\t- %s\n", hosts[i].name);

	}


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