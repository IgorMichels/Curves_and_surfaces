.phony: all

all: viewer.out
	./viewer.out

viewer.out: viewer.cpp viewer.h
	g++ viewer.cpp -o viewer.out -O3 -lGL -lGLU -lglut -lm -lGLEW