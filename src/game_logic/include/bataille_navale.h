/**
 *  \file       bataille_navale.h
 *  \brief      Fichier Header, définition des struct et des constantes utiles à l'application
 *  \author     MARTEL Mathieu
 *  \version    1.0
 */
#ifndef BATAILLE_H
#define BATAILLE_H
/*
*****************************************************************************************
 *  \noop       D E F I N I T I O N   DES   C O N S T A N T E S
 */
/** @brief Taille de la grille de jeu (10x10) */
#define TAILLE 10

/** @brief Nombre de bateaux par équipe */
#define NB_BATEAUX 5

/** @brief Nombre maximum de joueurs par équipe */
#define MAX_JOUEURS 10
/*
*****************************************************************************************
 *  \noop       D E F I N I T I O N   DES   M A C R O S
 */
/** @brief Vérifie si une valeur représente un bateau (entre 2 et 6) */
#define EST_BATEAU(v) ((v) >= 2 && (v) <= 6)

/** @brief Vérifie si une valeur représente une case touchée (entre 12 et 16) */
#define EST_TOUCHE(v) ((v) >= 12 && (v) <= 16)

/** @brief Convertit un id de bateau en id de case touchée */
#define TOUCHE(id) ((id) + 10)
/*
*****************************************************************************************
 *  \noop       S T R C T U R E S   DE   D O N N E E S
 */
/**
 * @brief Orientation d'un bateau
 */
typedef enum { 
    HORIZONTAL = 0,     /**< Bateau positionné horizontalement */
    VERTICAL = 1        /**< Bateau positionné verticalement */
} Orientation;

/**
 * @brief Représentation d'un bateau
 */
typedef struct {
    int id;             /**< I du bateau */
    int longueur;       /**< Longueur du bateau en cases */
    int ligne;          /**< Ligne de départ du bateau */
    int col;            /**< Colonne de départ du bateau */
    Orientation orient; /**< Orientation du bateau */
    int touches;        /**< Nombre de cases touchées */
    int coule;          /**< Bateau coulé (1) ou non (0) */
} Bateau;

/**
 * @brief Grille de jeu d'une équipe
 */
typedef struct {
    int cases[TAILLE][TAILLE];      /**< Cases de la grille */
    Bateau bateaux[NB_BATEAUX];     /**< Bateaux placés sur la grille */
    int nb_bateaux;                 /**< Nombre de bateaux placés */
    int nb_coules;                  /**< Nombre de bateaux coulés */
} Grille;

/**
 * @brief Représentation d'un joueur
 */
typedef struct {
    int id;             /**< Id du joueur */
    char nom[50];       /**< Nom du joueur */
} Joueur;

/**
 * @brief Représentation d'une équipe
 */
typedef struct {
    int id;                         /**< Id de l'équipe */
    char nom[50];                   /**< Nom de l'équipe */
    Grille grille;                  /**< Grille de jeu de l'équipe */
    int vue[TAILLE][TAILLE];        /**< Vue de la grille adverse */
    Joueur joueurs[MAX_JOUEURS];    /**< Liste des joueurs de l'équipe */
    int nb_joueurs;                 /**< Nombre de joueurs dans l'équipe */
    int joueur_actif;               /**< Index du joueur actif */
} Equipe;

/**
 * @brief Représentation d'une partie de jeu
 */
typedef struct {
    Equipe equipeA;     /**< Équipe A */
    Equipe equipeB;     /**< Équipe B */
    int equipe_active;  /**< Équipe dont c'est le tour */
    int fini;           /**< Partie terminée (1) ou non (0) */
    int gagnant;        /**< Équipe gagnante (-1 si partie en cours) */
} Jeu;

/**
 * @brief Placement d'un bateau sur la grille
 */
typedef struct {
    int id;             /**< Identifiant du bateau */
    int longueur;       /**< Longueur du bateau */
    int ligne;          /**< Ligne de départ */
    int col;            /**< Colonne de départ */
    Orientation orient; /**< Orientation du bateau */
} Placement;

/**
 * @brief Tir effectué par une équipe
 */
typedef struct {
    int ligne;          /**< Ligne visée */
    int col;            /**< Colonne visée */
    int equipe_id;      /**< Id de l'équipe qui tire */
} Tir;

/**
 * @brief Tour de jeu
 */
typedef struct {
    int equipe_id;      /**< Id de l'équipe active */
    int joueur_id;      /**< Id du joueur actif */
    int phase;          /**< Phase de jeu (0=placement, 1=bataille) */
} Tour;

/**
 * @brief Résultat d'un tir
 */
typedef struct {
    int ligne;          /**< Ligne du tir */
    int col;            /**< Colonne du tir */
    int touche;         /**< Tir touché (1) ou raté (0) */
    int coule;          /**< Bateau coulé (1) ou non (0) */
    int id_coule;       /**< Id du bateau touché/coulé */
} Resultat;

#endif