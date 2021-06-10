.phony: all clean

all: prog.out
	./prog.out

prog.out: main.o surface.o viewer.o
	g++ -O3 main.o surface.o viewer.o -o prog.out -ldl -lGL -lGLU -lglut -lm

main.o: main.cpp surface.h viewer.h types.h
	g++ -c -O3 main.cpp -o main.o

surface.o: surface.cpp surface.h types.h
	g++ -c -O3 surface.cpp -o surface.o

viewer.o: viewer.cpp viewer.h
	g++ -c -O3 viewer.cpp -o viewer.o

clean:
	-rm *.o *.out output.*
	clear