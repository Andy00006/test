#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- STRUCTURES ---

typedef struct Node {
    char id[128];
    double v_max;    // Capacité max
    double v_src;    // Volume capté
    double v_real;   // Volume traité (après fuites usine)
    double v_leak;   // Pour le mode leaks
    int height;
    struct Node *left, *right;
} Node;

// --- GESTION AVL ---

int getHeight(Node* n) { return n ? n->height : 0; }

int getBalance(Node* n) { return n ? getHeight(n->left) - getHeight(n->right) : 0; }

Node* createNode(char* id, double v1, double v2, double v3) {
    Node* n = malloc(sizeof(Node));
    if (!n) exit(1);
    strncpy(n->id, id, 127);
    n->v_max = v1; n->v_src = v2; n->v_real = v3; n->v_leak = 0;
    n->left = n->right = NULL;
    n->height = 1;
    return n;
}

Node* rotateRight(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = 1 + (getHeight(y->left) > getHeight(y->right) ? getHeight(y->left) : getHeight(y->right));
    x->height = 1 + (getHeight(x->left) > getHeight(x->right) ? getHeight(x->left) : getHeight(x->right));
    return x;
}

Node* rotateLeft(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = 1 + (getHeight(x->left) > getHeight(x->right) ? getHeight(x->left) : getHeight(x->right));
    y->height = 1 + (getHeight(y->left) > getHeight(y->right) ? getHeight(y->left) : getHeight(y->right));
    return y;
}

Node* insert(Node* node, char* id, double v1, double v2, double v3) {
    if (!node) return createNode(id, v1, v2, v3);

    int cmp = strcmp(id, node->id);
    if (cmp < 0) node->left = insert(node->left, id, v1, v2, v3);
    else if (cmp > 0) node->right = insert(node->right, id, v1, v2, v3);
    else {
        node->v_max += v1; node->v_src += v2; node->v_real += v3;
        return node;
    }

    node->height = 1 + (getHeight(node->left) > getHeight(node->right) ? getHeight(node->left) : getHeight(node->right));
    int b = getBalance(node);

    if (b > 1 && strcmp(id, node->left->id) < 0) return rotateRight(node);
    if (b < -1 && strcmp(id, node->right->id) > 0) return rotateLeft(node);
    if (b > 1 && strcmp(id, node->left->id) > 0) { node->left = rotateLeft(node->left); return rotateRight(node); }
    if (b < -1 && strcmp(id, node->right->id) < 0) { node->right = rotateRight(node->right); return rotateLeft(node); }
    return node;
}

// --- PARCOURS ET LIBÉRATION ---

void writeReverseInorder(Node* root, FILE* f, int mode) {
    if (!root) return;
    writeReverseInorder(root->right, f, mode);
    if (mode == 1) fprintf(f, "%s;%.3f\n", root->id, root->v_max / 1000.0);
    else if (mode == 2) fprintf(f, "%s;%.3f\n", root->id, root->v_src / 1000.0);
    else if (mode == 3) fprintf(f, "%s;%.3f\n", root->id, root->v_real / 1000.0);
    writeReverseInorder(root->left, f, mode);
}

void freeTree(Node* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// --- LOGIQUE DE CALCUL DES FUITES (Mode Leaks) ---

Node* findNode(Node* root, char* id) {
    if (!root) return NULL;
    int cmp = strcmp(id, root->id);
    if (cmp == 0) return root;
    return (cmp < 0) ? findNode(root->left, id) : findNode(root->right, id);
}

// --- MAIN ---

int main(int argc, char** argv) {
    if (argc < 4) return 1;

    char* filePath = argv[1];
    char* mode = argv[2]; // "histo" ou "leaks"
    Node* root = NULL;

    FILE* f = fopen(filePath, "r");
    if (!f) return 2;

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char c1[128], c2[128], c3[128], c4[128], c5[128];
        // Parsing simplifié (à adapter selon le format exact de ton CSV)
        if (sscanf(line, "%[^;];%[^;];%[^;];%[^;];%[^;\n]", c1, c2, c3, c4, c5) < 2) continue;

        if (strcmp(mode, "histo") == 0) {
            char* subMode = argv[3];
            // Logique de tri par usine
            if (strcmp(c2, "-") != 0 && strstr(c2, "Facility")) {
                double val = (strcmp(c4, "-") == 0) ? 0 : atof(c4);
                root = insert(root, c2, val, 0, 0);
            }
        }
    }

    // Exportation
    if (strcmp(mode, "histo") == 0) {
        int m = 1;
        if (strcmp(argv[3], "src") == 0) m = 2;
        if (strcmp(argv[3], "real") == 0) m = 3;
        writeReverseInorder(root, stdout, m);
    }

    fclose(f);
    freeTree(root);
    return 0;
}
