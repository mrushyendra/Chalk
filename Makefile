CC=g++
CPPFLAGS=-std=c++14 -Wall -pedantic -g3
 
all: solver test

solver: main.o solver.o
	$(CC) main.o solver.o -o $@

test: test.o solver.o
	$(CC) test.o solver.o -o $@

main.o: src/main.cpp src/solver.h
	$(CC) $(CPPFLAGS) -o main.o -c src/main.cpp

test.o: src/test.cpp src/solver.h
	$(CC) $(CPPFLAGS) -o test.o -c src/test.cpp

solver.o: src/solver.cpp src/solver.h
	$(CC) $(CPPFLAGS) -o solver.o -c src/solver.cpp

clean:
	$(RM) solver test *.o 
