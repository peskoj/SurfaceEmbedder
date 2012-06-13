#include "ogdfbase.h"
#include "embedder.h"

using namespace std;
using namespace ogdf;

int pOrient = 1;
int pDel = 0;
int pCheckOnly = 0;
int pGenus = 2;

int g, ng = 0;

int smaller_genus(Graph & G)
{
  if (pOrient >= 0 && graph_genus(G) < g)
    return 1;

  if (pOrient <= 0 && graph_genus_nonorientable(G) < ng)
    return 1;

  return 0;
}

int main(int argc, char ** argv)
{
  int c;
  while ((c = getopt (argc, argv, "cdCDo:g:")) != -1)
    switch (c) {
    case 'c':
      pCheckOnly = 1;
      break;
    case 'd':
      pDel = 1;
      break;
    case 'C':
      pCheckOnly = 0;
      break;
    case 'D':
      pDel = 0;
      break;
    case 'o':
      pOrient = atoi(optarg);
      if (pOrient < -1 || pOrient > 1) {
	printf("Error: Surface orientability is either -1, 0, or 1!\n");
	return 1;
      }
      break;
    case 'g':
      pGenus= atoi(optarg);
      if (pGenus < 0) {
	printf("Error: Surface genus has to be non-negative!\n");
	return 1;
      }
      break;
      
    case '?':
      printf("Unknown argument '-%c'\n", optopt);
      return 1;
    default:
      abort();
    }
  

  if (pCheckOnly) {
    while (true) {
      Embedder E;
      if (!read_emb(E, E.signature()))
	break;
      
#if VERBOSE
      print_graph(E);
#endif

      E.compute_faces();
      int g = E.genus();
      printf("Genus %d, orientable %d\n", g, E.orientable_emb());
    }
    return 0;
  }
  
  while (true) {
    Graph G;
    if (!read_graph(G))
      break;

#if VERBOSE
    print_graph(G);
#endif

    if (pOrient >= 0)
      g = graph_genus(G);

#if VERBOSE
    printf("The graph has orientable genus %d.\n", g);
#endif

    if (pOrient <= 0)
      ng = graph_genus_nonorientable(G);

#if VERBOSE
    printf("The graph has non-orientable genus %d.\n", ng);
#endif

#if !VERBOSE
    print_graph(G);
    printf("Graph of genus %d, non-orientable genus %d\n", g, ng);
#endif

    if (pDel) {
      int reducible = test_edge_deletion(G, &smaller_genus);
      
      if (!reducible)
	printf("Graph is deletion-minimal\n");
    }
  }

  return 0;
}
