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
#define REQ_STR_OUT "%i:%hhu:%s"
#define REQ_STR_IN "%i:%hhu:%[^\n]"
#define REP_STR_OUT "%i:%s"
#define REP_STR_IN "%i:%[^\n]"
/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
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

void req2str(req_t *req, char *str) {
	sprintf(str, REQ_STR_OUT, req->id, req->verb, req->data);
	
}


void str2req(char *str, req_t *req) {
	sscanf(str, REQ_STR_IN, &req->id, &req->verb, &req->data);
	
}


void rep2str(rep_t *rep, char *str) {
	sprintf(str, REP_STR_OUT, rep->id, rep->data);
	
}


void str2rep(char *str, rep_t *rep) {
	sscanf(str, REQ_STR_IN, &rep->id, &rep->data);
	
}


void sendRequest(socket_t *sockAppel, int status, uint8_t verb, generic data, pFct serial) {
	req_t request = creerRequete(status, verb, data, serial);

	envoyer(sockAppel,(generic) &request, (pFct) req2str);
}


void sendResponse(socket_t *sockDial, int status, generic data, pFct serial) {
	rep_t response = creerReponse(status, data, serial);

	envoyer(sockDial, (generic) &response, (pFct) rep2str);
}


void rcvRequest(socket_t *sockDial, req_t *request) {

	recevoir(sockDial,(generic) request, (pFct) str2req);

	logMessage("[%i] %hhu : %s\n"
		, DEBUG
		, request->id
		, request->verb
		, request->data
	);
}


void rcvResponse(socket_t *sockAppel, rep_t *response) {
	recevoir(sockAppel,(generic) response, (pFct) str2rep);

	logMessage("[%i] %s\n"
		, DEBUG
		, response->id
		, response->data
	);
}
