CC=gcc
C_FLAGS=-Wall -Wextra -fpermissive

RASTERLIB_INC := -I./include/

RASTERLIB_SRC := $(wildcard ./src/*.c)

RASTERLIB_OBJ := $(patsubst %.c, %.o, ${RASTERLIB_SRC})

MAIN_SRC = main.c

LIBS = -lm
MAIN_LIBS = -L. -lSDL2 -lrasterlib -lm

${RASTERLIB_OBJ} : ${RASTERLIB_SRC}
	${CC} ${C_FLAGS} ${RASTERLIB_INC} -c $< -o $@ ${LIBS}

librasterlib.a: ${RASTERLIB_OBJ}
	ar rcs librasterlib.a ${RASTERLIB_OBJ}

rasterlib-main: ${MAIN_SRC} librasterlib.a
	${CC} ${C_FLAGS} ${RASTERLIB_INC} -o main ${MAIN_SRC} ${MAIN_LIBS}

clean:
	rm ${RASTERLIB_OBJ}
