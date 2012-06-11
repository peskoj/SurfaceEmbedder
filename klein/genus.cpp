#include "ogdfbase.h"

using namespace std;
using namespace ogdf;

int pOrientability = -1;
int pDel = 0;

int g, ng = 0;

int smaller_genus(Graph & G)
{
  if (pOrientability >= 0 && graph_genus(G) < g)
    return 1;

  if (pOrientability <= 0 && graph_genus_nonorientable(G) < ng)
    return 1;

  return 0;
}

int main()
{
  Graph G;

  while (read_graph(G)) {
#if VERBOSE
    print_graph(G);
#endif

    if (pOrientability >= 0)
      g = graph_genus(G);

#if VERBOSE
    printf("The graph has orientable genus %d.\n", g);
#endif

    if (pOrientability <= 0)
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
