#include "ogdfbase.h"
#include "nauty/nauty.h"

#define PRUNE prune
#define PRUNELEVEL 6

using namespace std;
using namespace ogdf;

extern BoyerMyrvold BM;

#if DEBUG
extern int planarcount;
#endif


int graphcount = 0;
int torcount = 0;
int redcount = 0;
int plancount = 0;
#if VERBOSE
int prunecount = 0;
#endif

extern string str[2];
extern string stremb[2];
extern string strkur[2];
extern string strsubtype[16];
extern string strmin[2];
extern string strres[2];

//---------------------- From geng.c ------------------------------------------

static boolean
isbiconnected(graph *g, int n)
/* test if g is biconnected */
{
        int sp,v,w;
        setword sw;
        setword visited;
        int numvis,num[MAXN],lp[MAXN],stack[MAXN];
 
        if (n <= 2) return FALSE;
 
        visited = bit[0];
        stack[0] = 0;
        num[0] = 0;
        lp[0] = 0;
        numvis = 1;
        sp = 0;
        v = 0;
 
        for (;;)
        {
            if ((sw = g[v] & ~visited))           /* not "==" */
            {
                w = v;
                v = FIRSTBIT(sw);       /* visit next child */
                stack[++sp] = v;
                visited |= bit[v];
                lp[v] = num[v] = numvis++;
                sw = g[v] & visited & ~bit[w];
                while (sw)
                {
                    w = FIRSTBIT(sw);
                    sw &= ~bit[w];
                    if (num[w] < lp[v])  lp[v] = num[w];
                }
            }
            else
            {
                w = v;                  /* back up to parent */
                if (sp <= 1)          return numvis == n;
                v = stack[--sp];
                if (lp[w] >= num[v])  return FALSE;
                if (lp[w] < lp[v])    lp[v] = lp[w];
            }
        }
}


//----------------------------------------------------------------------------

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
    int torus = test_torus_multiple(G);

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


int test_graph(Graph & G, int last)
{
#if DEBUG
  printf("Testing graph embeddings\n");
#endif

    int planar = test_planarity(G);

#if VERBOSE
    printf("The graph is %s.\n", str[planar].c_str());
#endif

    if (planar) {
      if (last)
	plancount++;
      return 0;
    }

#if MIN_TEST
    int min = test_minimality(G);

#if VERBOSE
    printf("The graph is %s.\n", strmin[min].c_str());
#endif

    if (!min) {
      if (last)
	redcount++;
      return 1;
    }
#endif

    int torus = test_torus_multiple(G);
    assert(torus < 2);
    
#if VERBOSE
    printf("The graph is %s on the torus.\n", stremb[torus].c_str());
#endif

    if (torus) {
      if (last)
	torcount++;
      return 0;
    }

  
//     if (test_deletion(G))
//       return 0;

//     if (test_contraction(G))
//       continue;

    if (last) {
#if VERBOSE
      printf("Induced subgraph of a possible obstruction:\n");
#endif

#if !VERBOSE
      print_graph(G);
#endif
      graphcount++;
    }

    return 1;
}

int convert(graph * g, int n, Graph & G)
{
  vector<node> nodes(n, NULL);

  for (int i=0; i<n; i++)
    if (g[i])
      nodes[i] = G.newNode(i);

  for (int i=0; i<n; i++)
    for (int j=i+1; j<n; j++)
      if (g[i] & bit[j])
	G.newEdge(nodes[i], nodes[j]);

  return 0;
}

int prune(graph * g, int n, int maxn)
{
  if (n < maxn-PRUNELEVEL)
    return 0;

  if (!isbiconnected(g, n))
    return 0;

  Graph G;
  convert(g, n, G);

  
#if VERBOSE
  printf("Pruning graph (order %d):\n", n);
  print_graph(G);
  prunecount++;
#endif

  if ((G.numberOfEdges()*2)/3 < maxn-PRUNELEVEL) 
    return 0;

//   if (!isConnected(G))
//     return 0;

  int result = test_graph(G, n==maxn);
#if VERBOSE
  printf("Graph %s\n\n", strres[result].c_str());
#endif

  return result;
}


typedef struct
{
    long hi,lo;
} bigint;

void summary(bigint nout, double cpu)
{
  fprintf(stderr, ">Number of non-toroidal graphs: %d\n", graphcount);
  fprintf(stderr, ">Number of toroidal graphs: %d\n", torcount);
  fprintf(stderr, ">Number of reducible: %d\n", redcount);
  fprintf(stderr, ">Number of planar graphs: %d\n", plancount);

#if VERBOSE
  fprintf(stderr, ">Prune called %d-times\n", prunecount);
#endif 

#if DEBUG
  fprintf(stderr, ">Planarity test called %d-times\n", planarcount);
#endif
}

