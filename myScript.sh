#!/bin/bash

# 1. Capture du temps de début
debut=$(date +%s%3N)

# --- FONCTIONS ---
afficher_usage() {
    echo "Usage: $0 <fichier_donnees> <mode> <option/ID>"
    echo "Modes disponibles :"
    echo "  histo <max|src|real>"
    echo "  leaks <ID_Usine>"
    exit 1
}

# --- VÉRIFICATIONS ---
if [ "$#" -lt 3 ]; then afficher_usage; fi

FICHIER_CSV=$1
MODE=$2
OPTION=$3

if [ ! -f "$FICHIER_CSV" ]; then
    echo "Erreur : Le fichier '$FICHIER_CSV' est introuvable."
    exit 2
fi

# --- COMPILATION ---
if [ ! -f "./water_processor" ]; then
    echo "Compilation en cours..."
    make
fi

# --- EXÉCUTION ---
echo "Traitement C en cours..."
./water_processor "$FICHIER_CSV" "$MODE" "$OPTION"
CODE_RETOUR=$?

if [ $CODE_RETOUR -ne 0 ]; then
    echo "Erreur lors de l'exécution du programme C."
    exit $CODE_RETOUR
fi

# --- GÉNÉRATION DES GRAPHIQUES ---
if [ "$MODE" == "histo" ]; then
    FICHIER_DAT="resultat_${OPTION}.dat"
    
    # Choix de la colonne selon l'option : 2=max, 3=src, 4=real
    COL=2
    [ "$OPTION" == "src" ] && COL=3
    [ "$OPTION" == "real" ] && COL=4

    echo "Génération des graphiques pour l'option : $OPTION (colonne $COL)..."

    gnuplot << EOF
        set datafile separator ";"
        set terminal png size 1000,700
        set style data histograms
        set style fill solid 1.0
        set xtics rotate by -45 font ",8"
        set grid y
        set ylabel "Volume (M.m3)"
        
        # Top 10
        set output "${OPTION}_top10.png"
        set title "Top 10 des plus grandes usines - $OPTION"
        plot "< sort -t';' -k${COL} -rn ${FICHIER_DAT} | head -n 10" using ${COL}:xtic(1) title "Volume"
        
        # Bot 50 (on trie par ordre croissant et on filtre les valeurs > 0 avec awk)
        set output "${OPTION}_bot50.png"
        set title "50 plus petites usines (hors 0) - $OPTION"
        plot "< sort -t';' -k${COL} -n ${FICHIER_DAT} | awk -F';' '\$${COL} > 0' | head -n 50" using ${COL}:xtic(1) title "Volume"
EOF
    echo "Graphiques créés : ${OPTION}_top10.png et ${OPTION}_bot50.png"
fi

# --- FIN ET CHRONOMÈTRE ---
fin=$(date +%s%3N)
duree=$((fin - debut))

echo "-------------------------------------------"
echo "Traitement terminé en : $duree ms"
echo "-------------------------------------------"
