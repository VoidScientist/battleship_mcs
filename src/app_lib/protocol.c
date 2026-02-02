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
/**
 * @brief      convertis l'enum statusRange_t et action_t vers un code de status int
 *
 * @param[in]  type    l'intervalle de status
 * @param[in]  action  l'action
 *
 * @return     entier représentant le status ex: (REQ, CONNECT) => 101
 */
short enum2status(statusRange_t type, action_t action) {

	return (type + 1) * 100 + action + 1;

}
/**
 * @brief      récupère l'intervalle de status à partir d'un code
 *
 * @param[in]  code  le status
 *
 * @return     le statusRange_t correspondant à l'intervalle de status du code
 */
statusRange_t getStatusRange(short code) {

	switch (code / 100) {

		case 1: return REQ;
		case 2: return ACK;
		case 3: return ERR;

	}

}
/**
 * @brief      récupère l'action à partir d'un code de status
 *
 * @param[in]  code     code de status
 *
 * @return     l'action_t représentée par le code
 */
action_t getAction(short code) {

	switch (code % 100) {

		case 1: return CONNECT;
		case 2: return CELL;
		case 3: return GAME;
		case 4: return CURRENT_PLAYER;

	}

}