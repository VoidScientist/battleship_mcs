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
void joueur2str(const Joueur * joueur, char * str);
void str2joueur(const char * str, Joueur * joueur);

/**
 *	Fonctions pour la structure Tir
 */
void tir2str(const Tir * tir, char * str);
void str2tir(const char * str, Tir * tir);

/**
 *	Fonctions pour la structure Resultat
 */
void resultat2str(const Resultat * resultat, char * str);
void str2resultat(const char * str, Resultat * resultat);

/**
 *	Fonctions pour la structure Placement
 */
void place2str(const Placement * place, char * str);
void str2place(const char * str, Placement * place);

/**
 *	Fonctions pour la structure Tour
 */
void tour2str(const Tour * tour, char * str);
void str2tour(const char * str, Tour * tour);

#endif
