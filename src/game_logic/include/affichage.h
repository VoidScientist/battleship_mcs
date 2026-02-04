/**
 *	\file		affichage.h
 *	\brief		Fichier Header de affichage.c
 *	\author		MARTEL Mathieu
 *	\version	1.0
 */
 
 /*
 *****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
 #include "bataille_navale.h"
 
 /*
 *****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
 
/**
 * @brief Nettoyer le terminal
 */
 void clear_screen();
 
 /**
  * @brief Affiche une grille de jeu
  * 
  * @param grille 		Représentation de la gille de jeu
  * @param montrer 		Montrer (1) ou non (0) les bateaux de la grille
  */
 void afficher_grille(int grille[TAILLE][TAILLE], int montrer);
 
 /**
  * @brief Afficher la grille d'une équipe
  * 
  * @param equipe 		Equipe dont on affiche la grille (avec les bateaux)
  */
 void afficher_equipe(Equipe *equipe);
 
 /**
  * @brief Afficher la grille de tirs pour une équipe
  * 
  * @param equipe 		Equipe dont on affiche la vue (sans les bateaux)
  */
 void afficher_vue(Equipe *equipe);
 
 /**
  * @brief Lire les coordonnées d'un tir
  * 
  * @param ligne 		Numéro de la ligne
  * @param col 			Numéro de la colonne
  */
 void lire_coords(int *ligne, int *col);
 
 /**
  * @brief Lire les entrées utilisateur pour placer un bateau
  * 
  * @param ligne 		Numéro de la ligne
  * @param col 			Numéro de la colonne
  * @param orient 		Orientation du bateau
  */
 void lire_bateau(int *ligne, int *col, char *orient);