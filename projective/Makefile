OGDFBASE = ogdfbase
EMBEDDER = embedder

OGDF_INC = -I$(HOME)/apps/ogdf/OGDF/ -I$(OGDFBASE) -I$(EMBEDDER)
OGDF_LIB = -L$(HOME)/apps/ogdf/OGDF/_release/ \
	   -L$(HOME)/apps/ogdf/OGDF/_debug/

VPATH = $(OGDFBASE) $(EMBEDDER)

all: test


SWITCHES = -Wall -DCUTTER

ifeq  ($(findstring debug, $(MAKECMDGOALS)),)
  SWITCHES += -O3 #-DOPT_PATHS
  SUFFIX = -opt
endif

ifneq ($(findstring devel, $(MAKECMDGOALS)),)
  SWITCHES += -DMIN_TEST
  SUFFIX = -dev
  GOALS += devel
endif

ifneq ($(findstring debug, $(MAKECMDGOALS)),)
  SWITCHES += -g -D DEBUG -D VERBOSE
  SUFFIX = -deb
  GOALS += debug
endif

ifneq ($(findstring verbose, $(MAKECMDGOALS)),)
  SWITCHES += -D VERBOSE
  SUFFIX = -ver
  GOALS += verbose
endif

ifneq ($(findstring profile, $(MAKECMDGOALS)),)
  SWITCHES += -pg
  LINK_SWITCHES += -pg
  GOALS += profile
endif

ifneq ($(findstring opt_paths, $(MAKECMDGOALS)),)
  SWITCHES += -DOPT_PATHS
endif


.PHONY: recon devel clean test verbose debug special test-embed recurse

#---------------------- embedder-proj----------------------

test-embed: recurse

recurse: 
	+@$(MAKE) -w -C $(OGDFBASE) ogdf $(GOALS)
	+@$(MAKE) -w -C $(EMBEDDER) base $(GOALS)


test-embed: runembedder-proj$(SUFFIX)

OBJECTS = $(OGDFBASE)/ogdfbase$(SUFFIX).o $(EMBEDDER)/embedder$(SUFFIX).o embedder-proj$(SUFFIX).o
OBJPAT =  $(OGDFBASE)/ogdfbase%.o $(EMBEDDER)/embedder%.o embedder-proj%.o

embedder-proj%.o: embedder-proj.cpp ogdfbase.h embedder.h
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) embedder-proj.cpp

runembedder-proj%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(OGDF_LIB) -lOGDF -lpthread

#---------------------- runproj -------------------------------

test: runproj$(SUFFIX)


OBJECTS = ogdfbase$(SUFFIX).o test-proj$(SUFFIX).o
OBJPAT =  ogdfbase%.o test-proj%.o

$(OBJECTS): ogdfbase.h

test-proj%.o: test-proj.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) test-proj.cpp

runproj%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LINK_OBJ) $(OGDF_LIB) -lOGDF

#---------------------- reconstruct -------------------------------

recon: runrecon$(SUFFIX)


OBJECTS = ogdfbase$(SUFFIX).o reconstruct$(SUFFIX).o
OBJPAT =  ogdfbase%.o reconstruct%.o

$(OBJECTS): ogdfbase.h

reconstruct%.o: reconstruct.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) reconstruct.cpp

runrecon%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LINK_OBJ) $(OGDF_LIB) -lOGDF

#---------- ogdfprojective ----------------------------------

ogdfprojective.o: ogdfprojective.cpp ogdfbase.h
	g++ $(SWITCHES) -c $(OGDF_INC) ogdfprojective.cpp 

ogdfprojective: ogdfprojective.o ogdfbase.o
	g++ -o ogdfprojective ogdfprojective.o ogdfbase.o $(OGDF_LIB) -lOGDF

clean:
	rm *.o

special:
	g++ -o ogdfprojective ogdfprojective.o ogdfbase.o `find ~/apps/ogdf/OGDF/_saferelease/src/ -name "*o"`

