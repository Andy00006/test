#!/bin/bash

# 1. Capture du temps de début (en millisecondes)
debut=$(date +%s%3N)

# --- FONCTIONS ---

afficher_usage() {
    echo "Usage: $0 <fichier_donnees> <mode> <option/ID>"
    echo "Modes disponibles :"
    echo "  histo <max|src|real>"
    echo "  leaks <ID_Usine>"
    exit 1
}

# --- VÉRIFICATIONS DES ARGUMENTS ---

# Vérification du nombre minimal d'arguments
if [ "$#" -lt 3 ]; then
    afficher_usage
fi

FICHIER_CSV=$1
MODE=$2
OPTION=$3

# Vérification de l'existence du fichier de données
if [ ! -f "$FICHIER_CSV" ]; then
    echo "Erreur : Le fichier '$FICHIER_CSV' est introuvable."
    exit 2
fi

# --- COMPILATION ---

# On vérifie si l'exécutable existe, sinon on appelle make
if [ ! -f "./water_processor" ]; then
    echo "Exécutable introuvable. Lancement de la compilation via make..."
    make
    if [ $? -ne 0 ]; then
        echo "Erreur fatale : La compilation a échoué."
        exit 3
    fi
fi

# --- EXÉCUTION DU PROGRAMME C ---

echo "Traitement en cours... (veuillez patienter)"

# On lance le programme C avec les arguments fournis
./water_processor "$FICHIER_CSV" "$MODE" "$OPTION"
CODE_RETOUR=$?

if [ $CODE_RETOUR -ne 0 ]; then
    echo "Le programme C a rencontré une erreur (Code: $CODE_RETOUR)."
    # On continue quand même pour afficher le temps final
else
    echo "Traitement C terminé avec succès."
fi

# --- GÉNÉRATION DES GRAPHIQUES (Si mode histo) ---

if [ "$MODE" == "histo" ] && [ $CODE_RETOUR -eq 0 ]; then
    FICHIER_DAT="resultat_${OPTION}.dat"
    
    # On utilise Gnuplot pour générer les PNG
    # On prépare un petit script gnuplot à la volée
    gnuplot << EOF
        set datafile separator ";"
        set terminal png size 800,600
        set style data histograms
        set style fill solid
        set xtics rotate by -45
        
        # 10 plus grandes usines
        set output "${OPTION}_top10.png"
        set title "Top 10 des plus grandes usines (${OPTION})"
        plot "< sort -t';' -k2 -rn ${FICHIER_DAT} | head -n 10" using 2:xtic(1) title "Volume (M.m3)"
        
        # 50 plus petites usines
        set output "${OPTION}_bot50.png"
        set title "50 plus petites usines (${OPTION})"
        plot "< sort -t';' -k2 -n ${FICHIER_DAT} | tail -n +2 | head -n 50" using 2:xtic(1) title "Volume (M.m3)"
EOF
    echo "Graphiques générés : ${OPTION}_top10.png et ${OPTION}_bot50.png"
fi

if [ ! -f "./water_processor" ]; then
    make
fi

# --- FIN ET CHRONOMÈTRE ---

fin=$(date +%s%3N)
duree=$((fin - debut))

echo "-------------------------------------------"
echo "Durée totale du script : $duree ms"
echo "-------------------------------------------"
