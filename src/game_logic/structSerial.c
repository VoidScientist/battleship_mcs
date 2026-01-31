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
 #define JOUEUR_STR_IN "%d,%49[^\n]"
 
 #define RESULTAT_STR_OUT "%d,%d,%d,%d,%d"
 #define RESULTAT_STR_IN "%d,%d,%d,%d,%d"
 
 #define PLACEMENT_STR_OUT "%d,%d,%d,%d,%d"
 #define PLACEMENT_STR_IN "%d,%d,%d,%d,%d"
 
 #define TOUR_STR_OUT "%d,%d,%d"
 #define TOUR_STR_IN "%d,%d,%d"
 
 /*
 *****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */
 
 /**
 *	Fonctions pour la structure Joueur
 */
 void joueur2str(const Joueur * joueur, char * str) {
 	sprintf(str, JOUEUR_STR_OUT, joueur->id, joueur->nom);
 }
 
  void str2joueur(const char * str, Joueur * joueur) {
 	sscanf(str, JOUEUR_STR_IN, &joueur->id, joueur->nom);
 }
 
  /**
 *	Fonctions pour la structure Resultat
 */
  void resultat2str(const Resultat * resultat, char * str) {
 	sprintf(str, RESULTAT_STR_OUT, resultat->ligne, resultat->col, resultat->touche, resultat->coule, resultat->id_coule);
  }
 
  void str2resultat(const char * str, Resultat * resultat) {
 	sscanf(str, RESULTAT_STR_IN, &resultat->ligne, &resultat->col, &resultat->touche, &resultat->coule, &resultat->id_coule);
  }
  
  /**
 *	Fonctions pour la structure Placement
 */
  void place2str(const Placement * msg, char * str) {
 	sprintf(str, PLACEMENT_STR_OUT, msg->id, msg->longueur, msg->ligne, msg->col, msg->orient);
  }
 
  void str2place(const char * str, Placement * place) {
  	int orient;
 	sscanf(str, PLACEMENT_STR_IN, &place->id, &place->longueur, &place->ligne, &place->col, &orient);
 	place->orient = (Orientation)orient;
  }
  
  /**
 *	Fonctions pour la structure Tour
 */
  void tour2str(const Tour * tour, char * str) {
 	sprintf(str, TOUR_STR_OUT, tour->equipe_id, tour->joueur_id, tour->phase);
  }
 
  void str2tour(const char * str, Tour * tour) {
 	sscanf(str, TOUR_STR_IN, &tour->equipe_id, &tour->joueur_id, &tour->phase);
  }
