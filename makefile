IDIR = ./include
SDIR = ./src
ODIR = ./obj

.PHONY: clean mrproper

CC = gcc
CFLAGS = -g -Wall -I$(IDIR)

_DEPS = fat.h shell.h arquivo.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = fat.o shell.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: sisop_tp2

# Garante que a pasta obj exista antes de compilar objetos
$(ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Regra para criar pasta obj
$(ODIR):
	mkdir -p $(ODIR)

sisop_tp2: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(ODIR)/*.o

mrproper: clean
	rm -f sisop_tp2
