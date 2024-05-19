RAYLIB_PATH=./raylib-5.0
RAYLIB_FLAGS=-I${RAYLIB_PATH}/src -L${RAYLIB_PATH}/src  -l:libraylib.so -Wl,-rpath=./raylib-5.0/src ${FLAGS}
FLAGS= -Wall -Wextra -lm -ldl -ggdb

all: build_main run

build_main: src/main.c build_plugin
	cc src/main.c ${RAYLIB_FLAGS} -o build/main

build_plugin: src/plugin.c
	cc -c -fPIC src/plugin.c ${RAYLIB_FLAGS} -ggdb -o build/plugin.o
	cc build/plugin.o -shared ${RAYLIB_FLAGS} -o build/libplugin.so

run: build/main
	./build/main

hotreload: build_plugin