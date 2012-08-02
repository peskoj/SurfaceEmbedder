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

code: runfactory$(SUFFIX)

OBJECTS = $(OGDFBASE)/ogdfbase$(SUFFIX).o $(EMBEDDER)/embedder$(SUFFIX).o factory$(SUFFIX).o
OBJPAT =  $(OGDFBASE)/ogdfbase%.o $(EMBEDDER)/embedder%.o factory%.o

factory%.o: factory.cpp ogdfbase.h embedder.h
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) factory.cpp

runfactory%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LIB_PATHS) $(LIBS)
