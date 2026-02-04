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
/**
 * @brief enum contenant les verbes du protocole
 */
typedef enum {GET, POST, DELETE} verb_t;
/**
 * @brief enum contenant les intervalles de status du protocole
 */
typedef enum {REQ, ACK, ERR} statusRange_t;
/**
 * @brief enum contenant les actions du protocole
 */
typedef enum {CONNECT, PLACE, SHOOT, NEXT_TURN, END_GAME, START_GAME} action_t;

/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
/**
 * @brief      convertis l'enum statusRange_t et action_t vers un code de status int
 *
 * @param[in]  type    l'intervalle de status
 * @param[in]  action  l'action
 *
 * @return     entier représentant le status ex: (REQ, CONNECT) => 101
 */
short enum2status(statusRange_t type, action_t action);
/**
 * @brief      récupère l'intervalle de status à partir d'un code
 *
 * @param[in]  code  le status
 *
 * @return     le statusRange_t correspondant à l'intervalle de status du code
 */
statusRange_t getStatusRange(short code);
/**
 * @brief      récupère l'action à partir d'un code de status
 *
 * @param[in]  code     code de status
 *
 * @return     l'action_t représentée par le code
 */
action_t getAction(short code);

#endif /* PROTOCOL_H */
