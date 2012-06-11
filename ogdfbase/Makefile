include Makefile.profiles

all: ogdf

clean:
	rm -f *o

#------ ogdfbase -------


ogdf: ogdfbase$(SUFFIX).o


OBJECTS = ogdfbase$(SUFFIX).o 

$(OBJECTS): ogdfbase.h

ogdfbase%.o: ogdfbase.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) ogdfbase.cpp

