/**
 *	\file		repReq.c
 *	\brief		Implémentation de la couche repReq de l'application
 *	\author		Louis ARCELON, Baptiste PICAVET, Mathieu MARTEL
 *	\date		23 janvier 2026
 *	\version	1.0
 */
#include "repReq.h"
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
#define REQ_STR_OUT "%i:%s:%s"
#define REQ_STR_IN "%i:%hhu:%[^\n]"
#define REP_STR_OUT "%i:%s:%s"
#define REP_STR_IN "%i:%hhu:%[^\n]"
/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */
void req2str(req_t *req, char *str) {
	sprintf(str, REQ_STR_OUT, req->id, req->verb, req->opt);
	
}


void str2req(char *str, req_t *req) {
	sscanf(str, REQ_STR_IN, &req->id, &req->verb, &req->opt);
	
}


void rep2str(rep_t *rep, char *str) {
	sprintf(str, REP_STR_OUT, rep->id, rep->verb, rep->opt);
	
}


void str2rep(char *str, rep_t *rep) {
	sscanf(str, REQ_STR_IN, &rep->id, &rep->verb, &rep->opt);
	
}
