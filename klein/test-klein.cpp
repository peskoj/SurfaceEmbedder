#include "ogdfbase.h"

using namespace std;
using namespace ogdf;

extern BoyerMyrvold BM;

#if DEBUG
extern int planarcount;
#endif


extern string str[2];
extern string stremb[2];
extern string strkur[2];
extern string strsubtype[16];
extern string strmin[2];
extern string strres[2];


int test_deletion(Graph & G)
{
  bool minimal = true;
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
    int klein = test_klein_cutter(G);

#if DEBUG
    printf("After deletion of the edge between %d and %d, the graph is %s in the klein bottle (type %d).\n", u->index(), v->index(), stremb[klein > 0].c_str(), klein);
#endif

    G.newEdge(u, v);

    ec++;

    if (!klein) {
      minimal = false;

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

//     int planar = test_planarity(G);

// #if VERBOSE
//     printf("The graph is %s.\n", str[planar].c_str());
// #endif

//     if (planar)
//       continue;

    int klein = test_klein_cutter(G);

#if VERBOSE
    printf("The graph is %s.\n", str[klein >= 5].c_str());
#endif

    if (klein >= 5)
      continue;

#if VERBOSE
    printf("The graph is %s in the projective plane.\n", stremb[klein >= 4].c_str());
#endif

    if (klein >= 4)
      continue;

#if VERBOSE
    printf("The graph is %s in the klein bottle (type %d).\n", stremb[klein>0].c_str(), klein);
#endif

    if (klein)
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
