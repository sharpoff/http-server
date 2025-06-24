CXX = g++
SOURCEDIR = src
TARGET = main
CFLAGS = -Wall -g -fsanitize=address,undefined

SOURCES = $(shell find $(SOURCEDIR) -name '*.cpp')
OBJECTS = $(SOURCES:%.cpp=%.o)

all: main

%.o: %.c
	$(CXX) -c $^ -o $@ $(CFLAGS)

main: $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(CFLAGS)
	
.PHONY : clean
clean:
	rm -f $(TARGET) $(OBJECTS)
