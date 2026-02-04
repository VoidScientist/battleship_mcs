/**
 *	\file		affichage.c
 *	\brief		Fichier pour gérer l'affichage de l'application
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
#include "include/affichage.h"

 /*
 *****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */
 
void clear_screen() {
        system("clear");
}

void afficher_grille(int grille[TAILLE][TAILLE], int montrer) {
    int valeur;
    
    printf("   ");
    for(int i = 0; i < TAILLE; i++) printf(" %c", 'A' + i);
    printf("\n");
    
    for(int i = 0; i < TAILLE; i++) {
        printf("%2d ", i + 1);
        for(int j = 0; j < TAILLE; j++) {
            valeur = grille[i][j];
            if(valeur == 0) printf(" .");
            else if(valeur == 1) printf(" o");
            else if(EST_TOUCHE(valeur)) printf(" X");
            else if(EST_BATEAU(valeur) && montrer) printf(" %d", valeur);
            else printf(" .");
        }
        printf("\n");
    }
}

void afficher_equipe(Equipe *equipe) {
    printf("\nGrille %s:\n", equipe->nom);
    afficher_grille(equipe->grille.cases, 1);
}

void afficher_vue(Equipe *equipe) {
    printf("\nGrille adverse:\n");
    afficher_grille(equipe->vue, 0);
}

void lire_coords(int *ligne, int *col) {
    char saisie[10];
    int valide = 0;
    char colonne_char;
    int ligne_saisie;
    
    while(!valide) {
        printf("Position (col ligne): ");
        scanf("%s", saisie);
        
        if(strlen(saisie) < 2 || strlen(saisie) > 3) {
            printf("Format invalide. Utilisez une lettre (A-J) et un chiffre (1-10)\n");
            continue;
        }
        
        colonne_char = saisie[0];
        if(colonne_char >= 'a' && colonne_char <= 'j') {
            *col = colonne_char - 'a';
        } else if(colonne_char >= 'A' && colonne_char <= 'J') {
            *col = colonne_char - 'A';
        } else {
            printf("Colonne invalide. Utilisez A-J\n");
            continue;
        }
        
        ligne_saisie = atoi(&saisie[1]);
        if(ligne_saisie < 1 || ligne_saisie > 10) {
            printf("Ligne invalide. Utilisez 1-10\n");
            continue;
        }
        
        *ligne = ligne_saisie - 1;
        valide = 1;
    }
}

void lire_bateau(int *ligne, int *col, char *orient) {
    char saisie[10];
    int valide = 0;
    int ligne_saisie;
    char colonne_char;
    
    while(!valide) {
        printf("Position (col ligne): ");
        scanf("%s", saisie);
        
        if(strlen(saisie) < 2 || strlen(saisie) > 3) {
            printf("Format invalide. Utilisez une lettre (A-J) et un chiffre (1-10)\n");
            continue;
        }
        
        colonne_char = saisie[0];
        if(colonne_char >= 'a' && colonne_char <= 'j') {
            *col = colonne_char - 'a';
        } else if(colonne_char >= 'A' && colonne_char <= 'J') {
            *col = colonne_char - 'A';
        } else {
            printf("Colonne invalide. Utilisez A-J\n");
            continue;
        }
        
        ligne_saisie = atoi(&saisie[1]);
        if(ligne_saisie < 1 || ligne_saisie > 10) {
            printf("Ligne invalide. Utilisez 1-10\n");
            continue;
        }
        
        *ligne = ligne_saisie - 1;
        valide = 1;
    }
    
    valide = 0;
    while(!valide) {
        printf("Orientation (H/V): ");
        scanf(" %c", orient);
        
        if(*orient == 'h') *orient = 'H';
        if(*orient == 'v') *orient = 'V';
        
        if(*orient != 'H' && *orient != 'V') {
            printf("Orientation invalide. Utilisez H ou V\n");
            continue;
        }
        
        valide = 1;
    }
}
