CC = g++
CFLAGS = -std=c++17 -Wall -Werror -pedantic -g -O3 -pthread

# .hpp files
DEPS = network.hpp call.hpp

# compiled .o files (everything except voip.cpp, which has main)
OBJECTS = network.o call.o

# name of the program
PROGRAM = voip
LIBRARY = voip.a

all: $(PROGRAM)
.PHONY: all clean

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(PROGRAM): voip.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $^

$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^

clean:
	rm -f *.o $(PROGRAM) $(LIBRARY)