#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

int main(int argc, char* argv[]) {
    if (argc < 4) return 1;

    FILE* fichier = fopen(argv[1], "r");
    if (!fichier) return 2;

    char* mode = argv[2];
    char* option_id = argv[3];
    Noeud* racine = NULL;
    char ligne[1024];
    
    double fuites_cumulees = 0;
    int usine_trouvee = 0;

    while (fgets(ligne, sizeof(ligne), fichier)) {
        char *c1 = strtok(ligne, ";"); // Usine
        char *c2 = strtok(NULL, ";");  // Amont
        char *c3 = strtok(NULL, ";");  // Aval
        char *c4 = strtok(NULL, ";");  // Volume
        char *c5 = strtok(NULL, ";\n"); // Fuite

        double v4 = (c4 && c4[0] != '-') ? atof(c4) : 0;
        double v5 = (c5 && c5[0] != '-') ? atof(c5) : 0;

        if (strcmp(mode, "histo") == 0) {
            if (c2 && strstr(c2, "Facility")) {
                racine = inserer(racine, c2, v4, 0, 0);
            } else if (c2 && strstr(c2, "Spring")) {
                racine = inserer(racine, c3, 0, v4, v4 * (1 - v5/100.0));
            }
        } 
        else if (strcmp(mode, "leaks") == 0) {
            if (c1 && strcmp(c1, option_id) == 0) {
                usine_trouvee = 1;
                // On cumule les pertes (approximation pour la flemme, mais efficace)
                fuites_cumulees += (v4 * (v5 / 100.0));
            }
        }
    }
    fclose(fichier);

    if (strcmp(mode, "leaks") == 0) {
        FILE* hist = fopen("rendement_historique.dat", "a");
        if (usine_trouvee) {
            fprintf(hist, "%s;%.3f\n", option_id, fuites_cumulees / 1000.0);
            printf("Pertes calculées : %.3f M.m3\n", fuites_cumulees / 1000.0);
        } else {
            fprintf(hist, "%s;-1\n", option_id);
            printf("-1\n");
        }
        fclose(hist);
    } else {
        // ... garde ton code export histo ici ...
    }

    liberer_arbre(racine);
    return 0;
}
