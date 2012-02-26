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
