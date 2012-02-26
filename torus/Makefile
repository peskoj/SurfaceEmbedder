OGDF_INC = /home/pskoda/apps/ogdf/OGDF/
OGDF_LIB = /home/pskoda/apps/ogdf/OGDF/_release/

all: torus test prune obs

torus: torus.c
	gcc -O3 -g -o torus torus.c

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

#------ runtorus -------


test: runtorus$(SUFFIX)


OBJECTS = ogdfbase$(SUFFIX).o ogdftorus$(SUFFIX).o
OBJPAT =  ogdfbase%.o ogdftorus%.o

$(OBJECTS): ogdfbase.h

ogdftorus%.o: ogdftorus.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ -I$(OGDF_INC) ogdftorus.cpp

ogdfbase%.o: ogdfbase.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ -I$(OGDF_INC) ogdfbase.cpp

runtorus%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LINK_OBJ) -L$(OGDF_LIB) -lOGDF

#------ runembobs -------


obs: runembobs$(SUFFIX)


OBJECTS = ogdfbase$(SUFFIX).o embobs$(SUFFIX).o
OBJPAT =  ogdfbase%.o embobs%.o

$(OBJECTS): ogdfbase.h

embobs%.o: embobs.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ -I$(OGDF_INC) embobs.cpp

ogdfbase%.o: ogdfbase.cpp
	@echo To get: $@ from $^ input $<
	g++ $(SWITCHES) -c -o $@ -I$(OGDF_INC) ogdfbase.cpp

runembobs%: $(OBJPAT)
	@echo Need: $(OBJECTS)
	g++ $(LINK_SWITCHES) -o $@ $^ $(LINK_OBJ) -L$(OGDF_LIB) -lOGDF

#------- prune geng -------

prune: geng-prune-torus$(SUFFIX)

PRUNE_OBJ = ogdfbase%.o prune-torus%.o geng.o  gtools.o nauty1.o nautil1.o naugraph1.o
PRUNE_OBJECTS = ogdfbase$(SUFFIX).o prune-torus$(SUFFIX).o geng.o

PRUNE_INC = -I$(OGDF_INC) -Inauty
NAUTY = nauty/

gtools.o : $(NAUTY)gtools.h $(NAUTY)gtools.c
	${CC} $(ARCH) -O3 -c ${CFLAGS} $(NAUTY)gtools.c

nautil1.o: $(NAUTY)nauty.h $(NAUTY)nautil.c
	${CC} $(ARCH) -O3 -c ${CFLAGS} ${SMALL} -o nautil1.o $(NAUTY)nautil.c
nauty1.o: $(NAUTY)nauty.h $(NAUTY)nauty.c
	${CC} $(ARCH) -O3 -c ${CFLAGS} ${SMALL} -o nauty1.o $(NAUTY)nauty.c
naugraph1.o: $(NAUTY)nauty.h $(NAUTY)naugraph.c
	${CC} $(ARCH) -O3 -c ${CFLAGS} ${SMALL} -o naugraph1.o $(NAUTY)naugraph.c


geng.o: nauty/geng.c
	g++ $(ARCH) -O3 -DPRUNE=prune -DSUMMARY=summary -DMAXN=32 -c -o geng.o $(PRUNE_INC) nauty/geng.c

prune-torus%.o: ogdfbase.h prune-torus.cpp
	g++ $(ARCH) $(SWITCHES) -c -o $@ $(PRUNE_INC) prune-torus.cpp

geng-prune-torus%: $(PRUNE_OBJ)
	@echo Need: $(PRUNE_OBJECTS)
	g++ $(ARCH) $(LINK_SWITCHES) -o $@ $^ $(LINK_OBJ) -L$(OGDF_LIB) -lOGDF

clean:
	-rm -f *.o runtorus* geng-prune-torus*
