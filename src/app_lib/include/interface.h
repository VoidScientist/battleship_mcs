/**
 *	\file		interface.h
 *	\brief		fichier entête utilisé pour interagir avec stdin
 *				et faire un affichage dans le terminal
 *	\author		ARCELON Louis
 *	\date		1 février 2026
 *	\version	1.0
 */
#ifndef AMP_INTERFACE_H
#define AMP_INTERFACE_H
/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include <stdio.h>
#include <stdarg.h>
#include <semaphore.h>
#include "datastructs.h"
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
/**
 * @brief 	taille des buffer d'entrées utilisateur
 */
#define INPUT_BUFFER_SIZE	256
/**
 * @brief 	port par défaut choisi par l'interface
 */
#define DEFAULT_PORT		15500
/**
 * @brief 	code de succès des fonctions d'entrées
 */
#define STEP_SUCCESS		0
/**
 * @brief 	code renvoyé lorsque fgets plante
 */
#define FGETS_ERROR			-1
/**
 * @brief 	code renvoyé lorsque l'utilisateur fais une action qui 
 * 			implique choisir les valeurs par défaut
 */
#define USE_DEFAULT			-2
/**
 * @brief 	code renvoyé quand vsscanf rencontre une erreur
 */
#define VSSCANF_ERROR		-3
/**
 * @brief 	code d'erreur lorsque l'on rencontre un problème
 * 			de détermination du nombre de valeurs attendues par un format
 */
#define EXPECT_ERROR		-4
/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */
/**
 * @brief      type de fonction utilisé pour les callbacks des menus
 */
typedef void (*callback)();
/**
 * @brief      Paramètres à fournir au menu pour son bon fonctionnement.
 */
typedef struct {

	/// callback d'affichage des hôtes
	callback 		showHosts;
	/// callback de fermeture du programme
	callback 		exitProgram;
	/// pointeur vers la liste d'hôtes maintenue par le client
	clientInfo_t	*hosts;
	/// pointeur vers l'hôte choisi
	clientInfo_t	*chosenHost;
	
} playerMenuParams_t;
/**
 * @brief 	Enum d'états possibles du menu. (pages)
 */
typedef enum {MAIN_MENU, JOIN_MENU, CLOSE_MENU} menuState_t;
/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
/**
 * @brief      demande à l'utilisateur les informations pour compléter son profil
 *
 * @param      infos  pointeur vers les infos client
 */
void setupUserInfos(clientInfo_t *infos);
/**
 * @brief      Demande à l'utilisateur l'adresse applicative du serveur 
 * 			   d'enregistrement ou utilise celle par défaut.
 *
 * @param[in]   adrIP     l'adresse ip par défaut
 * @param[in]  	port      le port par défaut
 * @param[out]  userIP    pointeur vers la variable qui sera utilisée comme adresse IP
 * @param[out]  userPort  pointeur vers la variable qui sera utilisée comme port
 */
void getSrvEAddress(char* adrIP, unsigned short port, char *userIP, short *userPort);
/**
 * @brief      Affiche une liste d'hôtes dans le terminal
 *
 * @param      hosts   les hôtes à afficher
 * @param[in]  amount  la taille de `hosts`
 */
void displayHosts(clientInfo_t *hosts, int amount);
/**
 * @brief      affiche les menus dans le terminal avec une machine à états
 *
 * @param[in]  params  paramètres utiles aux menus (callbacks etc...)
 */
void displayPlayerMenu(playerMenuParams_t params);
/**
 * @brief      Récupère une entrée dans stdin avec des sécurités
 *
 * @param      fmt      Format d'entrée
 * @param[in]  ...      les pointeurs des variables dans lesquelles stocker les entrées
 * 
 * @return 	   retourne STEP_SUCCESS en cas de réussite ou USE_DEFAULT, EXPECT_ERROR, FGETS_ERROR, VSSCANF_ERROR 
 */
int retrieveInput(char *fmt, ...);
/**
 * @brief      fonctions permettant de récupérer une ligne dans stdin.
 *
 * @param      buffer  le buffer dans lequel stocker la ligne
 * @param[in]  size    la taille du buffer
 *
 * @return     retourne STEP_SUCCESS, USE_DEFAULT ou FGETS_ERROR 
 */
int saferFgets(char *buffer, int size);
/**
 * @brief      Récupère des entrées depuis un format, un string et les
 *             stock dans les pointeurs (avec des sécurités)
 *
 * @param      buff  le buffer dans lequel récupérer les données
 * @param      fmt   le format
 * @param[in]  args  les pointeurs
 *
 * @return     STEP_SUCCESS en cas de réussite sinon VSSCANF_ERROR
 */
int saferVsscanf(char *buff, char *fmt, va_list args);
/**
 * \brief      Calcule le résultat attendu de vsscanf à partir du format.
 *
 * \param      fmt   Le format
 * \param[in]  size  La taille du buffer (avec \0)
 *
 * \return     Le nombre de correspondances attendu, ou EXPECT_ERROR.
 */
int calculateExpectedFromFmt(char *fmt, int size);

/**
 * @brief      affiche le menu principal
 *
 * @param[in]  params  les paramètres utiles au menu (callbacks etc...)
 * @param      state   pointeur de l'état de la MEF du menu
 */
void displayMainMenu(playerMenuParams_t params, menuState_t *state);
/**
 * @brief      affiche le menu des hôtes
 *
 * @param[in]  params  les paramètres utiles au menu (callbacks etc...)
 * @param      state   pointeur de l'état de la MEF du menu
 */
void displayJoinMenu(playerMenuParams_t params, menuState_t *state);


#endif /* AMP_INTERFACE_H */