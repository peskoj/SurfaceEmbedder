include Makefile.profiles

all: recurse code

clean: recurse
	rm -f *o runcascades*

.PHONY: code

#---------------------- recurse -----------------------------------

recurse: 
	+@$(MAKE) -w -C $(OGDFBASE) $(MAKECMDGOALS)
	+@$(MAKE) -w -C $(EMBEDDER) $(MAKECMDGOALS)

#---------------------- embedder-proj----------------------

code: runcascades$(SUFFIX)

OBJECTS = $(OGDFBASE)/ogdfbase$(SUFFIX).o $(EMBEDDER)/embedder$(SUFFIX).o cascades$(SUFFIX).o
OBJPAT =  $(OGDFBASE)/ogdfbase%.o $(EMBEDDER)/embedder%.o cascades%.o

cascades%.o: cascades.cpp ogdfbase.h embedder.h
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) cascades.cpp

runcascades%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LIB_PATHS) $(LIBS)
