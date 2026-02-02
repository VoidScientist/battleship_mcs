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
/**
 * @brief taille maximum des données d'une réponse / requête en octets
 */
#define DATA_LENGTH	100
/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */
/**
 * \struct  request
 * \brief 	définition de la struct de requête
 */
typedef struct request {
	
	/** status de la requête */
	short id;
	/** verbe de la requête */
	uint8_t verb;
	/** donnée de la requête */
	char data[DATA_LENGTH];
	
} req_t;
/**
 * \struct  response
 * \brief 	définition de la struct de réponse
 */
typedef struct response {
	
	/** status de la réponse */
	short id;
	/** données de la réponse */
	char data[DATA_LENGTH];
	
} rep_t;
/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
/**
 * @brief      crée une struct req_t (requête) à partir des arguments fournis
 *
 * @param[in]  status  code de status
 * @param[in]  verb    verbe de la requête
 * @param[in]  data    données de la requête
 * @param[in]  serial  fonction de sérialisation des données (NULL si char *)
 *
 * @return     la structure remplie avec les informations fournies
 */
req_t creerRequete(int status, uint8_t verb, generic data, pFct serial);
/**
 * @brief      crée une struct rep_t (réponse) à partir des arguments fournis
 *
 * @param[in]  status  code de status
 * @param[in]  data    données de la réponse
 * @param[in]  serial  fonction de sérialisation des données (NULL si char *)
 *
 * @return     la structure remplie avec les informations fournies
 */
rep_t creerReponse(int status, generic data, pFct serial);
/**
 * @brief      Envoyer une requête à partir d'arguments
 *
 * @param      sockAppel  la socket d'appel
 * @param[in]  status     le status de la requête
 * @param[in]  verb       le verbe de la requête
 * @param[in]  data       les données de la requête
 * @param[in]  serial     la fonction de sérialisation des données (NULL si char *)
 */
void sendRequest(socket_t *sockAppel, int status, uint8_t verb, generic data, pFct serial);
/**
 * @brief      Envoyer une réponse à partir d'arguments
 *
 * @param      sockDial  la socket de dialogue
 * @param[in]  status    le status de la réponse
 * @param[in]  data      les données de la réponse
 * @param[in]  serial    la fonction de sérialisation des données (NULL si char *)
 */
void sendResponse(socket_t *sockDial, int status, generic data, pFct serial);
/**
 * @brief      Recevoir une requête
 *
 * @param      sockDial  socket de dialogue
 * @param      request   pointeur vers la struct request à remplir
 */
void rcvRequest(socket_t *sockDial, req_t *request);
/**
 * @brief      Recevoir une réponse
 *
 * @param      sockAppel  la socket d'appel
 * @param      response   pointeur vers la struct rep_t à remplir
 */
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
 * \param      reponse  pointeur de la réponse à sérialiser
 * \param      str      buffer de la représentation sérialisée de la réponse
 */
void rep2str(rep_t *reponse, char *str);
/**
 * \brief      fonction de désérialisation des réponses
 *
 * \param      str      buffer à désérialiser
 * \param      reponse  pointeur vers la struct à remplir
 */
void str2rep(char *str, rep_t *reponse);


#endif /* APM_DIAL_H */