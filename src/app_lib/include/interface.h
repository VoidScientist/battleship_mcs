/**
 *	\file		interface.h
 *	\brief		fichier entête utilisé pour interagir avec stdin
 *				et faire un affichage dans le terminal
 *	\author		ARCELON Louis
 *	\date		1 février 2026
 *	\version	1.0
 */
#include <stdio.h>
#include <stdarg.h>
#include <semaphore.h>
#include "datastructs.h"


#define USER_BUFFER_SIZE 	100
#define INPUT_BUFFER_SIZE	256

#define DEFAULT_PORT		15500

#define STEP_SUCCESS		0
#define FGETS_ERROR			-1
#define USE_DEFAULT			-2
#define VSSCANF_ERROR		-3
#define EXPECT_ERROR		-4


typedef void (*callback)();


typedef struct {

	callback 	showHosts;
	callback 	exitProgram;
	
} playerMenuParams_t;



void setupUserInfos(clientInfo_t *infos);

void getSrvEAddress(char* adrIP, unsigned short port, char *userIP, short *userPort);

void displayHosts(clientInfo_t *hosts, int amount);

void displayPlayerMenu(playerMenuParams_t *params);

int retrieveInput(char *fmt, ...);

int saferFgets(char *buffer, int size);


int saferVsscanf(char *buff, char *fmt, va_list args);

/**
 * @brief      Calcule le résultat attendu de vsscanf à partir du format.
 *
 * @param      fmt   Le format
 * @param[in]  size  La taille du buffer (avec \0)
 *
 * @return     Le nombre de correspondances attendu.
 */
int calculateExpectedFromFmt(char *fmt, int size);