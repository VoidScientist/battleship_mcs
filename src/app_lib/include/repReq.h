/**
 *	\file		repReq.h
 *	\brief		Spécification de la couche repReq de l'application
 *	\author		Louis ARCELON, Baptiste PICAVET, Mathieu MARTEL
 *	\date		23 janvier 2026
 *	\version	1.0
 */
#ifndef APM_DIAL_H
#define APM_DIAL_H
/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include <string.h>
#include "data.h"
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
#define DATA_LENGTH	100
/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */
/**
 * \struct  request
 * \brief 	définition de la struct de requête
 * \note	- id est le status de la requête
 * 			- verb est le verbe de la requête
 *    		- data représente les données de réponse
 */
typedef struct request {
	
	short id;
	uint8_t verb;
	char data[DATA_LENGTH];
	
	
} req_t;
/**
 * \struct  response
 * \brief 	définition de la struct de réponse
 * \note	- id est le status de la réponse
 *    		- data représente les données de réponse
 */
typedef struct response {
	
	short id;
	char data[DATA_LENGTH];
	
} rep_t;
/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
req_t creerRequete(int status, uint8_t verb, generic data, pFct serial);
rep_t creerReponse(int status, generic data, pFct serial);

void sendRequest(socket_t *sockAppel, int status, uint8_t verb, generic data, pFct serial);
void sendResponse(socket_t *sockDial, int status, generic data, pFct serial);

void rcvRequest(socket_t *sockDial, req_t *request);
void rcvResponse(socket_t *sockAppel, rep_t *response);
/**
 * \brief      fonction de sérialisation des requêtes
 *
 * \param      requete  pointeur de la requête à sérialiser
 * \param      str      buffer de la représentation sérialisée de la requête
 */
void req2str(req_t *requete, char *str);
/**
 * \brief      fonction de désérialisation des requêtes
 *
 * \param      str      buffer à désérialiser
 * \param      requete  pointeur vers la struct à remplir
 */
void str2req(char *str, req_t *requete);
/**
 * \brief      fonction de sérialisation des réponses
 *
 * \param      requete  pointeur de la réponse à sérialiser
 * \param      str      buffer de la représentation sérialisée de la réponse
 */
void rep2str(rep_t *reponse, char *str);
/**
 * \brief      fonction de désérialisation des réponses
 *
 * \param      str      buffer à désérialiser
 * \param      requete  pointeur vers la struct à remplir
 */
void str2rep(char *str, rep_t *reponse);


#endif /* APM_DIAL_H */