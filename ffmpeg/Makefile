CC=g++
CFLAGS=-c -std=c++11
LDFLAGS=
LIBS= -lavutil -lavcodec -lavformat -L/usr/local/lib 
SOURCES=ffmp.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=ffmp

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
 
clean:
	rm -rf *o $(EXECUTABLE)
