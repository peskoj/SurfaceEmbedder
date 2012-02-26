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

int contract = 0;
int add = 0;
int addmany = 0;
int maxdepth = 2;


int reconstruct(Graph & G);

int add_new_edge(Graph & G, int uindex, int vindex, int depth)
{
  if (depth > maxdepth)
    return 0;

  node u, v;
  forall_nodes(u, G) {
    forall_nodes(v, G) {
      if (u->index() < uindex || (u->index() == uindex && v->index() <= vindex))
	continue;

      if (u->index() >= v->index() || G.searchEdge(u, v))
	continue;
	
      edge g = G.newEdge(u, v);

#if DEBUG
      printf("Adding new edge %d-%d:\n", u->index(), v->index());
      print_graph_fast(G);
#endif
      int proj = test_projective_cutter(G);

      if (!proj) {
#if VERBOSE
	printf("Non-projective graph found:\n");
	print_graph_fast(G);
#endif

	int reduc = test_edge_deletion(G, &test_projective_cutter);

	if (!reduc) {
#if VERBOSE
	  printf("Deletion minimal obstruction\n");
#endif

	  reconstruct(G);
	}
      } else 
	add_new_edge(G, u->index(), v->index(), depth+1);
      
      G.delEdge(g);
    }
  }
  return 0;
}

int add_many_and_contract(Graph & G)
{
#if VERBOSE
  printf("In add_many_and_contract\n");
#endif

  edge e;
  forall_edges(e, G) {
    GraphCopySimple H(G);
    
    edge f = H.copy(e);
#if DEBUG
    printf("Contracting edge %d-%d\n", e->source()->index(), e->target()->index());
#endif

    H.contract(f);
    if (!isSimpleUndirected(H))
      continue;

    add_new_edge(H, -1, -1, 1);
  }

  return 1;
}


int add_and_contract(Graph & G)
{
#if VERBOSE
  printf("In add_and_contract\n");
#endif

  edge e;
  forall_edges(e, G) {
    GraphCopySimple H(G);
    
    edge f = H.copy(e);
#if DEBUG
    printf("Contracting edge %d-%d\n", e->source()->index(), e->target()->index());
#endif

    H.contract(f);
    if (!isSimpleUndirected(H))
      continue;
    
    node u, v;
    forall_nodes(u, H) {
      forall_nodes(v, H) {
	if (u->index() >= v->index() || H.searchEdge(u, v))
	  continue;
	
	edge g = H.newEdge(u, v);

#if DEBUG
	printf("Adding new edge %d-%d:\n", u->index(), v->index());
	print_graph_fast(H);
#endif
	int proj = test_projective_cutter(H);

	if (!proj) {
#if VERBOSE
	  printf("Non-projective graph found:\n");
	  print_graph_fast(H);
#endif

	  int reduc = test_edge_deletion(H, &test_projective_cutter);

	  if (!reduc) {
#if VERBOSE
	    printf("Deletion minimal obstruction\n");
#endif

	    reconstruct(H);
	  }
	}

	H.delEdge(g);
      }
    }
  }

  return 1;
}

int contract_edges(Graph & G, int index, int recursive);


int check_min(Graph & G)
{
  if (!contract_edges(G, -1, 0)) 
    return 0;

#if VERBOSE
  printf("Contraction minimal graph found:\n");
  print_graph(G);
#endif    
  
#if !VERBOSE
  print_graph(G);
#endif
    
  return 1;
}

int contract_edges(Graph & G, int index, int recursive)
{
#if DEBUG
  printf("In contract_edges with index %d\n", index);
  print_graph_fast(G);
#endif

  int minimal = 1;

  edge e;
  forall_edges(e, G) {
    if (e->index() < index)
      continue;

    GraphCopy H(G);
    
    edge f = H.copy(e);

    H.contract(f);

#if DEBUG
    printf("Contracting edge %d-%d\n", f->source()->index(), f->target()->index());
    print_graph_fast(H);
    assert(H.consistencyCheck());
#endif

    if (!isSimpleUndirected(H))
      continue;

    int proj = test_projective_cutter(H);

    if (!proj) {
#if VERBOSE
      printf("Contractible edge between %d and %d\n", e->source()->index(), e->target()->index());
#endif    

      if (recursive)
	contract_edges(H, e->index(), recursive);
      minimal = 0;
    }
  }

  if (minimal && recursive) {
    check_min(G);
  }

  return minimal;
}

int reconstruct(Graph & G)
{
#if VERBOSE
  printf("Reconstruction ...\n");
#endif

  contract_edges(G, -1, 1);

  return 0;
}

int main(int argc, char **argv)
{

  int c;
  while ((c = getopt (argc, argv, "acm:")) != -1)
    switch (c)
      {
      case 'c':
	contract = 1;
	break;
      case 'a':
	add = 1;
	break;
      case 'm':
	addmany = 1;
	maxdepth = atoi(optarg);
	if (maxdepth < 1) {
	  printf("Max depth expected > 0\n");
	  return 1;
	}
	break;
      case '?':
	printf("Unknown argument '-%c'\n", optopt);
	return 1;
      default:
	abort();
      }


  Graph G;

  while (read_graph(G)) {
#if VERBOSE
    print_graph(G);
#endif

    if (contract)
      reconstruct(G);
    else if (add)
      add_and_contract(G);
    else
      add_many_and_contract(G);
  }

#if DEBUG
  fprintf(stderr, "Planarity tested %d-times\n", planarcount);
#endif

  return 0;
}
