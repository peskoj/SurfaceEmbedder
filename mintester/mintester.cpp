#include "ogdfbase.h"
#include "embedder.h"


using namespace std;
using namespace ogdf;

#if STATISTICS
extern int planarcount;
#endif

int tcont = 0; //test contraction
int tdel = 1; //test deletion
int genus = 2; //euler genus (2x orientable genus)
int orientable = 1; //+1 orientable, -1 non-orientable, 0 Euler genus
int testall = 0; //test all edges for deletion/contraction

int test_embeddable(Graph & G)
{
  Embedder E(G);
  int res = E.embed(genus, orientable);
#if VERBOSE
  if (res) {
    E.set_embedding();
    print_emb(E, genus);
    int g = E.compute_genus();
    printf("The graph is embeddable into the surface of genus %d orientable %d (required %d,%d)\n", g, E.orientable_emb(), genus, orientable);
    if (g >= genus)
      assert(g == genus);
      if (orientable)
	assert(E.orientable_emb() == orientable);
    }
  }
#endif
  return res;
}
    

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
    int emb = test_embeddable(G);

#if VERBOSE && !DEBUG
    if (testall && emb)
      printf("After deletion of the edge between %d and %d the graph is %s in the surface.\n\n", u->index(), v->index(), stremb[emb].c_str());
#endif

#if DEBUG
    printf("After deletion of the edge between %d and %d the graph is %s in the surface.\n\n", u->index(), v->index(), stremb[emb].c_str());
#endif

    G.newEdge(u, v);

    ec++;

    if (!emb) {
      minimal = false;

#if VERBOSE
      printf("Superfluous edge between %d and %d\n\n", u->index(), v->index());
#endif
      
      if (!testall)
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
    
    bool emb = test_embeddable(G);
    ec++;

    if (!emb) {
      minimal = false;
      
#if VERBOSE
      printf("Contractible edge between %d and %d\n", f->source()->index(), f->target()->index());
#endif

      if (!testall)
	break;
    }
  }

#if VERBOSE
  printf("# of edges tested: %d\n", ec);
  printf("The graph is contraction-%s.\n", strmin[minimal].c_str());
#endif

  return !minimal;
}




int main(int argc, char ** argv)
{

  int c;
  while ((c = getopt (argc, argv, "acdCDg:o:")) != -1)
    switch (c)
      {
      case 'a':
	testall = 1;
	break;
      case 'c':
	tcont = 1;
	break;
      case 'd':
	tdel = 1;
	break;
      case 'C':
	tcont = 0;
	break;
      case 'D':
	tdel = 0;
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


  fprintf(stderr, "Testing for minimality: genus %d, orientable: %d, testing deletion: %d, testing contraction: %d\n", genus, orientable, tdel, tcont);

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

    int emb = test_embeddable(G);

#if VERBOSE
    printf("The graph is %s on the surface.\n\n", stremb[emb].c_str());
#endif

    if (emb) 
      continue;


  
    if (tdel && test_deletion(G))
      continue;

    if (tcont && test_contraction(G))
      continue;

#if VERBOSE
    printf("Minimal obstruction found\n\n");
#endif

#if !VERBOSE
    print_graph(G);
#endif
    
  }

#if STATISTICS
  fprintf(stderr, "Planarity tested %d-times\n", planarcount);
#endif

  return 0;
}
