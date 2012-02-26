OGDFBASE = ogdfbase
EMBEDDER = embedder

OGDF_INC = -I$(HOME)/apps/ogdf/OGDF/ -I$(OGDFBASE) -I$(EMBEDDER)

VPATH = ogdfbase

#all: test

all: base

clean: 
	rm *.o

.PHONY: base genus test clean debug devel verbose profile opt_paths server min_test


SWITCHES = -Wall

ifeq  ($(findstring debug, $(MAKECMDGOALS)),)
  SWITCHES += -O3 #-DOPT_PATHS
  SUFFIX = -opt
  OGDF_LIB = -L$(HOME)/apps/ogdf/OGDF/_release/
endif

ifneq ($(findstring devel, $(MAKECMDGOALS)),)
  SWITCHES += -DMIN_TEST -DDEVEL
  SUFFIX = -dev
endif

ifneq ($(findstring debug, $(MAKECMDGOALS)),)
  SWITCHES += -g -D DEBUG -D VERBOSE
  SUFFIX = -deb
  OGDF_LIB = -L$(HOME)/apps/ogdf/OGDF/_debug/
endif

ifneq ($(findstring verbose, $(MAKECMDGOALS)),)
  SWITCHES += -D VERBOSE
  SUFFIX = -ver
endif

ifneq ($(findstring profile, $(MAKECMDGOALS)),)
  SWITCHES += -pg
  LINK_SWITCHES += -pg
endif

ifneq ($(findstring opt_paths, $(MAKECMDGOALS)),)
  SWITCHES += -DOPT_PATHS
endif

ifneq ($(findstring server, $(MAKECMDGOALS)),)
  LINK_OBJ += `find server -name "*.o"`
  ARCH = -arch ppc
endif

ifneq ($(findstring min_test, $(MAKECMDGOALS)),)
  SWITCHES += -DMIN_TEST
  SUFFIX = -min
endif



#---------- embedder ------------

#test: runembed$(SUFFIX)
base: embedder$(SUFFIX).o

OBJECTS = embedder$(SUFFIX).o
OBJPAT =  embedder%.o $(OGDFBASE)/ogdfbase%.o

$(OBJECTS): $(OGDFBASE)/ogdfbase.h embedder.h

embedder%.o: embedder.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) embedder.cpp

#runembed%: $(OBJPAT)
#	@echo Need: $(OBJECTS)
#	g++ $(LINK_SWITCHES) -o $@ $^ $(LINK_OBJ) -L$(OGDF_LIB) -lOGDF

$(OGDFBASE)/ogdfbase%.o:
	$(MAKE) -w -C ogdfbase