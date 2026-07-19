CC=gcc
C_FLAGS=-Wall -Wextra

RASTERLIB_INC := -I./include/
RASTERLIB_LIB := -L./lib/

RASTERLIB_SRC := $(wildcard ./src/*.c)

RASTERLIB_OBJ := $(patsubst %.c, %.o, ${RASTERLIB_SRC})

MAIN_SRC = main.c

LIBS = -lSDL2 -lm
MAIN_LIBS = -L. -lSDL2 -lrasterlib -lm

all : rasterlib-main

./src/%.o: ./src/%.c
	${CC} ${C_FLAGS} ${RASTERLIB_LIB} ${RASTERLIB_INC} -c $< -o $@ ${LIBS}

librasterlib.a: ${RASTERLIB_OBJ}
	ar rcs librasterlib.a ${RASTERLIB_OBJ}

rasterlib-main: ${MAIN_SRC} librasterlib.a
	${CC} ${C_FLAGS} ${RASTERLIB_LIB} ${RASTERLIB_INC} -o main ${MAIN_SRC} ${MAIN_LIBS}

clean:
	rm ${RASTERLIB_OBJ}
