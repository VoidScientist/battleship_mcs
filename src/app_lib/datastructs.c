/**
 *	\file		datastructs.c
 *	\brief		Fichier implémentation représentant les structures de données d'échange
 *	\author		ARCELON Louis
 *	\date		28 janvier 2026
 *	\version	1.0
 */
#include <stdio.h>
#include <string.h>
#include "datastructs.h"
/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */
/**
 * @brief      Crée les informations d'un client à partir d'arguments
 *
 * @param      infos    La structure à remplir
 * @param      name     Le nom d'utilisateur
 * @param[in]  role     Le role
 * @param      address  L'adresse LAN du client
 * @param[in]  port     Le port du client
 */
void createClientInfo(clientInfo_t *infos, char *name, userRole_t role, char *address, short port){

	strcpy(infos->name, name);
	strcpy(infos->address, address);
	infos->role = role;
	infos->port = port;

}
/**
 * @brief      fonction de sérialisation des infos clients
 *
 * @param      infos  les infos
 * @param      str    le buffer sérialisé
 */
void clientInfo2str(clientInfo_t *infos, char *str) {

	sprintf(str, CLIENT_INFO_OUT, infos->name, infos->role, infos->address, infos->port);

}
/**
 * @brief      fonction de désérialisation des infos clients
 *
 * @param      str    le buffer sérialisé
 * @param      infos  les infos désérialisées
 */
void str2clientInfo(char *str, clientInfo_t *infos) {

	sscanf(str, CLIENT_INFO_IN, infos->name, &infos->role, infos->address, &infos->port);

}
/**
 * @brief      Récupère dans une list d'infos clients le nombre d'hôtes
 *
 * @param      clients  les clients
 * @param[in]  size     la taille du tableau
 *
 * @return     le nombre d'hôtes parmis les clients
 */
int getHostsAmount(clientInfo_t *clients, int size) {

	int result = 0;

	for (int i = 0; i < size; i++) {

		result += clients[i].role == HOST;

	}

	return result;

}