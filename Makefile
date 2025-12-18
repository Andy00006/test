# Nom de l'exécutable final
EXEC = water_proc

# Compilateur et options (O3 pour optimiser la vitesse sur les 8 millions de lignes)
CC = gcc
CFLAGS = -Wall -Wextra -O3

# Règle par défaut
all: $(EXEC)

# Compilation de l'exécutable
$(EXEC): main.o
	$(CC) $(CFLAGS) -o $(EXEC) main.o

# Compilation de l'objet main.o
main.o: main.c
	$(CC) $(CFLAGS) -c main.c

# Règle pour nettoyer le dossier (supprime l'exécutable et les fichiers .o)
clean:
	rm -f *.o $(EXEC)

# Règle pour forcer la recompilation
re: clean all
