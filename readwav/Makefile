CC=g++
CFLAGS=-c
LDFLAGS=
SOURCES=wavreader.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=wavreader

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
 
clean:
	rm -rf *o wavreader
