CC=g++
CPPFLAGS=-std=c++14 -Wall -pedantic -g3
 
all: solver test

solver: main.o solver.o
	$(CC) main.o solver.o -o $@

test: test.o solver.o
	$(CC) test.o solver.o -o $@

main.o: main.cpp solver.h
	$(CC) $(CPPFLAGS) -o main.o -c main.cpp

test.o: test.cpp solver.h
	$(CC) $(CPPFLAGS) -o test.o -c test.cpp

solver.o: solver.cpp solver.h
	$(CC) $(CPPFLAGS) -o solver.o -c solver.cpp

clean:
	$(RM) solver test *.o 
