
# On MacOS, install raylib with homebrew
# On Linux, it is part of the source code of this project
# On Windows, I really don't care what you do, but I'm not doing anything

ifeq ($(shell uname -s), Darwin)
	CC ?= clang
	RAYLIB = `pkg-config --cflags --libs raylib`
else
	CC ?= gcc
	RAYLIB = -L./raylib-5.0_linux_amd64/lib/ -l:libraylib.a -lm -I./raylib-5.0_linux_amd64/include/
endif

app: app.c
	$(CC) app.c $(RAYLIB) -o app

run: app
	./app
