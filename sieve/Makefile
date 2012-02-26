OGDFBASE = ogdfbase
EMBEDDER = embedder

OGDF_INC = -I$(HOME)/apps/ogdf/OGDF/ -I$(OGDFBASE) -I$(EMBEDDER)
OGDF_LIB = -L$(HOME)/apps/ogdf/OGDF/_release/ \
	   -L$(HOME)/apps/ogdf/OGDF/_debug/

VPATH = $(OGDFBASE) $(EMBEDDER)

all: recurse code

clean:
	+@$(MAKE) -w -C $(OGDFBASE) clean
	+@$(MAKE) -w -C $(EMBEDDER) clean
	rm -f *o runmintester*

SWITCHES = -Wall

ifeq  ($(findstring debug, $(MAKECMDGOALS)),)
  SWITCHES += -O3 #-DOPT_PATHS
  SUFFIX = -opt
endif

ifneq ($(findstring devel, $(MAKECMDGOALS)),)
  SWITCHES += -DMIN_TEST -DDEVEL
  SUFFIX = -dev
  GOALS += devel
endif

ifneq ($(findstring debug, $(MAKECMDGOALS)),)
  SWITCHES += -g -D DEBUG -D VERBOSE -DSTATISTICS
  SUFFIX = -deb
  GOALS += debug
endif

ifneq ($(findstring verbose, $(MAKECMDGOALS)),)
  SWITCHES += -D VERBOSE -DSTATISTICS
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


.PHONY: code devel clean verbose debug recurse

#---------------------- recurse -----------------------------------

recurse: 
	+@$(MAKE) -w -C $(OGDFBASE) ogdf $(GOALS)
	+@$(MAKE) -w -C $(EMBEDDER) base $(GOALS)

#---------------------- embedder-proj----------------------

code: runsieve$(SUFFIX)

OBJECTS = $(OGDFBASE)/ogdfbase$(SUFFIX).o $(EMBEDDER)/embedder$(SUFFIX).o sieve$(SUFFIX).o
OBJPAT =  $(OGDFBASE)/ogdfbase%.o $(EMBEDDER)/embedder%.o sieve%.o

sieve%.o: sieve.cpp ogdfbase.h embedder.h
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) sieve.cpp

runsieve%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(OGDF_LIB) -lOGDF -lpthread
