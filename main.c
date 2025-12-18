#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <fichier> <mode> <option>\n", argv[0]);
        return 1;
    }

    FILE* fichier = fopen(argv[1], "r");
    if (!fichier) return 2;

    char* mode = argv[2];
    Noeud* racine = NULL;
    char ligne[1024];

    // Lecture optimisée du fichier ligne par ligne
    while (fgets(ligne, sizeof(ligne), fichier)) {
        // On ignore les lignes vides ou incomplètes
        if (ligne[0] == '-' || ligne[0] == '\n') continue;

        char *c1 = strtok(ligne, ";"); // Usine ID (tronçons avals)
        char *c2 = strtok(NULL, ";");  // Amont ID
        char *c3 = strtok(NULL, ";");  // Aval ID
        char *c4 = strtok(NULL, ";");  // Volume / Capacité
        char *c5 = strtok(NULL, ";\n"); // % Fuite

        double val4 = (c4 && c4[0] != '-') ? atof(c4) : 0;
        double val5 = (c5 && c5[0] != '-') ? atof(c5) : 0;

        if (strcmp(mode, "histo") == 0) {
            // Cas 1 : Ligne descriptive de l'usine (Capacité Max)
            if (c2 && strstr(c2, "Facility") && (!c3 || c3[0] == '-')) {
                racine = inserer(racine, c2, val4, 0, 0);
            }
            // Cas 2 : Ligne Source -> Usine (Captage)
            else if (c2 && strstr(c2, "Spring")) {
                double traite = val4 * (1.0 - (val5 / 100.0));
                racine = inserer(racine, c3, 0, val4, traite);
            }
        }
    }

    // Exportation des données traitées
    char nom_sortie[128];
    sprintf(nom_sortie, "resultat_%s.dat", argv[3]);
    FILE* flux_out = fopen(nom_sortie, "w");
    if (flux_out) {
        fprintf(flux_out, "identifiant;volume_max;volume_capte;volume_traite\n");
        exporter_infixe_inverse(racine, flux_out);
        fclose(flux_out);
    }

    fclose(fichier);
    liberer_arbre(racine);
    return 0;
}
