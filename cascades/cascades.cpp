#include "ogdfbase.h"
#include "embedder.h"


using namespace std;
using namespace ogdf;

#if STATISTICS
extern int planarcount;
#endif

int pGenerate = 0;
int pTest = 0;
int pPlanar = 0;
int pTestall = 0;

AdjEntryArray<int> adjface;
NodeArray< List<int> > nodeface;
edge red;
int facecount;

int planarnonproj;

adjEntry get_arc(node u, edge e)
{
  if (e->source() == u)
    return e->adjSource();
  else
    return e->adjTarget();
}

adjEntry face_arc(ListIterator<edge> & it)
{
  edge e = *it;
  edge f;
  node u;
  if (it.pred().valid()) {
    f = *(it.pred());
    u = e->commonNode(f);
  } else {
    f = *(it.succ());
    u = e->opposite(e->commonNode(f));
  }

  assert(u);
  
  return get_arc(u, e);
}

adjEntry copy_arc(GraphCopy & G, adjEntry a)
{
  node u = G.copy(a->theNode());
  edge e = G.copy(a->theEdge());
  assert(e);

  return get_arc(u, e);
}


node split_arc(Graph & G, adjEntry a)
{
  edge e =  a->theEdge();
  node v = a->theNode();
#if DEBUG
  //  printf("Splitting arc%s\n", print_arc_str(a));
#endif

  node u = G.newNode();
  if (a == e->adjSource())
    G.moveSource(e, u);
  else
    G.moveTarget(e, u);

  G.newEdge(u, v);

  return u;
}

int create_cross(Graph & G, adjEntry s1, adjEntry s2, adjEntry t1, adjEntry t2)
{
  node s1n = split_arc(G, s1);
  node s2n = split_arc(G, s2);
  node t1n = split_arc(G, t1);
  node t2n = split_arc(G, t2);

  G.newEdge(s1n, s2n);
  G.newEdge(t1n, t2n);

  return 0;
}


int same_face(Graph G, edge e, edge f)
{
  return 1;
}

int jumps(Embedder & E)
{
  //Test jumps
#if DEBUG
  printf("Generating jumps\n");
#endif
  int count = 0;

  node u, v;
  forall_nodes(u, E) {
    forall_nodes(v, E) {
      if (u->index() >= v->index() | intersect(nodeface[u], nodeface[v]))
	continue;

      if (red->isIncident(u) && red->isIncident(v))
	continue;

      E.restoreEdge(red);
      GraphCopy G(E);
      E.hideEdge(red);

      G.newEdge(G.copy(u),G.copy(v));
      print_graph(G);
      count++;
    }
  }
    

  edge e;
  forall_edges(e, E) {
    for (edge f = e->succ(); f; f = f->succ()) {
      if (e->index() >= f->index() || same_face(E, e, f))
	continue;
#if DEBUG
      printf("Jump between edges %s and %s\n", print_edge_string(e).c_str(), print_edge_string(f).c_str());
#endif

      E.restoreEdge(red);
      GraphCopy G(E);
      E.hideEdge(red);

      node u = G.split(G.copy(e))->source();
      node v = G.split(G.copy(f))->source();
      G.newEdge(u,v);
      
      print_graph(G);
      count++;
    }
  }
  return count;
}

int crosses(Embedder & E)
{
#if DEBUG
  printf("Generating crosses\n");
#endif
  int count = 0;

//   node u, v;
//   forall_nodes(u, E) {
//     forall_nodes(v, E) {
//       if (u == v | intersect(nodeface[u], nodeface[v]))
// 	continue;

//       edge e = E.newEdge(u,v);
//       E.restoreEdge(red);
//       print_graph(E);
//       E.hideEdge(red);
//       E.delEdge(e);
//       count++;
//     }
//   }

  return count;
}

int cascade_factory(Embedder & E)
{
#if DEBUG
  printf("In factory\n");
#endif

  int count;

  count += jumps(E);

  count += crosses(E);
  return count;
}


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

int main(int argc, char ** argv)
{

  int c;
  while ((c = getopt (argc, argv, "gtp")) != -1)
    switch (c)
      {
      case 'g':
	pGenerate = 1;
	break;
      case 't':
	pTest = 1;
	break;
      case 'p':
	pPlanar = 1;
	break;
      case '?':
	printf("Unknown argument '-%c'\n", optopt);
	return 1;
      default:
	abort();
      }


  fprintf(stderr, "Running cascades: generate %d, test %d\n", pGenerate, pTest);
  
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

    edge r;
    forall_edges(r, G) {
      red = mapEdge[r];

#if VERBOSE
      printf("\nRed edge is %s\n", print_edge_str(red));
#endif

      E.hideEdge(red);
      
      int planar = test_planarity_with_embedding(E);
      
#if VERBOSE
      printf("H-red is %s.\n", str[planar].c_str());
#endif

      if (pPlanar && planar) {
	E.restoreEdge(red);
	print_graph(E);
	break;
      }


      if (pGenerate) {
	if (!planar) {
	  E.restoreEdge(red);
	  continue;
	}
      
	facecount = traverse_faces(E, adjface, nodeface);

	cascade_factory(E);
      }
      
      E.restoreEdge(red);

      if (pTest) {
	if (planar)
	  continue;
	

	planarnonproj = 0;
	//      printf("%d %s\n", E.firstEdge()->index(), print_edge_str(E.firstEdge()));
	if (!test_deletion(E))
	  continue;
	if (!test_contraction(E))
	  continue;
	
#if VERBOSE
	printf("There is e such that G*e is non-projective and G*e-red is planar: %d.\n", planarnonproj);
#endif
	
	if (!planarnonproj)
	  continue;
	
	print_graph(E);
	if (!pTestall)
	  break;
      }
    }
  }

#if STATISTICS
  fprintf(stderr, "Planarity tested %d-times\n", planarcount);
#endif

  return 0;
}
