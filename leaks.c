#include "avl.h"
#include <stdlib.h>
#include <string.h>

// Fonction pour créer un noeud de distribution
NoeudDist* creer_noeud_dist(char* id, double fuite) {
    NoeudDist* n = malloc(sizeof(NoeudDist));
    strcpy(n->id, id);
    n->pct_fuite = fuite;
    n->nb_enfants = 0;
    n->capacite_tab = 10;
    n->enfants = malloc(sizeof(NoeudDist*) * n->capacite_tab);
    return n;
}

// Calcul récursif avec répartition équitable
double calculer_fuites_totales(NoeudDist* n, double vol_entrant) {
    if (!n) return 0;
    
    double perte_ici = vol_entrant * (n->pct_fuite / 100.0);
    double vol_restant = vol_entrant - perte_ici;
    
    if (n->nb_enfants == 0) return perte_ici;
    
    double vol_par_enfant = vol_restant / n->nb_enfants;
    double cumul_aval = 0;
    for(int i = 0; i < n->nb_enfants; i++) {
        cumul_aval += calculer_fuites_totales(n->enfants[i], vol_par_enfant);
    }
    return perte_ici + cumul_aval;
}
