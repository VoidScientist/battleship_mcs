/**
 *	\file		protocol.h
 *	\brief		Fichier en-tête représentant notre protocol applicatif
 *	\author		ARCELON Louis
 *	\date		29 janvier 2026
 *	\version	1.0
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H
/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */
typedef enum {GET, POST, DELETE} verb_t;

typedef enum {REQ, ACK, ERR} statusRange_t;

typedef enum {CONNECT, CELL, GAME, CURRENT_PLAYER} action_t;

/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */

short enum2status(statusRange_t type, action_t action);

statusRange_t getStatusRange(short code);

action_t getAction(short code);

#endif /* PROTOCOL_H */