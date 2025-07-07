BINARY=bin
CODEDIRS=. src
INCDIRS=. ./include

CC=g++
OPT=-O0

DEPFLAGS=-MP -MD
CFLAGS=-Wall -Wextra -g $(foreach D, $(INCDIRS), -I$(D)) $(OPT) $(DEPFLAGS)

CFILES=$(foreach D, $(CODEDIRS), $(wildcard $(D)/*.cpp))

OBJECTS=$(patsubst %.cpp, %.o, $(CFILES))
DEPFILES=$(patsubst %.cpp, %.d, $(CFILES))

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY) $(DEPFILES) $(OBJECTS)

distribute: clean
	tar zvcf dist.tgz *

-include $(DEPFILES)