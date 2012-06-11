include Makefile.profiles

all: recurse code

clean:
	+@$(MAKE) -w -C $(OGDFBASE) clean
	+@$(MAKE) -w -C $(EMBEDDER) clean
	rm -f *o runmintester*

.PHONY: code

#---------------------- recurse -----------------------------------

recurse: 
	+@$(MAKE) -w -C $(OGDFBASE) all $(GOALS)
	+@$(MAKE) -w -C $(EMBEDDER) all $(GOALS)

#---------------------- embedder-proj----------------------

code: runmintester$(SUFFIX)

OBJECTS = $(OGDFBASE)/ogdfbase$(SUFFIX).o $(EMBEDDER)/embedder$(SUFFIX).o mintester$(SUFFIX).o
OBJPAT =  $(OGDFBASE)/ogdfbase%.o $(EMBEDDER)/embedder%.o mintester%.o

mintester%.o: mintester.cpp ogdfbase.h embedder.h
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) mintester.cpp

runmintester%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(OGDF_LIB) -lOGDF -lpthread
