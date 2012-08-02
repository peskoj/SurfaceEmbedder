include Makefile.profiles

all: recurse code

clean: recurse
	rm -f *o rungenstats*

.PHONY: code

#---------------------- recurse -----------------------------------

recurse: 
	+@$(MAKE) -w -C $(OGDFBASE) $(MAKECMDGOALS)
	+@$(MAKE) -w -C $(EMBEDDER) $(MAKECMDGOALS)

#---------------------- embedder-proj----------------------

code: rungenstats$(SUFFIX)

OBJECTS = $(OGDFBASE)/ogdfbase$(SUFFIX).o $(EMBEDDER)/embedder$(SUFFIX).o genstats$(SUFFIX).o
OBJPAT =  $(OGDFBASE)/ogdfbase%.o $(EMBEDDER)/embedder%.o genstats%.o

genstats%.o: genstats.cpp ogdfbase.h embedder.h
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) genstats.cpp

rungenstats%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LIB_PATHS) $(LIBS)
