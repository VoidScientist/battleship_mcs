/**
 *	\file		repReq.c
 *	\brief		Implémentation de la couche repReq de l'application
 *	\author		Louis ARCELON, Baptiste PICAVET, Mathieu MARTEL
 *	\date		23 janvier 2026
 *	\version	1.0
 */
#include "logging.h"
#include "data.h"
#include "repReq.h"
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
/**
 * @brief format de sérialisation des requêtes
 */
#define REQ_STR_OUT "%i:%hhu:%s"
/**
 * @brief format de désérialisation des requêtes
 */
#define REQ_STR_IN "%i:%hhu:%[^\n]"
/**
 * @brief format de sérialisation des réponses
 */
#define REP_STR_OUT "%i:%s"
/**
 * @brief format de désérialisation des réponses
 */
#define REP_STR_IN "%i:%[^\n]"
/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
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
req_t creerRequete(int status, uint8_t verb, generic data, pFct serial) {
	req_t request;

	request.id = status;
	request.verb = verb;

	if (serial == NULL) {
		strcpy(request.data, data);
	} else {
		serial(data, request.data);
	}

	return request;
}
/**
 * @brief      crée une struct rep_t (réponse) à partir des arguments fournis
 *
 * @param[in]  status  code de status
 * @param[in]  data    données de la réponse
 * @param[in]  serial  fonction de sérialisation des données (NULL si char *)
 *
 * @return     la structure remplie avec les informations fournies
 */
rep_t creerReponse(int status, generic data, pFct serial) {
	rep_t response;
	
	response.id = status;

	if (serial == NULL) {
		strcpy(response.data, data);
	} else {
		serial(data, response.data);
	}

	return response;
}
/**
 * \brief      fonction de sérialisation des requêtes
 *
 * \param      req  pointeur de la requête à sérialiser
 * \param      str      buffer de la représentation sérialisée de la requête
 */
void req2str(req_t *req, char *str) {
	sprintf(str, REQ_STR_OUT, req->id, req->verb, req->data);
	
}
/**
 * \brief      fonction de désérialisation des requêtes
 *
 * \param      str      buffer à désérialiser
 * \param      req  pointeur vers la struct à remplir
 */
void str2req(char *str, req_t *req) {
	sscanf(str, REQ_STR_IN, &req->id, &req->verb, &req->data);
	
}
/**
 * \brief      fonction de sérialisation des réponses
 *
 * \param      rep  pointeur de la réponse à sérialiser
 * \param      str      buffer de la représentation sérialisée de la réponse
 */
void rep2str(rep_t *rep, char *str) {
	sprintf(str, REP_STR_OUT, rep->id, rep->data);
	
}
/**
 * \brief      fonction de désérialisation des réponses
 *
 * \param      str      buffer à désérialiser
 * \param      rep  pointeur vers la struct à remplir
 */
void str2rep(char *str, rep_t *rep) {
	sscanf(str, REP_STR_IN, &rep->id, &rep->data);
	
}
/**
 * @brief      Envoyer une requête à partir d'arguments
 *
 * @param      sockAppel  la socket d'appel
 * @param[in]  status     le status de la requête
 * @param[in]  verb       le verbe de la requête
 * @param[in]  data       les données de la requête
 * @param[in]  serial     la fonction de sérialisation des données (NULL si char *)
 */
void sendRequest(socket_t *sockAppel, int status, uint8_t verb, generic data, pFct serial) {
	req_t request = creerRequete(status, verb, data, serial);

	envoyer(sockAppel,(generic) &request, (pFct) req2str);
}
/**
 * @brief      Envoyer une réponse à partir d'arguments
 *
 * @param      sockDial  la socket de dialogue
 * @param[in]  status    le status de la réponse
 * @param[in]  data      les données de la réponse
 * @param[in]  serial    la fonction de sérialisation des données (NULL si char *)
 */
void sendResponse(socket_t *sockDial, int status, generic data, pFct serial) {
	rep_t response = creerReponse(status, data, serial);

	envoyer(sockDial, (generic) &response, (pFct) rep2str);
}
/**
 * @brief      Recevoir une requête
 *
 * @param      sockDial  socket de dialogue
 * @param      request   pointeur vers la struct request à remplir
 */
void rcvRequest(socket_t *sockDial, req_t *request) {

	recevoir(sockDial,(generic) request, (pFct) str2req);

#ifdef DEBUG_ENABLED
	logMessage("[%i] %hhu : %s\n"
		, DEBUG
		, request->id
		, request->verb
		, request->data
	);
#endif
}
/**
 * @brief      Recevoir une réponse
 *
 * @param      sockAppel  la socket d'appel
 * @param      response   pointeur vers la struct rep_t à remplir
 */
void rcvResponse(socket_t *sockAppel, rep_t *response) {
	recevoir(sockAppel,(generic) response, (pFct) str2rep);

#ifdef DEBUG_ENABLED
	logMessage("[%i] %s\n"
		, DEBUG
		, response->id
		, response->data
	);
#endif
}
