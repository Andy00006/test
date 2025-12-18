#!/bin/bash

# Vérification
if [ "$#" -lt 3 ]; then
    echo "Erreur : Paramètres manquants."
    exit 1
fi

FICHIER=$1; MODE=$2; OPTION=$3
DEBUT=$(date +%s%3N)

# Compilation automatique (Makefile requis)
make > /dev/null

if [ "$MODE" == "histo" ]; then
    NOM_OUT="vol_${OPTION}.dat"
    echo "identifier;volume" > "$NOM_OUT"
    ./water_proc "$FICHIER" histo "$OPTION" >> "$NOM_OUT"
    
    # Tri pour Gnuplot
    grep -v "identifier" "$NOM_OUT" | sort -t';' -k2 -rn > temp.dat
    head -n 10 temp.dat > top10.dat
    tail -n 50 temp.dat > bottom50.dat

    # Gnuplot
    gnuplot << EOF
        set terminal png size 1000,600
        set datafile separator ";"
        set style fill solid
        set xtics rotate by -45
        set output "top10_${OPTION}.png"
        set title "Les 10 plus grandes usines ($OPTION)"
        plot "top10.dat" using 2:xtic(1) with boxes notitle
        
        set output "bottom50_${OPTION}.png"
        set title "Les 50 plus petites usines ($OPTION)"
        plot "bottom50.dat" using 2:xtic(1) with boxes notitle
EOF
    rm temp.dat top10.dat bottom50.dat
    echo "Images générées avec succès."

elif [ "$MODE" == "leaks" ]; then
    ./water_proc "$FICHIER" leaks "$OPTION" >> "rendement.dat"
    echo "Résultat ajouté à rendement.dat"
fi

FIN=$(date +%s%3N)
echo "Durée totale : $((FIN - DEBUT)) ms"
