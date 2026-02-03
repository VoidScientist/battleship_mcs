/**
 *	\file		logic.c
 *	\brief		Fichier qui contient la logique du jeu
 *	\author		MARTEL Mathieu
 *	\version	1.0
 */

 /*
 *****************************************************************************************
 *	\noop		I N C L U D E S 
 */
#include <string.h>

 /*
 *****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include "include/logic.h"

 /*
 *****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */
 
// initialise une grille vide
void init_grille(Grille *grille) {
    memset(grille->cases, 0, sizeof(grille->cases));
    grille->nb_bateaux = grille->nb_coules = 0;
}

// initialise une vue adversaire vide
void init_vue(int vue[TAILLE][TAILLE]) {
    memset(vue, 0, sizeof(int) * TAILLE * TAILLE);
}

// initialise un joueur avec son nom
void init_joueur(Joueur *joueur, int id, const char *nom) {
    joueur->id = id;
    strcpy(joueur->nom, nom);
}

// initialise une equipe avec sa grille
void init_equipe(Equipe *equipe, int id, const char *nom) {
    equipe->id = id;
    strcpy(equipe->nom, nom);
    equipe->nb_joueurs = equipe->joueur_actif = 0;
    init_grille(&equipe->grille);
    init_vue(equipe->vue);
}

// initialise le jeu complet
void init_jeu(Jeu *jeu) {
    init_equipe(&jeu->equipeA, 0, "Equipe A");
    init_equipe(&jeu->equipeB, 1, "Equipe B");
    jeu->equipe_active = jeu->fini = 0;
    jeu->gagnant = -1;
}

// ajoute un joueur a une equipe
int ajouter_joueur(Equipe *equipe, const char *nom) {
    if(equipe->nb_joueurs >= MAX_JOUEURS) return 0;
    init_joueur(&equipe->joueurs[equipe->nb_joueurs], equipe->nb_joueurs, nom);
    equipe->nb_joueurs++;
    return 1;
}

// retourne le joueur actif d'une equipe
Joueur* joueur_actif(Equipe *equipe) {
    return equipe->nb_joueurs > 0 ? &equipe->joueurs[equipe->joueur_actif] : NULL;
}

// passe au joueur suivant dans l'equipe
void joueur_suivant(Equipe *equipe) {
    equipe->joueur_actif = (equipe->joueur_actif + 1) % equipe->nb_joueurs;
}

// verifie si une position est valide pour placer un bateau
int position_valide(Grille *grille, int ligne, int col, int longueur, Orientation orient) {
    int ligne_test;
    int col_test; 
    
    if(ligne < 0 || col < 0 || ligne >= TAILLE || col >= TAILLE) return 0;
    if(orient == HORIZONTAL && col + longueur > TAILLE) return 0;
    if(orient == VERTICAL && ligne + longueur > TAILLE) return 0;
    
    for(int i = 0; i < longueur; i++) {
        ligne_test = (orient == VERTICAL) ? ligne + i : ligne;
        col_test = (orient == HORIZONTAL) ? col + i : col;
        if(grille->cases[ligne_test][col_test] != 0) return 0;
    }
    return 1;
}

// place un bateau sur la grille
int placer_bateau(Grille *grille, int id, int longueur, int ligne, int col, Orientation orient) {
    int ligne_pos;
    int col_pos;
    
    if(!position_valide(grille, ligne, col, longueur, orient)) return 0;
    if(grille->nb_bateaux >= NB_BATEAUX) return 0;
    
    Bateau *bateau = &grille->bateaux[grille->nb_bateaux++];
    bateau->id = id;
    bateau->longueur = longueur;
    bateau->ligne = ligne;
    bateau->col = col;
    bateau->orient = orient;
    bateau->touches = bateau->coule = 0;
		  
    for(int i = 0; i < longueur; i++) {
        ligne_pos = (orient == VERTICAL) ? ligne + i : ligne;
        col_pos = (orient == HORIZONTAL) ? col + i : col;
        grille->cases[ligne_pos][col_pos] = id;
    }
    return 1;
}

// trouve un bateau par son id
Bateau* trouver_bateau(Grille *grille, int id) {
    for(int i = 0; i < grille->nb_bateaux; i++)
        if(grille->bateaux[i].id == id) return &grille->bateaux[i];
    return NULL;
}

// effectue un sur la grille cible
Resultat tirer(Grille *cible, int vue[TAILLE][TAILLE], int ligne, int col) {
	int valeur_case;
	
    Resultat resultat = {ligne, col, 0, 0, 0};
    if(ligne < 0 || col < 0 || ligne >= TAILLE || col >= TAILLE) return resultat;
    
    valeur_case = cible->cases[ligne][col];
    if(valeur_case == 1 || EST_TOUCHE(valeur_case)) return resultat;
    
    if(EST_BATEAU(valeur_case)) {
        resultat.touche = 1;
        resultat.id_coule = valeur_case;
        cible->cases[ligne][col] = TOUCHE(valeur_case);
        vue[ligne][col] = TOUCHE(valeur_case);
        
        Bateau *bateau = trouver_bateau(cible, valeur_case);
        if(bateau) {
            bateau->touches++;
            if(bateau->touches >= bateau->longueur) {
                bateau->coule = 1;
                resultat.coule = 1;
                cible->nb_coules++;
            }
        }
    } else {
        cible->cases[ligne][col] = vue[ligne][col] = 1;
    }
    return resultat;
}

// verifie si tous les bateaux sont coules
int victoire(Grille *grille) {
    return grille->nb_coules >= grille->nb_bateaux && grille->nb_bateaux > 0;
}

// gere un tour de jeu
void tour(Jeu *jeu, int ligne, int col, int *rejouer) {
    *rejouer = 0;
    Equipe *attaquant = jeu->equipe_active ? &jeu->equipeB : &jeu->equipeA;
    Equipe *defenseur = jeu->equipe_active ? &jeu->equipeA : &jeu->equipeB;
    
    Resultat resultat = tirer(&defenseur->grille, attaquant->vue, ligne, col);
    joueur_suivant(attaquant);
    
    if(resultat.touche) {
        *rejouer = 1;
        if(victoire(&defenseur->grille)) {
            jeu->fini = 1;
            jeu->gagnant = jeu->equipe_active;
        }
    } else {
        jeu->equipe_active = 1 - jeu->equipe_active;
    }
}
