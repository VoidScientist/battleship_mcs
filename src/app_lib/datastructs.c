/**
 *	\file		datastructsc
 *	\brief		Fichier implémentation représentant les structures de données d'échange
 *	\author		ARCELON Louis
 *	\date		28 janvier 2026
 *	\version	1.0
 */
#include <stdio.h>
#include <string.h>
#include "datastructs.h"


void createClientInfo(clientInfo_t *infos, char *name, userRole_t role, char *address, short port){

	strcpy(infos->name, name);
	strcpy(infos->address, address);
	infos->role = role;
	infos->port = port;

}


void clientInfo2str(clientInfo_t *infos, char *str) {

	sprintf(str, CLIENT_INFO_OUT, infos->name, infos->role, infos->address, infos->port);

}

void str2clientInfo(char *str, clientInfo_t *infos) {

	sscanf(str, CLIENT_INFO_IN, infos->name, &infos->role, infos->address, &infos->port);

}


int getHostsAmount(clientInfo_t *clients, int size) {

	int result = 0;

	for (int i = 0; i < size; i++) {

		result += clients[i].role == HOST;

	}

	return result;

}