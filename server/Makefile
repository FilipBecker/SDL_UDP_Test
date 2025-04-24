SRCDIR = ./src
COMMONSRCDIR = ../lib/src
INCDIR = ../lib/include
CC=gcc
INCLUDE = C:\msys64\mingw64\include\SDL2 -I$(INCDIR)

CFLAGS = -g -I$(INCLUDE) -c 
LDFLAGS = -lmingw32 -lSDL2main -lSDL2_image -lSDL2 -lSDL2_ttf -lSDL2_net -lm

eget: main.o text.o
	$(CC) -o egetServer main.o text.o $(LDFLAGS)

main.o: $(SRCDIR)\main.c $(INCDIR)/text.h
	$(CC) $(CFLAGS) $(SRCDIR)\main.c

text.o: $(COMMONSRCDIR)/text.c $(INCDIR)/text.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/text.c