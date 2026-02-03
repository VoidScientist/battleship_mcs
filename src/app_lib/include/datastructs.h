/**
 *	\file		datastructs.h
 *	\brief		Fichier en-tête représentant les structures de données d'échange
 *	\author		ARCELON Louis
 *	\date		31 janvier 2026
 *	\version	1.0
 */

#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
/**
 * @brief format de sérialisation des infos clients
 */
#define CLIENT_INFO_OUT "%s,%d,%s,%d"
/**
 * @brief format de désérialisation des infos clients
 */
#define CLIENT_INFO_IN "%[^,],%d,%[^,],%d"

/**
 * @brief taille d'une adresse IPv4 en termes de string
 */
#define ADDR_SIZE 15
/**
 * @brief taille maximum du pseudo en termes de string
 */
#define PSEUDO_SIZE 11
/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */
/**
 * @brief enum donnant les états de connexion des clients
 */
typedef enum {DISCONNECTED, CONNECTING, CONNECTED} userStatus_t;
/**
 * @brief enum donnant les rôles possibles des clients
 */
typedef enum {PLAYER, HOST} userRole_t;
/**
 * @brief struct rassemblant les infos clients
 */
typedef struct {

	/// nom d'utilisateur du client
	char name[PSEUDO_SIZE];
	/// status de connexion du client
	userStatus_t status;
	/// role du client
	userRole_t role;
	/// adresse LAN du client
	char address[ADDR_SIZE];
	/// port du client (s'il est hôte)
	short port;

} clientInfo_t;
/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
/**
 * @brief      Crée les informations d'un client à partir d'arguments
 *
 * @param      infos    La structure à remplir
 * @param      name     Le nom d'utilisateur
 * @param[in]  role     Le role
 * @param      address  L'adresse LAN du client
 * @param[in]  port     Le port du client
 */
void createClientInfo(clientInfo_t *infos, char *name, userRole_t role, char *address, short port);
/**
 * @brief      fonction de sérialisation des infos clients
 *
 * @param      infos  les infos
 * @param      str    le buffer sérialisé
 */
void clientInfo2str(clientInfo_t *infos, char *str);
/**
 * @brief      fonction de désérialisation des infos clients
 *
 * @param      str    le buffer sérialisé
 * @param      infos  les infos désérialisées
 */
void str2clientInfo(char *str, clientInfo_t *infos);
/**
 * @brief      Récupère dans une list d'infos clients le nombre d'hôtes
 *
 * @param      clients  les clients
 * @param[in]  size     la taille du tableau
 *
 * @return     le nombre d'hôtes parmis les clients
 */
int getHostsAmount(clientInfo_t *clients, int size);


#endif /* DATASTRUCTS_H */