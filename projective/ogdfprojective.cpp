#include "ogdfbase.h"

using namespace std;
using namespace ogdf;

extern BoyerMyrvold BM;

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
    int projective = test_projective_cutter(G);
#else    
    int projective = test_projective_multiple(G);
#endif
    
#if DEBUG
    if (projective > 1)
      printf("After deletion of the edge between %d and %d, the graph is planar.\n", u->index(), v->index());
    else
      printf("After deletion of the edge between %d and %d, the graph is %s on the projective.\n", u->index(), v->index(), stremb[projective].c_str());
#endif

    G.newEdge(u, v);

    ec++;

    if (!projective) {
      minimal = false;
      if (VERBOSE)
	printf("Superfluous edge between %d and %d\n", u->index(), v->index());
      break;
    }
  }
  if (VERBOSE)
    printf("# of edges tested: %d\n", ec);

  if (VERBOSE)
    printf("The graph is deletion-%s.\n", strmin[minimal].c_str());

  return !minimal;
}

int test_contraction(Graph & H)
{
  bool minimal = true;

  edge f;
  int ec = 0;
  forall_edges(f, H) {
    if (DEBUG)
      printf("Contracting edge between %d and %d\n", f->source()->index(), f->target()->index());
    GraphCopySimple G(H);
    
    if (DEBUG) {
      printf("Graph copy:\n");
      print_graph(G);
    }
    edge e = G.copy(f);
    G.contract(e);
    makeSimpleUndirected(G);
    
#if CUTTER
    int projective = test_projective_cutter(G);
#else    
    int projective = test_projective_multiple(G);
#endif

    ec++;

    if (!projective) {
      minimal = false;
      if (VERBOSE)
	printf("Contractible edge between %d and %d\n", f->source()->index(), f->target()->index());
      break;
    }
  }
  if (VERBOSE)
    printf("# of edges tested: %d\n", ec);

  if (VERBOSE)
    printf("The graph is contraction-%s.\n", strmin[minimal].c_str());

  return !minimal;
}



int main()
{
  Graph G;

  while (read_graph(G)) {
    if (VERBOSE)
      print_graph(G);

    int planar = test_planarity(G);

    if (VERBOSE)
      printf("The graph is %s.\n", str[planar].c_str());

    if (planar)
      continue;

#if CUTTER
    int projective = test_projective_cutter(G);
#else    
    int projective = test_projective_multiple(G);
#endif
    
    if (VERBOSE)
    printf("The graph is %s on the projective plane.\n", stremb[projective].c_str());
    
    if (projective)
      continue;

    if (test_deletion(G))
      continue;
    
    if (test_contraction(G))
      continue;

    if (VERBOSE)
      printf("Minimal obstruction found\n");

    if (!VERBOSE)
      print_graph(G);
  }
  

  return 0;
}
