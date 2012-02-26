OGDF_INC = /home/pskoda/apps/ogdf/OGDF/
OGDF_LIB = /home/pskoda/apps/ogdf/OGDF/_release/

all: test

clean: 
	rm *.o

.PHONY: genus test clean debug devel verbose profile opt_paths server min_test


SWITCHES = -Wall

ifeq  ($(findstring debug, $(MAKECMDGOALS)),)
  SWITCHES += -O3 #-DOPT_PATHS
  SUFFIX = -opt
endif

ifneq ($(findstring devel, $(MAKECMDGOALS)),)
  SWITCHES += -DMIN_TEST -DDEVEL
  SUFFIX = -dev
endif

ifneq ($(findstring debug, $(MAKECMDGOALS)),)
  SWITCHES += -g -D DEBUG -D VERBOSE
  SUFFIX = -deb
endif

ifneq ($(findstring verbose, $(MAKECMDGOALS)),)
  SWITCHES += -D VERBOSE
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


#------ runtorus -------


test: runklein$(SUFFIX)


OBJECTS = ogdfbase$(SUFFIX).o test-klein$(SUFFIX).o
OBJPAT =  ogdfbase%.o test-klein%.o

$(OBJECTS): ogdfbase.h

test-klein%.o: test-klein.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ -I$(OGDF_INC) test-klein.cpp

ogdfbase%.o: ogdfbase.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ -I$(OGDF_INC) ogdfbase.cpp

runklein%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LINK_OBJ) -L$(OGDF_LIB) -lOGDF

#---------- genus ------------

genus: rungenus$(SUFFIX)


OBJECTS = ogdfbase$(SUFFIX).o genus$(SUFFIX).o
OBJPAT =  ogdfbase%.o genus%.o

$(OBJECTS): ogdfbase.h

genus%.o: genus.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ -I$(OGDF_INC) genus.cpp

rungenus%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LINK_OBJ) -L$(OGDF_LIB) -lOGDF

