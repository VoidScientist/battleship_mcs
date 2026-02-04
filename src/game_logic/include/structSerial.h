/**
 *	\file		structSerial.h
 *	\brief		Fichier Header pour la serialisation des structures
 *	\author		MARTEL Mathieu
 *	\version	1.0
 */
#ifndef STRUCT_SERIAL_H
#define STRUCT_SERIAL_H

/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include "bataille_navale.h"

/*
*****************************************************************************************
 *	\noop		P R O T O T Y P E S   DE   F O N C T I O N S
 */

/**
 *	Fonctions pour la structure Joueur
 */
/**
 * @brief Convertir la structure Joueur en une chaine de caractères
 * 
 * @param joueur 		Joueur à sérialiser 
 * @param str           Chaine de destination
 */
void joueur2str(const Joueur * joueur, char * str);
/**
 * @brief Convertir une chaine de caractères en structure Joueur
 * 
 * @param str 			Chaine source
 * @param joueur        Joueur de destination 
 */
void str2joueur(const char * str, Joueur * joueur);

/**
 *	Fonctions pour la structure Tir
 */
/**
 * @brief Convertir la structure Tir en une chaine de caractères
 * 
 * @param tir 		    Tir à sérialiser 
 * @param str           Chaine de destination
 */
void tir2str(const Tir * tir, char * str);
/**
 * @brief Convertir une chaine de caractères en structure Tir
 * 
 * @param str 			Chaine source
 * @param tir           Tir de destination
 */
void str2tir(const char * str, Tir * tir);

/**
 *	Fonctions pour la structure Resultat
 */
/**
 * @brief Convertir la structure Resultat en une chaine de caractères
 * 
 * @param resultat 		Resultat à sérialiser
 * @param str 			Chaine de destination
 */
void resultat2str(const Resultat * resultat, char * str);
/**
 * @brief Convertir une chaine de caractères en structure Resultat
 * 
 * @param str 			Chaine source
 * @param resultat      Resultat de destination
 */
void str2resultat(const char * str, Resultat * resultat);

/**
 *	Fonctions pour la structure Placement
 */
/**
 * @brief Convertir la structure Place en une chaine de caractères
 * 
 * @param place 		Place à sérialiser
 * @param str 			Chaine de destination
 */
void place2str(const Placement * place, char * str);
/**
 * @brief Convertir une chaine de caractères en structure Place
 * 
 * @param str 			Chaine source
 * @param place 		Place de destination
 */
void str2place(const char * str, Placement * place);

/**
 *	Fonctions pour la structure Tour
 */
/**
 * @brief Convertir la structure Tour en une chaine de caractères
 * 
 * @param tour 			Tour à sérialiser
 * @param str 			Chaine de destination
 */
void tour2str(const Tour * tour, char * str);
/**
 * @brief Convertir une chaine de caractères en structure Tour
 * 
 * @param str 			Chaine source
 * @param tour 			Tour de destination
 */
void str2tour(const char * str, Tour * tour);

#endif
