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
#include "data.h"
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
#define OPT_LENGTH	100
/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */
/**
 * \struct  request
 * \brief 	définition de la struct de requête
 * \note	- idReq est le status de la requête
 * 			- verbReq est le verbe de la requête
 *    		- optReq est le contenu de la requête
 */
typedef struct request {
	
	short id;
	uint8_t verb[VERB_LENGTH];
	char opt[OPT_LENGTH];
	
	
} req_t;
/**
 * \struct  response
 * \brief 	définition de la struct de réponse
 * \note	- idRep est le status de la réponse
 * 			- verbRep est le verbe de la réponse
 *    		- optRep est le contenu de la réponse
 */
typedef struct response {
	
	short id;
	uint8_t verb;
	char opt[OPT_LENGTH];
	
} rep_t;
/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
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