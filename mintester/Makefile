include Makefile.profiles

all: recurse code

clean: recurse
	rm -f *o runmintester*

.PHONY: code

#---------------------- recurse -----------------------------------

recurse: 
	+@$(MAKE) -w -C $(OGDFBASE) $(MAKECMDGOALS)
	+@$(MAKE) -w -C $(EMBEDDER) $(MAKECMDGOALS)

#---------------------- embedder-proj----------------------

code: runmintester$(SUFFIX)

OBJECTS = $(OGDFBASE)/ogdfbase$(SUFFIX).o $(EMBEDDER)/embedder$(SUFFIX).o mintester$(SUFFIX).o
OBJPAT =  $(OGDFBASE)/ogdfbase%.o $(EMBEDDER)/embedder%.o mintester%.o

mintester%.o: mintester.cpp ogdfbase.h embedder.h
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) mintester.cpp

runmintester%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LIB_PATHS) $(LIBS)
