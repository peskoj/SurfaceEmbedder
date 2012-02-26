#include "ogdfbase.h"
#include "embedder.h"


using namespace std;
using namespace ogdf;

#if STATISTICS
extern int planarcount;
#endif

int cubic = 1;
int genus = 2;
int orientable = 1; //+1 orientable, -1 non-orientable, 0 Euler genus

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

int create_twist(Graph & G, adjEntry s, adjEntry t1, adjEntry t2, adjEntry t3)
{
  node s1n = split_arc(G, s);
  node s2n = split_arc(G, s);
  node s3n = split_arc(G, s);
  node t1n = split_arc(G, t1);
  node t2n = split_arc(G, t2);
  node t3n = split_arc(G, t3);

  G.newEdge(s1n, t1n);
  G.newEdge(s2n, t3n);
  G.newEdge(s3n, t2n);

  return 0;
}

int cubic_jumps(Embedder & E)
{
  //Test jumps
#if DEBUG
  printf("Generating jumps\n");
#endif
  int count = 0;

  edge e;
  forall_edges(e, E) {
    for (edge f = e->succ(); f; f = f->succ()) {
      if (e == f || E.same_face(e, f))
	continue;
#if DEBUG
      printf("Jump between edges %s and %s\n", print_edge_string(e).c_str(), print_edge_string(f).c_str());
#endif

      GraphCopy G(E);
      cubic_new_edge(G, G.copy(e), G.copy(f));
      
      print_graph(G);
      count++;
    }
  }
  return count;
}

int cubic_crosses(Embedder & E, Face & F)
{
#if DEBUG
  printf("Generating crosses in face %d\n", F.id());
#endif
  int count = 0;

  forall_nonconst_listiterators(edge, s1, F.edges()) {
    for (ListIterator<edge> t1 = s1.succ(); t1.valid(); t1++) {
      for (ListIterator<edge> s2 = t1.succ(); s2.valid(); s2++) {
	if (*s1 == *s2)
	  continue;
	
	if (!E.unique_emb(*s1, *s2))
	  continue;

	for (ListIterator<edge> t2 = s2.succ(); t2.valid(); t2++) {
	  if (*t1 == *t2)
	    continue;

	  if (!E.unique_emb(*t1, *t2))
	    continue;
#if DEBUG
	  printf("Cross at %s-%s and %s-%s\n", print_edge_string(*s1).c_str(), print_edge_string(*s2).c_str(), 
		 print_edge_string(*t1).c_str(), print_edge_string(*t2).c_str());
#endif
	  GraphCopy G(E);
	  create_cross(G,  copy_arc(G, face_arc(s1)), copy_arc(G, face_arc(s2)), copy_arc(G, face_arc(t1)), copy_arc(G, face_arc(t2)));
	  
	  print_graph(G);
	  count++;
	}
      }
    }
  }
  return count;
}

int cubic_singular_cords(Embedder & E, Face & F)
{
  int count = 0;

  edge sing = F.singEdges().front();
  forall_nonconst_listiterators(edge, s1, F.edges()) { 
    edge e = *s1;
    if (e == sing)
      continue;
    
    int crossed = 0;
    ListIterator<edge> singit = s1;
    for (ListIterator<edge> s2 = s1.succ(); s2.valid(); s2++) {
      edge f = *s2;
      if (f == sing) {
	crossed++;
	singit = s2;
	continue;
      }
      if (!crossed)
	continue;
      
      if (crossed > 1)
	break;

      if (E.unique_emb(e, f)) {
#if DEBUG
	printf("Cord between edges %s and %s\n", print_edge_string(e).c_str(), print_edge_string(f).c_str());
#endif
	GraphCopy G(E);
	cubic_new_edge(G, G.copy(e), G.copy(f));
	  
	print_graph(G);
	count++;
      }

#if DEBUG
      printf("Twisted cords between edges %s and %s\n", print_edge_string(e).c_str(), print_edge_string(f).c_str());
#endif

      {
	GraphCopy G(E);
	create_cross(G,  copy_arc(G, face_arc(singit)), copy_arc(G, face_arc(s2)), copy_arc(G, face_arc(singit)), copy_arc(G, face_arc(s1)));
	
	print_graph(G);
	count++;
      }
      
    }
  }
  return count;
}

int cubic_singular_crosses(Embedder & E, Face & F)
{
  int count = 0;

  edge sing = F.singEdges().front();
#if DEBUG
  printf("Generating crosses in singular face %d, sing edge%s\n", F.id(), print_edge_str(sing));
#endif

  forall_nonconst_listiterators(edge, t1, F.edges()) {
    if (*t1 == sing) 
      continue;

    for (ListIterator<edge> s2 = t1.succ(); s2.valid(); s2++) {
      if (*s2 == sing) 
	break;

      for (ListIterator<edge> t2 = s2.succ(); t2.valid(); t2++) {
	if (*t2 == sing) 
	  break;

	if (!E.unique_emb(*t1, *t2))
	  continue;

	if (*t1 == *t2) //Can this happen?
	  continue;

	
#if DEBUG
	printf("Simple cross at %s-%s and %s-%s\n", print_edge_string(sing).c_str(), print_edge_string(*s2).c_str(), 
	       print_edge_string(*t1).c_str(), print_edge_string(*t2).c_str());
#endif
	{
	  GraphCopy G(E);
	  create_cross(G,  copy_arc(G, sing->adjSource()), copy_arc(G, face_arc(s2)), copy_arc(G, face_arc(t1)), copy_arc(G, face_arc(t2)));
	
	  print_graph(G);
	  count++;
	}

#if DEBUG
	printf("Twist at %s to %s,%s,%s\n", print_edge_string(sing).c_str(), print_edge_string(*s2).c_str(), 
	       print_edge_string(*t1).c_str(), print_edge_string(*t2).c_str());
#endif
	{
	  GraphCopy G(E);
	  create_twist(G,  copy_arc(G, sing->adjSource()), copy_arc(G, face_arc(s2)), copy_arc(G, face_arc(t1)), copy_arc(G, face_arc(t2)));
	
	  print_graph(G);
	  count++;
	}
      }
    }
  }
  return count;
}

int cubic_factory(Embedder & E)
{
#if DEBUG
  printf("In cubic_factory\n");
#endif
  int count = 0;

  //  E.construct_faces();

  count += cubic_jumps(E);

  for (int i=0; i< E.numberOfFaces(); i++) {
    Face & F(E.faces(i));

    if (F.edgeSingular()) {
#if DEBUG
      printf("Face %d is edge singular, %d singular edges, %d singular nodes\n", F.id(), F.singEdges().size(), F.singNodes().size());
#endif
      
      if (F.singEdges().size() > 1) {
#if DEBUG
	printf("Warning: Skipping the face\n");
#endif
	continue;
      }
      assert(F.singEdges().size() == 1);

      //      count += cubic_singular_cords(E, F);
      //      count += cubic_singular_crosses(E, F);
      continue;
    }

    //    count += cubic_crosses(E, F);
  }
  
#if DEBUG
  printf("In total %d new graphs constructed\n\n", count);
#endif

  return count;
}

int factory(Embedder & E)
{
#if DEBUG
  printf("In factory\n");
#endif

  EdgeArray< List<int> > faces;


  return 0;
}


int main(int argc, char ** argv)
{

  int c;
  while ((c = getopt (argc, argv, "cCg:o:")) != -1)
    switch (c)
      {
      case 'c':
	cubic = 1;
	break;
      case 'C':
	cubic = 0; 
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


  fprintf(stderr, "Running sieve: genus %d, orientable %d, cubic %d\n", genus, orientable, cubic);
  assert(cubic);
  
  Graph G;

  while (read_graph(G)) {
#if VERBOSE
    print_graph(G);
#endif

    Embedder E(G);
    
    int emb = E.embed(genus, orientable);

#if VERBOSE
    printf("The graph is %s on the surface.\n\n", stremb[emb].c_str());
#endif

    if (!emb)
      continue;
      
    E.set_embedding();
    E.construct_faces();
    E.compute_singularities();
    
#if DEBUG
    int g = E.genus();
    printf("Computed genus is %d\n", g);
#endif    

    if (cubic)
      cubic_factory(E);
    else
      factory(E);
  }

#if STATISTICS
  fprintf(stderr, "Planarity tested %d-times\n", planarcount);
#endif

  return 0;
}
