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
 *	\fn					void clear_screen()
 *	\brief		Clear le terminal
 */
 void clear_screen();
 
 /**
 *	\fn					void afficher_grille(int grille[TAILLE][TAILLE], int montrer)
 *	\brief		Afficher une grille de jeu
 *	\param 		grille[TAILLE][TAILLE] : tableau 2 dimensions pour représenter la grille de jeu de taille TAILLE*TAILLE
 *	\param 		montrer : montrer (1) ou non (0) les bateaux 
 */
 void afficher_grille(int grille[TAILLE][TAILLE], int montrer);
 
 /**
 *	\fn					void afficher_equipe(Equipe *equipe)
 *	\brief		Afficher la grille d'une equipe
 *	\param 		equipe : l'equipe dont il faut afficher la grille
 */
 void afficher_equipe(Equipe *equipe);
 
 /**
 *	\fn					void afficher_vue(Equipe *equipe)
 *	\brief		Afficher la grille de tirs pour une equipe
 *	\param 		equipe : equipe
 */
 void afficher_vue(Equipe *equipe);
 
 /**
 *	\fn					void lire_coords(int *ligne, int *col)
 *	\brief		Lire les coordonnés d'un tir
 *	\param 		ligne : numero de la ligne
 *	\param 		col : numero de la colonne
 */
 void lire_coords(int *ligne, int *col);
 
 /**
 *	\fn					void lire_bateau(int *ligne, int *col, char *orient)
 *	\brief		Lire les informations pour placer un bateau
 *	\param 		ligne : numero de la ligne
 *	\param 		col : numero de la colonne
 *	\param 		orient : orientation du bateau
 */
 void lire_bateau(int *ligne, int *col, char *orient);
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
