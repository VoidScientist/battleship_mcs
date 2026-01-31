/**
 *	\file		bataille_navale.h
 *	\brief		Fichier Header, définition des struct et des constantes utiles à l'application
 *	\author		MARTEL Mathieu
 *	\version	1.0
 */


#ifndef BATAILLE_H
#define BATAILLE_H

/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   C O N S T A N T E S
 */
#define TAILLE 10
#define NB_BATEAUX 5
#define MAX_JOUEURS 10

/*
*****************************************************************************************
 *	\noop		D E F I N I T I O N   DES   M A C R O S
 */
#define EST_BATEAU(v) ((v) >= 2 && (v) <= 6)
#define EST_TOUCHE(v) ((v) >= 12 && (v) <= 16)
#define TOUCHE(id) ((id) + 10)

/*
*****************************************************************************************
 *	\noop		S T R C T U R E S   DE   D O N N E E S
 */
typedef enum { HORIZONTAL = 0, VERTICAL = 1 } Orientation;

typedef struct {
    int id, longueur, ligne, col;
    Orientation orient;
    int touches, coule;
} Bateau;

typedef struct {
    int cases[TAILLE][TAILLE];
    Bateau bateaux[NB_BATEAUX];
    int nb_bateaux, nb_coules;
} Grille;

typedef struct {
    int id;
    char nom[50];
} Joueur;

typedef struct {
    int id;
    char nom[50];
    Grille grille;
    int vue[TAILLE][TAILLE];
    Joueur joueurs[MAX_JOUEURS];
    int nb_joueurs, joueur_actif;
} Equipe;

typedef struct {
    Equipe equipeA, equipeB;
    int equipe_active, fini, gagnant;
} Jeu;

typedef struct {
	int id, longueur, ligne, col;
	Orientation orient;
} Placement;

typedef struct {
	int equipe_id, joueur_id;
	int phase; 		// 0 = placement	|	1 = bataille 
} Tour;

typedef struct {
	int ligne, col;
    int touche, coule, id_coule;
} Resultat;

#endif
