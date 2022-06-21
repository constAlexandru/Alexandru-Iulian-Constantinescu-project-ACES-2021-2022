C_FILE=$(shell find -name "code.c")
C_FILE:
	gcc code.c -lpng -fopenmp -O3 -o prog
    