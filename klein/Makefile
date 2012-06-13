include Makefile.profiles

all: recurse code

clean:
	rm -f *o rungenus*

.PHONY: code

#---------------------- recurse -----------------------------------

recurse: 
	+@$(MAKE) -w -C $(OGDFBASE) $(MAKECMDGOALS)
	+@$(MAKE) -w -C $(EMBEDDER) $(MAKECMDGOALS)

#---------- genus ------------

code: rungenus$(SUFFIX)

OBJECTS = $(OGDFBASE)/ogdfbase$(SUFFIX).o $(EMBEDDER)/embedder$(SUFFIX).o genus$(SUFFIX).o
OBJPAT =  $(OGDFBASE)/ogdfbase%.o $(EMBEDDER)/embedder%.o genus%.o

genus%.o: genus.cpp ogdfbase.h embedder.h
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) genus.cpp

rungenus%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LIB_PATHS) $(LIBS)

