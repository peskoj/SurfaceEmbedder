#include "ogdfbase.h"
#include "embedder.h"


using namespace std;
using namespace ogdf;

#if STATISTICS
extern int planarcount;
#endif

int pTest = 0;
int pPlanar = 0;
int pTestall = 0;
int pTryred = 1;
int pFilter = 0;

AdjEntryArray<int> adjface;
NodeArray< List<int> > nodeface;
edge red;
int facecount;

int planarnonproj;


int test_deletion(Graph & G)
{
#if DEBUG
  printf("In test_deletion\n");
#endif
  
  edge e;
  forall_edges(e, G) {
    if (e->index() == red->index())
      continue;
    NodeArray< node > mapNode;
    EdgeArray< edge > mapEdge;

    Embedder H(G, mapNode, mapEdge);
    edge redcopy = mapEdge[red];
    assert(redcopy);

    edge ecopy = mapEdge[e];
    assert(ecopy);
    
#if VERBOSE
    printf("Testing embeddability of H-e, e = %s\n", print_edge_str(e));
#endif

    H.delEdge(ecopy);
    int proj = H.embed(1,-1);


#if DEBUG
    printf("Testing planarity of H-e-red\n");
#endif

    H.hideEdge(redcopy);

#if DEBUG
    print_graph(H);
#endif
    int planar = test_planarity(H);

#if VERBOSE
    printf("H-e is %s projective planar\n", strneg[proj].c_str());
    printf("H-e-red is %s\n", str[planar].c_str());
#endif

    if (planar && !proj)
      planarnonproj = 1;

    if (proj || planar)
      continue;

#if VERBOSE
    printf("Edge %s is superfluous\n", print_edge_str(e));
#endif

    return 0;
  }
  return 1;
}

int test_contraction(Graph & G)
{
#if DEBUG
  printf("In test_contraction\n");
#endif
  
  edge e;
  forall_edges(e, G) {
    if (e->index() == red->index())
      continue;
    NodeArray< node > mapNode;
    EdgeArray< edge > mapEdge;

    Embedder H(G, mapNode, mapEdge);
    edge redcopy = mapEdge[red];
    assert(redcopy);

    edge ecopy = mapEdge[e];
    assert(ecopy);
    

#if DEBUG
    printf("Testing planarity of H/e-red\n");
#endif

    H.contract(ecopy);
    H.hideEdge(redcopy);
#if DEBUG
    print_graph(H);
#endif
    int planar = test_planarity(H);
    H.restoreEdge(redcopy);

#if VERBOSE
    printf("Testing embeddability of H/e, e = %s\n", print_edge_str(e));
#endif

    makeSimpleUndirected(H);
    remove_isolated(H);
#if DEBUG
    print_graph(H);
#endif
    int proj = H.embed(1,-1);

#if VERBOSE
    printf("H/e is %s projective planar\n", strneg[proj].c_str());
    printf("H/e-red is %s\n", str[planar].c_str());
#endif

    if (planar && !proj)
      planarnonproj = 1;

    if (proj || planar)
      continue;

#if VERBOSE
    printf("Edge %s is contractible\n", print_edge_str(e));
#endif

    return 0;
  }
  return 1;
}


int test_red(Embedder & E) {
#if VERBOSE
  printf("\nRed edge is %s\n", print_edge_str(red));
#endif

  E.hideEdge(red);
  
  int planar = test_planarity_with_embedding(E);
  
#if VERBOSE
  printf("H-red is %s.\n", str[planar].c_str());
#endif
    
  E.restoreEdge(red);
  
  if (planar)
    return 0;	

  planarnonproj = 0;
  if (!test_deletion(E))
    return 0;
  if (!test_contraction(E))
    return 0;
    
#if VERBOSE
  printf("There is e such that G*e is non-projective and G*e-red is planar: %d.\n", planarnonproj);
#endif
    
  if (!planarnonproj)
    return 0;
  
  return 1;
}

int test_red_planar(Embedder & E)
{
  E.hideEdge(red);
  
  int planar = test_planarity_with_embedding(E);
    
#if VERBOSE
  printf("H-red is %s.\n", str[planar].c_str());
#endif
  E.restoreEdge(red);
  
  if (planar) {
    print_graph(E);
    return 1;
  }
  return 0;
}

int filter(Embedder & E)
{
  E.hideEdge(red);
  
  int planar = test_planarity_with_embedding(E);
  
#if VERBOSE
  printf("G is %s.\n", str[planar].c_str());
#endif
    
  E.restoreEdge(red);
  
  if (planar)
    return 0;	

  int proj = E.embed(1,-1);
#if VERBOSE
  printf("The graph G^+ is %s into the projective plane.\n", stremb[proj].c_str());
#endif
  if (proj)
      return 0;

  return 1;
}

int main(int argc, char ** argv)
{

  int c;
  while ((c = getopt (argc, argv, "tprf")) != -1)
    switch (c)
      {
      case 't':
	pTest = 1;
	break;
      case 'p':
	pPlanar = 1;
	break;
      case 'r':
	pTryred = 0;
	break;
      case 'f':
	pFilter = 1;
	break;
      case '?':
	printf("Unknown argument '-%c'\n", optopt);
	return 1;
      default:
	abort();
      }


  fprintf(stderr, "Running cascades: test %d, planar %d, find terminals %d, filter only %d\n", pTest, pPlanar, pTryred, pFilter);
  
  Graph G;

  while (read_graph(G)) {
#if VERBOSE
    print_graph(G);
#endif

    EdgeArray<edge> mapEdge;
    NodeArray<node> mapNode;
    Embedder E(G, mapNode, mapEdge);

    if (0 & pTest) {
      int emb = E.embed(2, 0);
      
#if VERBOSE
      printf("The graph is %s on a surface of euler genus 2.\n", stremb[emb].c_str());
#endif

      if (!emb)
	continue;
    }

    if (pPlanar) {
      edge r;
      forall_edges(r, G) {
	red = mapEdge[r];
	
	if (test_red_planar(E))
	  break;
      }
      continue;
    }
    
    if (pFilter) {
      node x = E.firstNode();
      node y = x->succ();
      assert(x->index() == 0 && y->index() == 1);
      
      edge r = E.searchEdge(x, y);
      if (r) 
	red = r;
      else
	red = E.newEdge(x, y);
      if (filter(E))
	print_graph(G);
    }    

    if (pTest) {
      if (!pTryred) {
	node x = E.firstNode();
	node y = x->succ();
	assert(x->index() == 0 && y->index() == 1);
	
	edge r = E.searchEdge(x, y);
	if (r) 
	  red = r;
	else
	  red = E.newEdge(x, y);
	if (test_red(E))
	  print_graph(G);
      } else {
	edge r;
	forall_edges(r, G) {
	  red = mapEdge[r];
	  
	  if (test_red(E)) {
	    E.delEdge(red);
	    print_graph(E);
	    if (!pTestall)
	      break;
	  }
	}
      }
    }
  }

#if STATISTICS
  fprintf(stderr, "Planarity tested %d-times\n", planarcount);
#endif

  return 0;
}
