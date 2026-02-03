/**
 *	\file		protocol.c
 *	\brief		Fichier implémentation représentant notre protocol applicatif
 *	\author		ARCELON Louis
 *	\date		29 janvier 2026
 *	\version	1.0
 */
#include "protocol.h"
/*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

short enum2status(statusRange_t type, action_t action) {

	return (type + 1) * 100 + action + 1;

}

statusRange_t getStatusRange(short code) {

	switch (code / 100) {

		case 1: return REQ;
		case 2: return ACK;
		case 3: return ERR;
		default: return ERR;
	}

}

action_t getAction(short code) {
	switch (code % 100) {
		case 1: return CONNECT;
		case 2: return CELL;
		case 3: return GAME;
		case 4: return CURRENT_PLAYER;
		case 5: return PLACE;
		case 6: return SHOOT;
		case 7: return NEXT_TURN;
		case 8: return END_GAME;
		case 9: return START_GAME;
		default: return CONNECT;

	}
}
