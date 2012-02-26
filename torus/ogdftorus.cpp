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

    if (DEBUG)
      printf("Deleting edge between %d and %d\n", u->index(), v->index());

    G.delEdge(e);

#if CUTTER
    int torus = test_torus_cutter(G);
#else
    int torus = test_torus_multiple(G); // old version
#endif

    if (torus == 2) {
      if (DEBUG)
	printf("Edge between %d and %d is a jump\n", u->index(), v->index());
      return 1;
    }

    if (DEBUG)
      printf("After deletion of the edge between %d and %d, the graph is %s on the torus.\n", u->index(), v->index(), stremb[torus].c_str());

    G.newEdge(u, v);

    ec++;

    if (!torus) {
      minimal = false;
      if (VERBOSE)
	printf("Superfluous edge between %d and %d\n", u->index(), v->index());
      break;
    }
  }

#if VERBOSE
  printf("# of edges tested: %d\n", ec);
  printf("The graph is deletion-%s.\n", strmin[minimal].c_str());
#endif

  return !minimal;
}

int test_contraction(Graph & H)
{
  bool minimal = true;

  edge f;
  int ec = 0;
  forall_edges(f, H) {
    GraphCopySimple G(H);
    
    edge e = G.copy(f);
    G.contract(e);
    makeSimpleUndirected(G);
    
    bool torus = test_torus(G);
    ec++;

    if (!torus) {
      minimal = false;
      printf("Contractible edge between %d and %d\n", f->source()->index(), f->target()->index());
      break;
    }
  }
  printf("# of edges tested: %d\n", ec);

  printf("The graph is contraction-%s.\n", strmin[minimal].c_str());

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

#if MIN_TEST
    int min = test_minimality(G);
    
#if VERBOSE
    printf("The graph is %s.\n", strmin[min].c_str());
#endif

    if (!min)
      continue;
#endif
    
#if CUTTER
    int torus = test_torus_cutter(G);
#else
    int torus = test_torus_multiple(G);
#endif

    assert(torus < 2);
    
#if VERBOSE
      printf("The graph is %s on the torus.\n", stremb[torus].c_str());
#endif

    if (torus)
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
