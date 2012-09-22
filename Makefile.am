AUTOMAKE_OPTIONS = foreign 
#SUBDIRS = mintester scripts

SHELL = /bin/sh
OGDF = OGDF

SWITCHES = -Wall
if DEBUG
  SWITCHES += -g -DDEBUG -DOGDF_DEBUG
  OGDF_LIB = $(OGDF)/_debug
else
  SWITCHES += -O3
  OGDF_LIB = $(OGDF)/_release
endif

if VERBOSE
  SWITCHES += -DVERBOSE -DSTATISTICS
endif


MY_LIBS_INC = $(OGDF_LIB)  -L$(HOME)/lib/json
MY_INCLUDE = -Iogdfbase -Iembedder -I$(OGDF) -I$(HOME)/include
AM_CXXFLAGS = $(SWITCHES) $(MY_INCLUDE) -L$(MY_LIBS_INC)
MY_LIBS = -lOGDF -ljson -lpthread 
OGDFBASE = ogdfbase/ogdfbase.cpp
EMBEDDER = embedder/embedder.cpp

bin_PROGRAMS = runmintester runcascades
runmintester_SOURCES = mintester/mintester.cpp $(OGDFBASE) $(EMBEDDER)
runmintester_LDFLAGS = $(MY_LIBS)
#runmintester_LDADD = ogdfbase.o embedder.o

runcascades_SOURCES = cascades/cascades.cpp $(OGDFBASE) $(EMBEDDER)
runcascades_LDFLAGS = $(MY_LIBS)
