/**
 *	\file		logic.h
 *	\brief		Fichier Header de logic.c
 *	\author		MARTEL Mathieu / ARCELON Louis
 *	\version	1.0
 */
 
 /*
 *****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include "bataille_navale.h"
/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
// Équipes
#define EQUIPE_A			0
#define EQUIPE_B			1
 /*
 *****************************************************************************************
 *	\noop		P R O T O T Y P E S   DES   F O N C T I O N S
 */
 
/**
 * @brief Initialise une grille vide
 * 
 * @param grille        Grille remise à zéro
 */
 void init_grille(Grille *grille);

 /**
  * @brief Initialiser une vue de la grille adverse vide
  * 
  * @param vue          Grille adverse vide
  */
 void init_vue(int vue[TAILLE][TAILLE]);
 
 /**
  * @brief Initialiser un jouer
  * 
  * @param joueur       Le joueur a initialiser
  * @param id           Id du joueur
  * @param nom          Nom du joueur
  */
 void init_joueur(Joueur *joueur, int id, const char *nom);
 
 /**
  * @brief Initialiser une équipe
  * 
  * @param equipe       L'équipe a initialiser
  * @param id           Id de l'équipe
  * @param nom          Nom de l'équipe
  */
 void init_equipe(Equipe *equipe, int id, const char *nom);
 
 /**
  * @brief Initialiser une partie de jeu
  * 
  * @param jeu          Jeu a initialiser
  */
 void init_jeu(Jeu *jeu);
 
 /**
  * @brief Ajouter un joueur a une équipe
  * 
  * @param equipe       Equipe d'ajout
  * @param nom          Nom du joueur
  * 
  * @return Réussite: 1 | Echec: 0 pour l'ajout du joueur à l'équipe
  */
 int ajouter_joueur(Equipe *equipe, const char *nom);
 
 /**
  * @brief Retourner le jouer actif d'une équipe
  * 
  * @param equipe       Equipe du joueur
  * 
  * @return Le joueur actif de l'équipe
  */
 Joueur* joueur_actif(Equipe *equipe);
 
 /**
  * @brief Passer au joueur suivant dans une équipe
  * 
  * @param equipe       L'équipe du traitement
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
 /**
  * @brief Vérifier si la position est valide pour poser un bateau
  * 
  * @param grille       Grille de la pose
  * @param ligne        Numéro de la ligne
  * @param col          Numéro de la colonne
  * @param longueur     Longueur du bateau
  * @param orient       Orientation du bateau
  * 
  * @return Valide: 1 | Invalide: 0
  */
 int position_valide(Grille *grille, int ligne, int col, int longueur, Orientation orient);
 
 /**
  * @brief Poser un bateau
  * 
  * @param grille       Grille de la pose
  * @param id           Id du bateau
  * @param longueur     Longueur du bateau
  * @param ligne        Numéro de la ligne
  * @param col          Numéro de la colonne
  * @param orient       Orientation du bateau
  * 
  * @return Réussite: 1 | Echec: 0 de la pose du bateau
  */
 int placer_bateau(Grille *grille, int id, int longueur, int ligne, int col, Orientation orient);
 
 /**
  * @brief Trouver un bateau par son Id
  * 
  * @param grille       Grille de recherche
  * @param id           Id du bateau
  * 
  * @return Le bateau recherché
  */
 Bateau* trouver_bateau(Grille *grille, int id);
 
 /**
  * @brief Tirer
  * 
  * @param cible        Grille cible du tir
  * @param vue          Vue de la grille adverse
  * @param ligne        Numéro de la ligne
  * @param col          Numéro de la colonne
  * 
  * @return Résultat du tir
  */
 Resultat tirer(Grille *cible, int vue[TAILLE][TAILLE], int ligne, int col);
 
 /**
  * @brief Vérifier si tous les bateaux d'une grille sont coulés
  * 
  * @param grille       Grille à vérifier
  * 
  * @return Victoire: 1 | Partie en cours: 0  
  */
 int victoire(Grille *grille);
 
 /**
  * @brief Gérer un tour de jeu complet
  * 
  * @param jeu          Jeu en cours
  * @param ligne        Numéro de la ligne (pour le tir)
  * @param col          Numéro de la colonne (pour le tir)
  * @param rejouer [description]
  */
 void tour(Jeu *jeu, int ligne, int col, int *rejouer);
 
/**
 * @brief Retourne l'équipe correspondante à l'Id
 * 
 * @param jeu           Partie de jeu en cours
 * @param equipeId      Id de l'équipe
 * 
 * @return L'équipe recherchée
 */
Equipe* obtenirMonEquipe(Jeu *jeu, int equipeId);