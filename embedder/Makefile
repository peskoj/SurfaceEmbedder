include Makefile.profiles

all: code

clean: 
	rm *.o

.PHONY: code

#---------- embedder ------------

code: embedder$(SUFFIX).o

OBJECTS = embedder$(SUFFIX).o
OBJPAT =  embedder%.o $(OGDFBASE)/ogdfbase%.o

$(OBJECTS): $(OGDFBASE)/ogdfbase.h embedder.h

embedder%.o: embedder.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) embedder.cpp
