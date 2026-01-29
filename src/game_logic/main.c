/**
 *	\file		main.c
 *	\brief		Fichier principal du jeu. Gère les boucles de jeu.
 *	\author		MARTEL Mathieu
 *	\version	1.0
 */

/*
*****************************************************************************************
 *	\noop		I N C L U D E S 
 */
#include <stdio.h>

/*
*****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include "include/bataille_navale.h"
#include "include/affichage.h"
#include "include/logic.h"

/*
 *****************************************************************************************
 *	\noop		D E C L A R A T I O N   DES   V A R I A B L E S    G L O B A L E S
 */
 
 /**
 *	\var		bateaux_ids
 *	\brief		ID des bateaux
 */
int bateaux_ids[] = {2, 3, 4, 5, 6};

 /**
 *	\var		bateaux_longueurs
 *	\brief		Longueur des bateaux
 */
int bateaux_longueurs[] = {5, 4, 3, 3, 2};

// phase de placement pour une equipe
void placer_equipe(Equipe *equipe) {
    clear_screen();
    printf("\n=== Placement %s ===\n", equipe->nom);
    
    for(int i = 0; i < NB_BATEAUX; i++) {
        int ok = 0;
        while(!ok) {
            clear_screen();
            printf("\n=== Placement %s ===\n", equipe->nom);
            printf("\nBateau %d/%d (longueur %d)\n", i+1, NB_BATEAUX, bateaux_longueurs[i]);
            afficher_equipe(equipe);
            
            int ligne, col;
            char orient_char;
            lire_bateau(&ligne, &col, &orient_char);
            
            Orientation orient = (orient_char == 'H') ? HORIZONTAL : VERTICAL;
            if(placer_bateau(&equipe->grille, bateaux_ids[i], bateaux_longueurs[i], ligne, col, orient)) {
                printf("Bateau place avec succes !\n");
                ok = 1;
            } else {
                printf("Position invalide pour ce bateau (sort de la grille ou chevauche un autre bateau)\n");
                printf("Appuyez sur Entree...");
                getchar();
            }
        }
    }
    
    clear_screen();
    printf("\n=== Placement %s termine ===\n", equipe->nom);
    afficher_equipe(equipe);
    printf("\nAppuyez sur Entree pour continuer...");
    getchar();
}

// boucle de jeu principale
void jouer(Jeu *jeu) {
    clear_screen();
    printf("\n=== Debut de la partie ===\n");
    printf("Appuyez sur Entree...");
    getchar();
    
    while(!jeu->fini) {
        clear_screen();
        Equipe *equipe = jeu->equipe_active ? &jeu->equipeB : &jeu->equipeA;
        
        Joueur *joueur = joueur_actif(equipe);
        
        printf("\n--- Tour de %s (%s) ---\n", joueur->nom, equipe->nom);
        afficher_equipe(equipe);
        afficher_vue(equipe);
        
        int ligne, col, rejouer;
        lire_coords(&ligne, &col);
        tour(jeu, ligne, col, &rejouer);
        
        printf(rejouer ? "\nTouche !\n" : "\nLoupe.\n");
        if(jeu->fini) {
            clear_screen();
            printf("\n============================\n");
            printf("    %s gagne !\n", equipe->nom);
            printf("============================\n");
            break;
        }
        
        printf("Appuyez sur Entree...");
        getchar();
    }
}

int main() {
    Jeu jeu;
    init_jeu(&jeu);
    
    printf("Bataille Navale\n\n");
    
    int nb_joueurs_a, nb_joueurs_b;
    printf("Joueurs equipe A: ");
    scanf("%d", &nb_joueurs_a);
    printf("Joueurs equipe B: ");
    scanf("%d", &nb_joueurs_b);
    
    char nom[50];
    for(int i = 0; i < nb_joueurs_a; i++) {
        sprintf(nom, "JoueurA%d", i+1);
        ajouter_joueur(&jeu.equipeA, nom);
    }
    
    for(int i = 0; i < nb_joueurs_b; i++) {
        sprintf(nom, "JoueurB%d", i+1);
        ajouter_joueur(&jeu.equipeB, nom);
    }
    
    placer_equipe(&jeu.equipeA);
    placer_equipe(&jeu.equipeB);
    jouer(&jeu);
    
    return 0;
}
