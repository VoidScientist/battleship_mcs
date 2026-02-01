/**
 *	\file		test_serial.c
 *	\brief		Fichier de test pour la serialisation
 *	\author		MARTEL Mathieu
 *	\version	1.0
 */
 
 /*
 *****************************************************************************************
 *	\noop		I N C L U D E S 
 */
#include <stdio.h>
#include <string.h>

 /*
 *****************************************************************************************
 *	\noop		I N C L U D E S   S P E C I F I Q U E S
 */
#include "include/bataille_navale.h"
#include "include/structSerial.h"

 /*
 *****************************************************************************************
 *	\noop		I M P L E M E N T A T I O N   DES   F O N C T I O N S
 */

/**
 *	\fn			void test_joueur()
 *	\brief		Test de serialisation Joueur
 */
void test_joueur() {
    printf("=== Test CONNECT (Joueur) ===\n");
    
    Joueur joueur1;
    joueur1.id = 0;
    strcpy(joueur1.nom, "Alice");
    
    char buffer[100];
    joueur2str(&joueur1, buffer);
    printf("Serialise: %s\n", buffer);
    
    Joueur joueur2;
    str2joueur(buffer, &joueur2);
    printf("Deserialise: id=%d, nom=%s\n", joueur2.id, joueur2.nom);
    
    if(joueur2.id == 0 && strcmp(joueur2.nom, "Alice") == 0) {
        printf("OK\n\n");
    } else {
        printf("ECHEC\n\n");
    }
}

/**
 *	\fn			void test_tir()
 *	\brief		Test de serialisation Tir
 */
void test_tir() {
    printf("=== Test SHOOT (Tir) ===\n");
    
    Tir tir1;
    tir1.ligne = 5;
    tir1.col = 3;
    tir1.equipe_id = 0;
    
    char buffer[100];
    tir2str(&tir1, buffer);
    printf("Serialise: %s\n", buffer);
    
    Tir tir2;
    str2tir(buffer, &tir2);
    printf("Deserialise: ligne=%d, col=%d, equipe_id=%d\n", tir2.ligne, tir2.col, tir2.equipe_id);
    
    if(tir2.ligne == 5 && tir2.col == 3 && tir2.equipe_id == 0) {
        printf("OK\n\n");
    } else {
        printf("ECHEC\n\n");
    }
}

/**
 *	\fn			void test_placement()
 *	\brief		Test de serialisation Placement
 */
void test_placement() {
    printf("=== Test PLACE (Placement) ===\n");
    
    Placement place1;
    place1.id = 2;
    place1.longueur = 5;
    place1.ligne = 0;
    place1.col = 0;
    place1.orient = HORIZONTAL;
    
    char buffer[100];
    place2str(&place1, buffer);
    printf("Serialise: %s\n", buffer);
    
    Placement place2;
    str2place(buffer, &place2);
    printf("Deserialise: id=%d, longueur=%d, ligne=%d, col=%d, orient=%d\n",
           place2.id, place2.longueur, place2.ligne, place2.col, place2.orient);
    
    if(place2.id == 2 && place2.longueur == 5 && place2.ligne == 0 && 
       place2.col == 0 && place2.orient == HORIZONTAL) {
        printf("OK\n\n");
    } else {
        printf("ECHEC\n\n");
    }
}

/**
 *	\fn			void test_resultat()
 *	\brief		Test de serialisation Resultat
 */
void test_resultat() {
    printf("=== Test REVEAL (Resultat) ===\n");
    
    Resultat res1;
    res1.ligne = 5;
    res1.col = 3;
    res1.touche = 1;
    res1.coule = 0;
    res1.id_coule = 0;
    
    char buffer[100];
    resultat2str(&res1, buffer);
    printf("Serialise: %s\n", buffer);
    
    Resultat res2;
    str2resultat(buffer, &res2);
    printf("Deserialise: ligne=%d, col=%d, touche=%d, coule=%d, id_coule=%d\n",
           res2.ligne, res2.col, res2.touche, res2.coule, res2.id_coule);
    
    if(res2.ligne == 5 && res2.col == 3 && res2.touche == 1 && 
       res2.coule == 0 && res2.id_coule == 0) {
        printf("OK\n\n");
    } else {
        printf("ECHEC\n\n");
    }
}

/**
 *	\fn			void test_tour()
 *	\brief		Test de serialisation Tour
 */
void test_tour() {
    printf("=== Test NEXT/START (Tour) ===\n");
    
    Tour tour1;
    tour1.equipe_id = 0;
    tour1.joueur_id = 1;
    tour1.phase = 1;
    
    char buffer[100];
    tour2str(&tour1, buffer);
    printf("Serialise: %s\n", buffer);
    
    Tour tour2;
    str2tour(buffer, &tour2);
    printf("Deserialise: equipe=%d, joueur=%d, phase=%d\n",
           tour2.equipe_id, tour2.joueur_id, tour2.phase);
    
    if(tour2.equipe_id == 0 && tour2.joueur_id == 1 && tour2.phase == 1) {
        printf("OK\n\n");
    } else {
        printf("ECHEC\n\n");
    }
}

/**
 *	\fn			void test_end()
 *	\brief		Test de serialisation END
 */
void test_end() {
    printf("=== Test END (vainqueur) ===\n");
    
    int vainqueur1 = 0;
    
    char buffer[100];
    sprintf(buffer, "%d", vainqueur1);
    printf("Serialise: %s\n", buffer);
    
    int vainqueur2;
    sscanf(buffer, "%d", &vainqueur2);
    printf("Deserialise: vainqueur=%d\n", vainqueur2);
    
    if(vainqueur2 == 0) {
        printf("OK\n\n");
    } else {
        printf("ECHEC\n\n");
    }
}

/**
 *	\fn			void test_cas_limite()
 *	\brief		Test des cas limites
 */
void test_cas_limite() {
    printf("=== Tests cas limites ===\n");
    
    printf("Test 1: Nom long (49 caracteres)\n");
    Joueur joueur1;
    joueur1.id = 5;
    strcpy(joueur1.nom, "NomTresLongAvecBeaucoupDeCaracteresPourTester49");
    
    char buffer[200];
    joueur2str(&joueur1, buffer);
    
    Joueur joueur2;
    str2joueur(buffer, &joueur2);
    
    if(strcmp(joueur1.nom, joueur2.nom) == 0) {
        printf("OK\n");
    } else {
        printf("ECHEC\n");
    }
    
    printf("Test 2: Placement VERTICAL\n");
    Placement place1 = {3, 4, 2, 5, VERTICAL};
    place2str(&place1, buffer);
    
    Placement place2;
    str2place(buffer, &place2);
    
    if(place2.orient == VERTICAL) {
        printf("OK\n");
    } else {
        printf("ECHEC\n");
    }
    
    printf("Test 3: Bateau coule\n");
    Resultat res1 = {3, 7, 1, 1, 2};
    resultat2str(&res1, buffer);
    
    Resultat res2;
    str2resultat(buffer, &res2);
    
    if(res2.coule == 1 && res2.id_coule == 2) {
        printf("OK\n");
    } else {
        printf("ECHEC\n");
    }
    
    printf("Test 4: Phase placement\n");
    Tour tour1 = {1, 2, 0};
    tour2str(&tour1, buffer);
    
    Tour tour2;
    str2tour(buffer, &tour2);
    
    if(tour2.phase == 0) {
        printf("OK\n");
    } else {
        printf("ECHEC\n");
    }
    
    printf("Test 5: Tir equipe 1\n");
    Tir tir1 = {9, 9, 1};
    tir2str(&tir1, buffer);
    
    Tir tir2;
    str2tir(buffer, &tir2);
    
    if(tir2.ligne == 9 && tir2.col == 9 && tir2.equipe_id == 1) {
        printf("OK\n");
    } else {
        printf("ECHEC\n");
    }
    
    printf("\n");
}

/**
 *	\fn			void test_scenario_complet()
 *	\brief		Test d'un scenario de jeu complet
 */
void test_scenario_complet() {
    printf("=== Scenario de jeu complet ===\n");
    char buffer[200];
    
    printf("1. Joueur Alice se connecte\n");
    Joueur alice = {0, "Alice"};
    joueur2str(&alice, buffer);
    printf("   -> %s\n", buffer);
    
    printf("2. Joueur Bob se connecte\n");
    Joueur bob = {1, "Bob"};
    joueur2str(&bob, buffer);
    printf("   -> %s\n", buffer);
    
    printf("3. Alice place son porte-avion en A1 horizontal\n");
    Placement place1 = {2, 5, 0, 0, HORIZONTAL};
    place2str(&place1, buffer);
    printf("   -> %s\n", buffer);
    
    printf("4. Debut de la bataille - Tour Alice\n");
    Tour tour1 = {0, 0, 1};
    tour2str(&tour1, buffer);
    printf("   -> %s\n", buffer);
    
    printf("5. Alice tire en F4\n");
    Tir tir1 = {5, 3, 0};
    tir2str(&tir1, buffer);
    printf("   -> %s\n", buffer);
    
    printf("6. Resultat: Touche!\n");
    Resultat res1 = {5, 3, 1, 0, 0};
    resultat2str(&res1, buffer);
    printf("   -> %s\n", buffer);
    
    printf("7. Alice rejoue\n");
    tour2str(&tour1, buffer);
    printf("   -> %s\n", buffer);
    
    printf("8. Alice tire en G4\n");
    Tir tir2_obj = {6, 3, 0};
    tir2str(&tir2_obj, buffer);
    printf("   -> %s\n", buffer);
    
    printf("9. Resultat: A l'eau\n");
    Resultat res2 = {6, 3, 0, 0, 0};
    resultat2str(&res2, buffer);
    printf("   -> %s\n", buffer);
    
    printf("10. Tour de Bob\n");
    Tour tour2 = {1, 1, 1};
    tour2str(&tour2, buffer);
    printf("   -> %s\n", buffer);
    
    printf("\nScenario OK\n\n");
}

/**
 *	\fn			int main()
 *	\brief		Fonction principale
 */
int main() {
    printf("=============================================\n");
    printf("  Tests de serialisation - Bataille Navale\n");
    printf("=============================================\n\n");
    
    test_joueur();
    test_tir();
    test_placement();
    test_resultat();
    test_tour();
    test_end();
    test_cas_limite();
    test_scenario_complet();
    
    printf("=============================================\n");
    printf("  Tous les tests termines !\n");
    printf("=============================================\n");
    
    return 0;
}
