/**
 *	\file		logging.h
 *	\brief		en-tête de la lib logging
 *	\author		ARCELON Louis
 *	\date		7 janvier 2026
 *	\version	1.0
 */
#ifndef ARC_LOGGING_H
#define ARC_LOGGING_H
/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
 #include <stdio.h>
 #include <stdarg.h>
/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */
/**
 * \enum		LOG_LEVEL
 * \brief		Définition de l'enum LOG_LEVEL
 * \note		Prends les valeurs DEBUG, WARNING, ERROR, INSTR
 */
enum LOG_LEVEL {DEBUG, WARNING, ERROR, INSTR};
/**
 *	\typedef	loglevel_t
 *	\brief		Définition du type de données loglevel_t
 */
typedef enum LOG_LEVEL loglevel_t;
/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
/**
 * \fn			void logMessage(char *msg, loglevel_t level, ...)
 * \brief		Fonction wrapper de vfprintf() permettant d'afficher
 * 				un message à l'écran avec un niveau de log.
 */
void logMessage(char *msg, loglevel_t level, ...);

#endif /* ARC_LOGGING_H */
