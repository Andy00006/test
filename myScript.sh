#!/bin/bash

# Vérification des arguments
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 <fichier.csv> <histo|leaks> <option|id_usine>"
    exit 1
fi

DATA_FILE=$1
MODE=$2
PARAM=$3

# Compilation automatique
if [ ! -f "prog_c" ]; then
    gcc -O3 main.c -o prog_c
fi

# Mesure du temps
START_TIME=$(date +%s%N)

# Exécution du programme C
if [ "$MODE" == "histo" ]; then
    OUTPUT_FILE="vol_${PARAM}.dat"
    echo "identifier;volume" > "$OUTPUT_FILE"
    ./prog_c "$DATA_FILE" "histo" "$PARAM" >> "$OUTPUT_FILE"
elif [ "$MODE" == "leaks" ]; then
    OUTPUT_FILE="leaks.dat"
    ./prog_c "$DATA_FILE" "leaks" "$PARAM" >> "$OUTPUT_FILE"
fi

END_TIME=$(date +%s%N)
DURATION=$(( (END_TIME - START_TIME) / 1000000 ))

echo "Traitement terminé en $DURATION ms."
