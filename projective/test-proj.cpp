#include "ogdfbase.h"

using namespace std;
using namespace ogdf;

#if DEBUG
extern int planarcount;
#endif



int test_deletion(Graph & G)
{
  int minimal = 1;
  edge e;
  List<edge> edges;
  forall_edges(e, G) {
    edges.pushBack(e);
  }

  int ec = 0;
  forall_listiterators(edge, it, edges) {
    e = *it;
    assert(e);
    node u = e->source();
    node v = e->target();

#if DEBUG
    printf("Deleting edge between %d and %d\n", u->index(), v->index());
#endif

    G.delEdge(e);
    int proj = test_projective_cutter(G);

#if DEBUG
      printf("After deletion of the edge between %d and %d, the graph is %s on the proj.\n", u->index(), v->index(), stremb[proj].c_str());
#endif

    G.newEdge(u, v);

    ec++;

    if (!proj) {
      minimal = 0;

#if VERBOSE
      printf("Superfluous edge between %d and %d\n", u->index(), v->index());
#endif

      break;
    }
  }

#if VERBOSE
  printf("# of edges tested: %d\n", ec);
  printf("The graph is deletion-%s.\n", strmin[minimal].c_str());
#endif

  return !minimal;
}


int main()
{
  Graph G;

  while (read_graph(G)) {
#if VERBOSE
    print_graph(G);
#endif

    int planar = test_planarity(G);
    
#if VERBOSE
    printf("The graph is %s.\n", str[planar].c_str());
#endif
    
    if (planar)
      continue;
    
    int proj = test_projective_cutter(G);

#if VERBOSE
    printf("The graph is %s in the projective plane.\n", stremb[proj].c_str());
#endif

    if (proj)
      continue;
  
    if (test_deletion(G))
      continue;

//     if (test_contraction(G))
//       continue;

#if VERBOSE
    printf("Minimal obstruction found\n");
#endif

#if !VERBOSE
    print_graph(G);
#endif
    
  }

#if DEBUG
  fprintf(stderr, "Planarity tested %d-times\n", planarcount);
#endif

  return 0;
}
