/**
 *	\file		logic.h
 *	\brief		Fichier Header de logic.c
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
 *	\fn					void init_grille(Grille *grille)
 *	\brief		Initialiser une grille vide
 *	\param 		grille : grille remise a zero
 */
 void init_grille(Grille *grille);
 
 /**
 *	\fn					void init_vue(int vue[TAILLE][TAILLE])
 *	\brief		Initialiser une vue adverse vide
 *	\param 		vue[TAILLE][TAILLE] : grille adverse vide de taille TAILLE*TAILLE
 */
 void init_vue(int vue[TAILLE][TAILLE]);
 
 /**
 *	\fn					void init_joueur(Joueur *joueur, int id, const char *nom)
 *	\brief		Initialiser un joueur
 *	\param 		joueur : le joueur initialise
 *	\param 		id : son id
 *	\param 		nom : son nom
 */
 void init_joueur(Joueur *joueur, int id, const char *nom);
 
 /**
 *	\fn					void init_equipe(Equipe *equipe, int id, const char *nom)
 *	\brief		Initialiser une equipe
 *	\param 		equipe : l'equipe initialisee
 *	\param 		id : son id
 *	\param 		nom : son nom
 */
 void init_equipe(Equipe *equipe, int id, const char *nom);
 
 /**
 *	\fn					void init_jeu(Jeu *jeu)
 *	\brief		Initialiser une partie de jeu
 *	\param 		jeu : le jeu initialise
 */
 void init_jeu(Jeu *jeu);
 
 /**
 *	\fn					int ajouter_joueur(Equipe *equipe, const char *nom)
 *	\brief		Ajouter un joueur a une equipe
 *	\param 		equipe : l'equipe d'ajout
 *	\param 		nom : nom du joueur ajoute a l'equipe
  *	\ret		int: 0 (echec) ou 1 (reussite)
 */
 int ajouter_joueur(Equipe *equipe, const char *nom);
 
 /**
 *	\fn					Joueur* joueur_actif(Equipe *equipe)
 *	\brief		Retourner le joueur actif d'une equipe
 *	\param 		equipe : l'equipe du joueur actif
 * 	\ret		Joueur : le joueur actif
 */
 Joueur* joueur_actif(Equipe *equipe);
 
 /**
 *	\fn					void joueur_suivant(Equipe *equipe))
 *	\brief		Passer au joueur suivant dans une equipe
 *	\param 		equipe : l'equipe du traitement
 */
 void joueur_suivant(Equipe *equipe);
 
 /**
 *	\fn					int position_valide(Grille *grille, int ligne, int col, int longueur, Orientation orient)
 *	\brief		Verifier si la position est valide pour poser un bateau
 *	\param 		grille : grille en cours
 *	\param 		ligne : numero de ligne
 *	\param 		col : numero de colonne
 *	\param 		longueur : longueur du bateau
 *	\param 		orient : orientation du bateau
 *	\ret		int: 0 (invalide) ou 1 (valide)
 */
 int position_valide(Grille *grille, int ligne, int col, int longueur, Orientation orient);
 
 /**
 *	\fn					int position_valide(Grille *grille, int ligne, int col, int longueur, Orientation orient)
 *	\brief		Verifier si la position est valide pour poser un bateau
 *	\param 		grille : grille en cours
 *	\param 		ligne : numero de ligne
 *	\param 		col : numero de colonne
 *	\param 		longueur : longueur du bateau
 *	\param 		orient : orientation du bateau
 */
 int placer_bateau(Grille *grille, int id, int longueur, int ligne, int col, Orientation orient);
 
 /**
 *	\fn					Bateau* trouver_bateau(Grille *grille, int id)
 *	\brief		Trouver un bateau par son id
 *	\param 		grille : grille sur laquelle on cherche le bateau
 *	\param 		id : id du bateau
 *	\ret		Bateau : retourne le bateau
 */
 Bateau* trouver_bateau(Grille *grille, int id);
 
  /**
 *	\fn					Resultat tirer(Grille *cible, int vue[TAILLE][TAILLE], int ligne, int col)
 *	\brief		Trouver un bateau par son id
 *	\param 		grille : grille sur laquelle on cherche le bateau
 *	\param 		id : id du bateau
 *	\ret		Bateau : retourne le bateau
 */
 Resultat tirer(Grille *cible, int vue[TAILLE][TAILLE], int ligne, int col);
 
 int victoire(Grille *grille);
 
 void tour(Jeu *jeu, int ligne, int col, int *rejouer);
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
