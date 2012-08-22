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

//------------------------- Minimality tests -----------------------------

int test_edge_deletion(Graph & G, int (*test)(Graph & G))
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

#if VERBOSE
    node u = e->source();
    node v = e->target();
#endif

#if DEBUG
    printf("Deleting edge between %d and %d\n", u->index(), v->index());
#endif

    G.hideEdge(e);
    int res = (*test)(G);

#if DEBUG
    printf("After deletion of the edge between %d and %d, result = %d.\n", u->index(), v->index(), res);
#endif

    G.restoreEdge(e);

    ec++;

    if (!res) {
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


#if MIN_TEST
int test_cycles(Graph & G, NodeArray<int> & nodes, NodeArray<int> & comps, int pieces)
{
  node v;
  forall_nodes(v, G) {
    if (nodes[v] < pieces)
      continue;

    int c = 0;
    edge e;
    forall_adj_edges(e, v) {
      node u = e->opposite(v);
      if (comps[u] != comps[v] || nodes[u] != nodes[v]) 
	break;
      c++;
    }
    if (c == 3) {
#if DEBUG
      printf("Vertex of degree 3 found: %d\n", v->index()); 
#endif
      return 0;
    }
  }
  return 1;
} 

int test_feet(Graph & G, NodeArray<int> & nodes, NodeArray<int> & comps, int cc, int pieces)
{
  int feet[cc][pieces];

  for (int i=0;i<cc; i++)
    for (int j=0;j<pieces; j++)
      feet[i][j] = 0;

  node v;
  forall_nodes(v, G) {
    if (nodes[v] < pieces)
      continue;

    edge e;
    forall_adj_edges(e, v) {
      node u = e->opposite(v);

      if (nodes[u] < pieces)
	feet[nodes[v]][nodes[u]]++;
    }
  }

#if DEBUG
  for(int i=MAXKS; i<cc; i++) {
    printf("Bridge %d is attached to:", i);
    for(int j=0; j<pieces; j++) 
      if (feet[i][j])
	printf(" %d(%dx)", j, feet[i][j]);
    printf("\n");
  }
#endif

  return 1;
}

int minimality_test_kur(Graph & G, KuratowskiWrapper & K)
{
  EdgeArray<int> bedges(G, 0);
  NodeArray<int> bnodes(G, 0);

  node v;

  forall_slistiterators(edge, it, K.edgeList) {
    edge e = *it;
    bedges[e] = 1;
    bnodes[e->source()] = 1;
    bnodes[e->target()] = 1;
  }

#if DEBUG
  print_graph_color(G, bnodes, bedges);
#endif

  int c = 2;
  forall_nodes(v, G) {
    if (!bnodes[v])
      continue;

    int deg = 0;
    edge e;
    forall_adj_edges(e, v)
      if (bedges[e])
	deg++;

    if (deg == 3)
      bnodes[v] = c++;
  }


  forall_nodes(v, G) {
    if (bnodes[v] != 1)
      continue;
    
    color_comp(bnodes, bedges, 1, v, c);
    c++;
  }

  int pieces = c;
  c = MAXKS;
#if DEBUG
  printf("Coloring bridges starting with %d\n", c);
#endif

  edge e;
  forall_edges(e, G) {
    if (bedges[e]) 
      continue;
    
    color_comp(bnodes, bedges, 0, e->source(), c);
    color_comp(bnodes, bedges, 0, e->target(), c);
    bedges[e] = c;
    c++;
  }

  NodeArray<int> comps(G, 0);
  forall_nodes(v, G) {
    if (bnodes[v] < pieces || comps[v])
      continue;

    two_con_comp(G, bnodes, bnodes[v], comps, v);
  }

#if DEBUG
  forall_nodes(v, G) {
    printf("Node %d in bridge %d comp %d\n", v->index(), bnodes[v], comps[v]);
  }  
#endif

  int cycles = test_cycles(G, bnodes, comps, pieces);
  if (!cycles)
    return 0;


#if DEBUG
  printf("Trivial bridges:");
  forall_edges(e, G) {
    //    printf("Edge %d-%d with color %d\n", e->source()->index(), e->target()->index(), bedges[e]);
    if (bedges[e] < pieces)
      continue;

    if (bnodes[e->source()] == bedges[e] || bnodes[e->target()] == bedges[e])
      continue;

    printf(" %d-%d", bnodes[e->source()], bnodes[e->target()]);
  }
  printf("\n");
#endif

  int feet = test_feet(G, bnodes, comps, c, pieces);
  if (!feet)
    return 0;
  
  return 1;
}



int test_minimality(Graph & G)
{
#if DEBUG
  printf("In minimality_test:\n");
#endif

  SList<KuratowskiWrapper> output;

  int c = MAXK;
  int planar = test_planarity_bounded(G, c, output);

#if DEBUG
    printf("%d Kuratowski subdivision found\n", output.size());
    printf("The graph is %s.\n", str[planar].c_str());
#endif

  if (planar)
    return 2;
  
  int m = G.numberOfEdges() + 1; //G.numberOfEdges() + 1;
  KuratowskiWrapper * K = 0;

  for (SListIterator<KuratowskiWrapper> it = output.begin(); it != output.end(); it++) {
    KuratowskiWrapper & L = *it;

    int size = L.edgeList.size();
    
#if DEBUG
    printf("%s, subtype %s, of size %d at node %d\n", strkur[L.isK33()].c_str(), strsubtype[L.subdivisionType].c_str(), size, L.V->index());

    printf("Edges:");
    print_edge_list(L.edgeList);
#endif
    if (size < m) {
      K = &L;
      m = size;
    }
  }

  assert(K);

  return minimality_test_kur(G, *K);
}
#endif

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

    Embedder E(G);
    int g = E.min_genus(3, 1);

    if (g <= 2) {
      if (DEBUG)
	printf("Edge between %d and %d is a jump\n", u->index(), v->index());
      return 1;
    }

    if (DEBUG)
      printf("After deletion of the edge between %d and %d, the graph has genus %d.\n", u->index(), v->index(), g);

    G.newEdge(u, v);

    ec++;

    if (g > 2) {
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
    
    Embedder E(G);
    int g = E.min_genus(3, 1);
    ec++;

    if (g > 2) {
      minimal = false;
      printf("Contractible edge between %d and %d\n", f->source()->index(), f->target()->index());
      break;
    }
  }
  printf("# of edges tested: %d\n", ec);

  printf("The graph is contraction-%s.\n", strmin[minimal].c_str());

  return !minimal;
}

int create_cross(Graph & G, adjEntry s1, adjEntry t1, adjEntry s2, adjEntry t2)
{
#if DEBUG
  printf("Creating cross at ");
  print_edge(s1->theEdge());
  print_edge(t1->theEdge());
  print_edge(s2->theEdge());
  print_edge(t2->theEdge());
  printf("\n");
#endif

  GraphCopySimple H(G);
  
  node u1 = subdivideEdge(H, H.copy(s1->theEdge()));
  node u2 = subdivideEdge(H, H.copy(t1->theEdge()));
  node v1 = subdivideEdge(H, H.copy(s2->theEdge()));
  node v2 = subdivideEdge(H, H.copy(t2->theEdge()));

  H.newEdge(u1, v1);
  H.newEdge(u2, v2);

  print_graph(H);
  
  return 0;
}

int create_jump(Graph & G, edge e, edge f)
{
#if DEBUG
  printf("Creating jump at ");
  print_edge(e);
  print_edge(f);
  printf("\n");
#endif

  GraphCopySimple H(G);
  
  node u = subdivideEdge(H, H.copy(e));
  node v = subdivideEdge(H, H.copy(f));

  H.newEdge(u, v);

  print_graph(H);

  return 0;
}


int create_planar_obs(Graph & G)
{
#if DEBUG
  printf("In create_planar_obs\n");
#endif

  CombinatorialEmbedding E(G);
  NodeArray<long long> faces(G, 0);

  int i = 0;
  face f;
  forall_faces(f, E) {
    adjEntry a;
    forall_face_adj(a, f) {
      faces[a->theNode()] |= 1 << i;
    }
    i++;
  }

#if DEBUG
  node u;
  printf("Face incidence:\n");
  forall_nodes(u, G) {
    printf("%2d: ", u->index());
    PRINT_BIN(faces[u], E.numberOfFaces());
    printf("\n");
  }
#endif 
  

  forall_faces(f, E) {
    adjEntry a = f->firstAdj();
    adjEntry s1 = a;
    while (s1->faceCycleSucc() != a) {
      adjEntry t1 = s1->faceCycleSucc();
      while (t1 != a) {
	adjEntry s2 = t1->faceCycleSucc();
	while (s2 != a) {
	  adjEntry t2 = s2->faceCycleSucc();
	  while (t2 != a) {
	    create_cross(G, s1, t1, s2, t2);

	    t2 = t2->faceCycleSucc();
	  }
	  s2 = s2->faceCycleSucc();
	}
	t1 = t1->faceCycleSucc();
      }
      s1 = s1->faceCycleSucc();
    }
  }

  edge e, g;
  forall_edges(e, G) {
    forall_edges(g, G) {
      if (E.rightFace(e->adjSource()) == E.rightFace(g->adjSource()) ||
	  E.rightFace(e->adjSource()) == E.leftFace(g->adjSource()) ||
	  E.leftFace(e->adjSource()) == E.leftFace(g->adjSource()) ||
	  E.leftFace(e->adjSource()) == E.rightFace(g->adjSource()))
	continue;

      create_jump(G, e, g);
    }
  }

  return 0;
}


int main()
{
  Graph G;

  while (read_graph(G)) {
#if VERBOSE
    print_graph(G);
#endif

    int planar = test_planarity_with_embedding(G);

#if VERBOSE
    printf("The graph is %s.\n", str[planar].c_str());
#endif

    if (planar) {
      create_planar_obs(G);
      continue;
    }

    continue;

#if MIN_TEST
    int min = test_minimality(G);
    
#if VERBOSE
    printf("The graph is %s.\n", strmin[min].c_str());
#endif

    if (!min)
      continue;
#endif
    
    Embedder E(G);
    int g = E.min_genus(3, 1);

#if VERBOSE
    printf("The graph has genus %d.\n", g);
#endif

    if (g <= 2)
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
