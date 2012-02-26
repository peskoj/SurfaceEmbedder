#include "ogdfbase.h"
#include "embedder.h"


using namespace std;
using namespace ogdf;

#if STATISTICS
extern int planarcount;
#endif

int contraction = 0;
int jumps = 0;
int filter = 1;
int genus = 1;
int orientable = 1;

void contraction_factory(Graph & H)
{
  edge f;
  forall_edges(f, H) {
    GraphCopySimple G(H);
    
    edge e = G.copy(f);
    G.contract(e);
    makeSimpleUndirected(G);

#if VERBOSE
    print_graph(H)
#endif
    filter(G);
  }
}

void filter(Graph & G)
{
  if (!filter) {
    print_graph(G);
    return;
  }
}

int main(int argc, char ** argv)
{

  int c;
  while ((c = getopt (argc, argv, "cCjJfFg:o:")) != -1)
    switch (c)
      {
      case 'c':
	contraction = 1;
	break;
      case 'C':
	contraction = 0; 
	break;
      case 'j':
	jumps = 1;
	break;
      case 'J':
	jumps = 0; 
	break;
      case 'f':
	filter = 1;
	break;
      case 'F':
	filter = 0; 
	break;
      case 'g':
	genus = atoi(optarg);
	if (genus < 0) {
	  printf("Error: Surface genus has to be non-negative!\n");
	  return 1;
	}
	break;
      case 'o':
	orientable = atoi(optarg);
	if (orientable < -1 || orientable > 1) {
	  printf("Error: Surface orientability is either -1, 0, or 1!\n");
	  return 1;
	}
	break;
      case '?':
	printf("Unknown argument '-%c'\n", optopt);
	return 1;
      default:
	abort();
      }


  fprintf(stderr, "Running factory: genus %d, orientable %d, contraction %d\n", genus, orientable, contraction);
  
  Graph G;

  while (read_graph(G)) {
#if DEBUG
  printf("Graph read\n");
#endif
#if VERBOSE
    print_graph(G);
#endif

    if (contraction) {
      contraction_factory(G);
    }

    filter(G);
  }

#if STATISTICS
  fprintf(stderr, "Planarity tested %d-times\n", planarcount);
#endif

  return 0;
}
