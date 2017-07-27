# Make file for solver and parser
CFLAGS = -std=c++11 -Wall -O3
CC = g++-4.9
CC49 = g++-4.9
TARGET = ../build/

all: parser solver

solver: FaultGraph.o primitive.o utilities.o ../build/intermediate.o
	$(CC) -o $(TARGET)$@ $(CFLAGS) $^
parser: parser.o
	$(CC) -o $(TARGET)$@ ${CFLAG} $^
.cpp.o:
	$(CC) $(CFLAGS) -c $<
../build/intermediate.o: ../build/intermediate.cpp
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(TARGET)* *.o


