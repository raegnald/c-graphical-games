
ifeq ($(shell uname -s), Darwin)
	CC ?= clang
else
	CC ?= gcc
endif

RAYLIB = `pkg-config --libs raylib`

EXE = app

SRC = $(wildcard *.c)
OBJ = $(SRC:.d=.o)

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(RAYLIB) -o $(EXE)

%.o: %.d
	$(DCOMP) $< -o $@ $(RAYLIB)

run: $(EXE)
	./$(EXE)

clean:
	rm -f $(OBJ) $(EXE)
