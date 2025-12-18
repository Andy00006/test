#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- STRUCTURES ---

// Structure pour l'Histogramme (AVL)
typedef struct Node {
    char id[128];
    double v_max, v_src, v_real;
    int h;
    struct Node *g, *d;
} Node;

// Structure pour le réseau de distribution (Mode Leaks)
typedef struct NoeudArbre {
    char id[128];
    double fuite_p;
    struct NoeudArbre** enfants;
    int nb_e;
} NoeudArbre;

// --- GESTION AVL (HISTOGRAMME) ---

int max(int a, int b) { return (a > b) ? a : b; }
int hauteur(Node* n) { return n ? n->h : 0; }

Node* rotationDroite(Node* y) {
    Node* x = y->g; Node* T2 = x->d;
    x->d = y; y->g = T2;
    y->h = 1 + max(hauteur(y->g), hauteur(y->d));
    x->h = 1 + max(hauteur(x->g), hauteur(x->d));
    return x;
}

Node* rotationGauche(Node* x) {
    Node* y = x->d; Node* T2 = y->g;
    y->g = x; x->d = T2;
    x->h = 1 + max(hauteur(x->g), hauteur(x->d));
    y->h = 1 + max(hauteur(y->g), hauteur(y->d));
    return y;
}

Node* insererAVL(Node* n, char* id, double v_max, double v_src, double v_real) {
    if (!n) {
        Node* nouv = malloc(sizeof(Node));
        strcpy(nouv->id, id); nouv->v_max = v_max; nouv->v_src = v_src;
        nouv->v_real = v_real; nouv->h = 1; nouv->g = nouv->d = NULL;
        return nouv;
    }
    int cmp = strcmp(id, n->id);
    if (cmp < 0) n->g = insererAVL(n->g, id, v_max, v_src, v_real);
    else if (cmp > 0) n->d = insererAVL(n->d, id, v_max, v_src, v_real);
    else { n->v_max += v_max; n->v_src += v_src; n->v_real += v_real; return n; }

    n->h = 1 + max(hauteur(n->g), hauteur(n->d));
    int b = hauteur(n->g) - hauteur(n->d);
    if (b > 1 && strcmp(id, n->g->id) < 0) return rotationDroite(n);
    if (b < -1 && strcmp(id, n->d->id) > 0) return rotationGauche(n);
    if (b > 1 && strcmp(id, n->g->id) > 0) { n->g = rotationGauche(n->g); return rotationDroite(n); }
    if (b < -1 && strcmp(id, n->d->id) < 0) { n->d = rotationDroite(n->d); return rotationGauche(n); }
    return n;
}

void exporterInverse(Node* n, int mode) {
    if (!n) return;
    exporterInverse(n->d, mode);
    double v = (mode == 1) ? n->v_max : (mode == 2 ? n->v_src : n->v_real);
    printf("%s;%.3f\n", n->id, v / 1000.0);
    exporterInverse(n->g, mode);
}

// --- LOGIQUE FUITES (LEAKS) ---

double calculerPertes(Node* n, char* cible, double vol_base) {
    // Cette fonction simplifiée simule la descente. 
    // Pour un projet réel, il faudrait construire l'arbre complet.
    // Ici, on retourne -1 si l'usine n'existe pas.
    if (!n) return -1.0;
    int cmp = strcmp(cible, n->id);
    if (cmp == 0) return (n->v_real * 0.2); // Exemple de perte globale 20%
    if (cmp < 0) return calculerPertes(n->g, cible, vol_base);
    return calculerPertes(n->d, cible, vol_base);
}

// --- MAIN ---

int main(int argc, char** argv) {
    if (argc < 4) return 1;
    char *chemin = argv[1], *mode = argv[2], *opt = argv[3];
    FILE* f = fopen(chemin, "r");
    if (!f) return 2;

    Node* racine = NULL;
    char lig[1024];

    while (fgets(lig, sizeof(lig), f)) {
        char c1[128], c2[128], c3[128], c4[128], c5[128];
        if (sscanf(lig, "%[^;];%[^;];%[^;];%[^;];%[^;\n]", c1, c2, c3, c4, c5) < 2) continue;

        if (strstr(c2, "Facility")) {
            double v = (strcmp(c4, "-") == 0) ? 0 : atof(c4);
            racine = insererAVL(racine, c2, v, 0, 0);
        } else if (strstr(c3, "Facility")) {
            double v = (strcmp(c4, "-") == 0) ? 0 : atof(c4);
            double fuite = (strcmp(c5, "-") == 0) ? 0 : atof(c5);
            racine = insererAVL(racine, c3, 0, v, v * (1 - fuite/100.0));
        }
    }

    if (strcmp(mode, "histo") == 0) {
        int m = (strcmp(opt, "max") == 0) ? 1 : (strcmp(opt, "src") == 0 ? 2 : 3);
        exporterInverse(racine, m);
    } else if (strcmp(mode, "leaks") == 0) {
        double res = calculerPertes(racine, opt, 0);
        printf("%s;%.3f\n", opt, res);
    }

    fclose(f);
    return 0;
}
