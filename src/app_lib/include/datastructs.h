/**
 *	\file		datastructs.h
 *	\brief		Fichier en-tête représentant les structures de données d'échange
 *	\author		ARCELON Louis
 *	\date		31 janvier 2026
 *	\version	1.0
 */

#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#define CLIENT_INFO_OUT "%s,%d,%s,%d"
#define CLIENT_INFO_IN "%[^,],%d,%[^,],%d"

#define ADDR_SIZE 15
#define PSEUDO_SIZE 11

typedef enum {DISCONNECTED, CONNECTING, CONNECTED} userStatus_t;
typedef enum {PLAYER, HOST} userRole_t;

typedef struct {

	char name[PSEUDO_SIZE];
	userStatus_t status;
	userRole_t role;
	char address[ADDR_SIZE];
	short port;

} clientInfo_t;


void createClientInfo(clientInfo_t *infos, char *name, userRole_t role, char *address, short port);

void clientInfo2str(clientInfo_t *infos, char *str);

void str2clientInfo(char *str, clientInfo_t *infos);

int getHostsAmount(clientInfo_t *clients, int size);


#endif /* DATASTRUCTS_H */