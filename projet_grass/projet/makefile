CC		= gcc
CFLAGS	= -Wall -O2 -g
LDFLAGS	= -lglfw -lGLU -lGL -lm

BINDIR	= bin/
SRCDIR	= src/
OBJDIR	= obj/

# Fichiers Projet
OBJ_PROJET_GRASS= grass_projet_synthese_d_images.o
EXEC_PROJET_GRASS= grass_projet_synthese_d_images.out

# Fichiers Main
OBJ_MAIN= main.o
EXEC_MAIN= main.out

# Regles compilation Projet

all : grass_projet_synthese_d_images

projet : $(OBJDIR)$(OBJ_PROJET_GRASS)
	$(CC) $(CFLAGS) $(OBJDIR)$(OBJ_PROJET_GRASS) -o $(BINDIR)$(EXEC_PROJET_GRASS) $(LDFLAGS)

game : $(OBJDIR)$(OBJ_MAIN)
	$(CC) $(CFLAGS) $(OBJDIR)$(OBJ_MAIN) -o $(BINDIR)$(EXEC_MAIN) $(LDFLAGS)

clean :
	rm -rf *~
	rm -rf $(SRCDIR)*/*~
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)*

$(OBJDIR)%.o: $(SRCDIR)%.c
	mkdir -p `dirname $@`
	$(CC) -o $@ -c $< $(CFLAGS)
