OGDF_INC = -I$(HOME)/apps/ogdf/OGDF/ 
OGDF_LIB = -L$(HOME)/apps/ogdf/OGDF/_release/ \
	   -L$(HOME)/apps/ogdf/OGDF/_debug/


all: ogdf

clean:
	rm -f *o

SWITCHES = -Wall

ifeq  ($(findstring debug, $(MAKECMDGOALS)),)
  SWITCHES += -O3 #-DOPT_PATHS
  SUFFIX = -opt
endif

ifneq ($(findstring devel, $(MAKECMDGOALS)),)
  SWITCHES += -DCUTTER #-DMIN_TEST 
  SUFFIX = -dev
endif

ifneq ($(findstring debug, $(MAKECMDGOALS)),)
  SWITCHES += -g -D DEBUG -D VERBOSE -DSTATISTICS
  SUFFIX = -deb
endif

ifneq ($(findstring verbose, $(MAKECMDGOALS)),)
  SWITCHES += -D VERBOSE -DSTATISTICS
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

.PHONY: debug devel verbose profile clean

#------ ogdfbase -------


ogdf: ogdfbase$(SUFFIX).o


OBJECTS = ogdfbase$(SUFFIX).o 

$(OBJECTS): ogdfbase.h

ogdfbase%.o: ogdfbase.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ $(OGDF_INC) ogdfbase.cpp

