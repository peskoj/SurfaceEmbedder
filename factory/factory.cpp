#include "ogdfbase.h"
#include "embedder.h"


using namespace std;
using namespace ogdf;

#if STATISTICS
extern int planarcount;
#endif

int contraction = 0;

int jumps = 0;
int triads = 0;
int tripods = 0;
int crosses = 0;

int filter = 1;
int genus = 0;
int orientable = 1;

typedef Tuple2<int,int> Pair;
List< Pair > addedges;

void filter_graph(Graph & G)
{
  Array<node> nodes(G.maxNodeIndex()+1);
  node u;
  forall_nodes(u, G)
    nodes[u->index()] = u;

  forall_listiterators(Pair, it, addedges) {
    Pair p = *it;
    assert(p.x1() <= G.maxNodeIndex() && p.x2() <= G.maxNodeIndex());

    node u = nodes[p.x1()];
    node v = nodes[p.x2()];
    G.newEdge(u,v);
  }

  if (!filter) {
    print_graph(G);
    return;
  }

  print_graph(G);
}

void contraction_factory(Graph & H)
{
  edge f;
  forall_edges(f, H) {
    GraphCopySimple G(H);
    
    edge e = G.copy(f);
    G.contract(e);
    makeSimpleUndirected(G);

#if VERBOSE
    print_graph(H);
#endif
    filter_graph(G);
  }
}

void edge_factory(Graph & H, node u, node v)
{
  GraphCopySimple G(H);
  
  node uc = G.copy(u);
  node vc = G.copy(v);
  
  G.newEdge(uc, vc);
  filter_graph(G);
}

void edge_factory(Graph & H, node u, edge e)
{
  GraphCopySimple G(H);
  
  node uc = G.copy(u);
  edge ec = G.copy(e);
  node v = subdivide_edge(G, ec);  
  G.newEdge(uc, v);

  filter_graph(G);
}

void edge_factory(Graph & H, edge e, edge f)
{
  GraphCopySimple G(H);
  
  edge ec = G.copy(e);
  edge fc = G.copy(f);

  node u = subdivide_edge(G, ec);  
  node v = subdivide_edge(G, fc);  
  G.newEdge(u, v);

  filter_graph(G);
}

void simple_jump_factory(Graph & H)
{
  node u;
  forall_nodes(u, H) {
    node v;
    forall_nodes(v, H) {
      if (u->index() >= v->index() || H.searchEdge(u, v))
	continue;
      
      edge_factory(H, u, v);
    }
  }
}

void jump_factory(Embedder & E)
{
#if DEBUG
  printf("jump_factory\n");
#endif

  NodeArray< NodeArray<int> > seen;
  NodeArray< EdgeArray<int> > seen_edge;

  seen.init(E);
  seen_edge.init(E);

  node u;
  forall_nodes(u, E) {
    seen[u].init(E, 0);
    seen_edge[u].init(E, 0);
  }

  forall_nodes(u, E) {
    node v;
    forall_nodes(v, E) {
      if (!seen[u][v] && !E.same_face(u, v)) {
	seen[u][v] = 1;
	seen[v][u] = 1;
	edge_factory(E, u, v);	
      }
    }
  }

  edge e;
  forall_nodes(u, E) {
    forall_edges(e, E) {
      node v1, v2;
      v1 = e->source();
      v2 = e->target();
      
      if (!seen[u][v1] && !seen[u][v2] && !E.same_face(e, u)) {
	seen_edge[u][e] = 1;
	edge_factory(E, u, e);	
      }
    }
  }

  forall_edges(e, E) {
    edge f;
    forall_edges(f, E) {
      if (e->index() >= f->index() || E.same_face(e, f))
	continue;

      node u1, u2, v1, v2;
      u1 = e->source();
      u2 = e->target();

      v1 = f->source();
      v2 = f->target();

      if (!seen[u1][v1] && !seen[u1][v2] && !seen[u2][v1] && !seen[u2][v2] && !seen_edge[u1][f] && !seen_edge[u2][f] && !seen_edge[v1][e] && !seen_edge[v2][e]) {
	edge_factory(E, e, f);
      }
    }
  }
}

void triad_factory(Embedder & E)
{
#if DEBUG
  printf("triad_factory\n");
#endif

  node u;
  forall_nodes(u, E) {
    if (u->degree() != 3)
      continue;
    
    GraphCopySimple G(E);
  
    node v = G.newNode();

    adjEntry a;
    forall_adj(a, u) {
      node w = a->twinNode();

      G.newEdge(G.copy(w), v);
    }

    filter_graph(G);
  }
}

void attach_tripod(Graph & H, node u1, node u2, node u3)
{
  GraphCopySimple G(H);

  node w1 = G.copy(u1);
  node w2 = G.copy(u2);
  node w3 = G.copy(u3);
  
  edge e;
  if ((e = G.searchEdge(w1, w2)))
    G.delEdge(e);

  if ((e = G.searchEdge(w1, w3)))
    G.delEdge(e);

  if ((e = G.searchEdge(w2, w3)))
    G.delEdge(e);
  
  node v1 = G.newNode();
  node v2 = G.newNode();

  G.newEdge(w1, v1);
  G.newEdge(w2, v1);
  G.newEdge(w3, v1);

  G.newEdge(w1, v2);
  G.newEdge(w2, v2);
  G.newEdge(w3, v2);
  
  filter_graph(G);
}

void tripod_factory(Embedder & E)
{
#if DEBUG
  printf("tripod_factory\n");
#endif

  Face * F;
  forall_emb_faces(F, E) {
    forall_listiterators(adjEntry, it1, F->adj()) {
      adjEntry a1 = *it1;
      node u1 = a1->theNode();

      forall_listiterators(adjEntry, it2, F->adj()) {
	adjEntry a2 = *it2;
	node u2 = a2->theNode();
	if (u1->index() >= u2->index())
	  continue;

	forall_listiterators(adjEntry, it3, F->adj()) {
	  adjEntry a3 = *it3;
	  node u3 = a3->theNode();
	  
	  if (u2->index() >= u3->index())
	    continue;

	  attach_tripod(E, u1, u2, u3);
	}
      }
    }
  }
}

void construct_cross(Graph & H, node x1, node x2, node y1, node y2)
{
#if DEBUG
  printf("construct_cross %d-%d, %d-%d\n", x1->index(), y1->index(), x2->index(), y2->index());
#endif

  GraphCopySimple G(H);

  G.newEdge(G.copy(x1), G.copy(y1));
  G.newEdge(G.copy(x2), G.copy(y2));
  
  filter_graph(G);
}

void cross_factory(Embedder & E)
{
#if DEBUG
  printf("cross_factory\n");
#endif

  Face * F;
  forall_emb_faces(F, E) {
    ListIterator<adjEntry> it1;
    for (it1 = F->adj().begin(); it1.valid(); ++it1) {
      adjEntry a1 = *it1;
      node u1 = a1->theNode();
      
      ListIterator<adjEntry> it2;
      for (it2 = it1.succ(); it2.valid(); ++it2) {
	adjEntry a2 = *it2;
	node u2 = a2->theNode();

	ListIterator<adjEntry> it3;
	for (it3 = it2.succ(); it3.valid(); ++it3) {
	  adjEntry a3 = *it3;
	  node u3 = a3->theNode();

	  ListIterator<adjEntry> it4;
	  for (it4 = it3.succ(); it4.valid(); ++it4) {
	    adjEntry a4 = *it4;
	    node u4 = a4->theNode();
	    
	    construct_cross(E, u1, u2, u3, u4);
	  }
	}
      }
    }
  }
}

int main(int argc, char ** argv)
{

  int c;
  int u = 0, v = 0;
  while ((c = getopt (argc, argv, "cCjJtTpPrRfFg:o:e:")) != -1)
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
      case 't':
	triads = 1;
	break;
      case 'T':
	triads = 0; 
	break;
      case 'p':
	tripods = 1;
	break;
      case 'P':
	tripods = 0; 
	break;
      case 'r':
	crosses = 1;
	break;
      case 'R':
	crosses = 0; 
	break;
      case 'f':
	filter = 1;
	break;
      case 'F':
	filter = 0; 
	break;
      case 'e':
	sscanf(optarg, "%d %d", &u, &v);
	addedges.pushBack(Pair(u,v));
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


  fprintf(stderr, "Running factory: genus %d, orientable %d, contraction %d, jumps %d, filter %d\n", genus, orientable, contraction, jumps, filter);
  
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

    Embedder E(G);
    int emb = E.embed(genus, orientable);

    if (!emb)
      continue;

    E.construct_faces();

#if VERBOSE
    print_emb(E, E.signature(), 0);
#endif

    if (jumps) 
      jump_factory(E);

    if (triads) 
      triad_factory(E);
    
    if (tripods)
      tripod_factory(E);
    
    if (crosses)
      cross_factory(E);

    filter_graph(G);
  }

#if STATISTICS
  fprintf(stderr, "Planarity tested %d-times\n", planarcount);
#endif

  return 0;
}
