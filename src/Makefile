.phony: all

all: program.out
	./program.out

program.out: viewer.o app.o
	g++ viewer.o app.o -o program.out -lGL -lGLU -lglut -lm

app.o: app.cpp defs.h
	g++ -c app.cpp -o app.o -O3

viewer.o: viewer.cpp defs.h
	g++ -c viewer.cpp -o viewer.o -O3