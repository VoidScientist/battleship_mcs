/**
 *	\file		structSerial.c
 *	\brief		Fichier pour serialiser les structures
 *	\author		MARTEL Mathieu
 *	\version	1.0
 */

 /*
 *****************************************************************************************
 *	\noop		I N C L U D E S 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
 /*
 *****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
 #include "include/bataille_navale.h"
 
 /*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
 #define JOUEUR_STR_OUT "%d,%s"
 #define JOUEUR_STR_IN ""
 
 #define RESULT_STR_OUT "%d,%d,%d"
 #define RESULT_STR_IN ""
 
 /*
 *****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */
 
 /**
 *	Fonctions pour la structure Joueur
 */
 void joueur2str(const Joueur * joueur, char * str) {
 	sprintf(str, JOUEUR_STR_OUT, Joueur->id, Joueur->nom);
 }
 
  void str2joueur(const char * str, Joueur * joueur) {
 	sscanf(str, JOUEUR_STR_IN, &Joueur->id, &Joueur->nom);
 }
 
  /**
 *	Fonctions pour la structure Resultat
 */
  void result2str(const Resultat * resultat, char * str) {
 	sprintf(str, RESULT_STR_OUT, Resultat->touche, Resultat->coule, Resultat->id_coule);
  }
 
  void str2result(const char * str, Joueur * joueur) {
 	sscanf(str, RESULT_STR_IN, &Resultat->touche, &Resultat->coule, &Resultat->id_coule);
  }
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
