/**
 *	\file		dial.h
 *	\brief		Fichier en-tête représentant les dialogues applicatifs
 *	\author		ARCELON Louis
 *	\date		28 janvier 2026
 *	\version	1.0
 */
#ifndef DIAL_H
#define DIAL_H
/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include "data.h"
#include "repReq.h"
/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
/**
 * \fn 			dialClt2Srv()
 * \brief       fonction s'occupant du dialogue entre le client et le serveur
 * 
 * \param		sockAppel		structure socket_t contenant le descripteur
 * 								de fichier de la socket d'appel
 * 
 * \note 		s'occupe donc de l'envoi de requêtes et réception de réponses
 */
void dialClt2Srv(socket_t *sockAppel);
/**
 * \fn 			dialSrv2Clt()
 * \brief       fonction s'occupant du dialogue entre le serveur est le client
 * 
 * \param		sockDial		structure socket_t contenant les informations
 * 								sur la socket de dialogue avec le client
 * 
 * \note		s'occupe donc de l'envoi de réponses et réception de réponses
 */
void dialSrv2Clt(socket_t *sockDial);



#endif /* DIAL_H */