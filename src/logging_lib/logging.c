/**
 *	\file		logging.c
 *	\brief		implémentation de la lib de logging
 *	\author		ARCELON Louis
 *	\date		7 janvier 2026
 *	\version	1.0
 */
#include <unistd.h>
#include "logging.h"
 /*
*****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

void logMessage(char *msg, loglevel_t level, ...) {

	va_list va;

	va_start(va, level);

	fprintf(stderr, "[%d]", getpid());

	switch (level) {
		case DEBUG: fprintf(stderr, "[DEBUG] "); break;
		case WARNING: fprintf(stderr, "[WARNING] "); break;
		case ERROR: fprintf(stderr, "[ERROR] "); break;
		case INSTR: fprintf(stderr, "[INSTR] "); break;
	}

	vfprintf(stderr, msg, va);

	va_end(va);

}

