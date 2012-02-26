#include "ogdfbase.h"

using namespace std;
using namespace ogdf;

BoyerMyrvold BM;

// Assumed to be present in a source file including ogdfbase.h:
extern "C++" int cut_along_cycle(Graph & G, List<edge> & cycle);


void print_edge(edge e)
{
  int a = e->source()->index(), b = e->target()->index(), t;
  if (a > b) {
    t = a;
    a = b;
    b = t;
  }
  
  printf("  %d %d", a, b);
}

void print_edge_list(SListPure<edge> & list)
{
  for (SListPure<edge>::iterator it = list.begin(); it.valid(); ++it)
    print_edge(*it);
  printf("\n");
}

void print_edge_list(List<edge> & list)
{
  forall_listiterators(edge, it, list)
    print_edge(*it);
  printf("\n");
}

void print_node_list(List<node> & list)
{
  forall_listiterators(node, it, list)
    printf(" %d", (*it)->index());
  printf("\n");
}

void print_subdivision(KuratowskiSubdivision & S)
{
  int i;
  for (i=S.low(); i<=S.high(); i++) {
    printf("Edge[%d]:", i);
    print_edge_list(S[i]);
//     for (ListIterator<edge> lit = S[i].begin(); lit.valid(); ++lit)
//       print_edge(*lit);
  }
}

void print_graph(Graph & G)
{
  int n = G.numberOfNodes();
  int m = G.numberOfEdges();
  printf("%d %d", n, m);

  Array<edge> list(m);

  int i = 0;
  edge e;
  forall_edges(e, G)
    list[i++] = e;

  EdgeCmp cmp;
  list.quicksort(cmp);

  for (i=0; i<m; i++)
    print_edge(list[i]);
  printf("\n");

}

int read_graph(Graph & G)
{
  G.clear();

  int n, m;
  scanf("%d %d", &n, &m);
  if (feof(stdin))
    return 0;

  vector<node> nodes;

  for (int i = 0; i<n; i++)
    nodes.push_back(G.newNode(i));

  for (int i = 0; i<m; i++) {
    int v, w;
    scanf("%d %d", &v, &w);
    G.newEdge(nodes[v], nodes[w]);
  }

  return 1;
}

int test_planarity(Graph & G)
{
  bool planar;
  planar = BM.planar(G);

  return planar;
}

int test_planarity_with_embedding(Graph & G)
{
  bool planar;

  SList <KuratowskiWrapper> list;
  planar = BM.planarEmbed(G, list, -2);

  return planar;
}


void add(Graph & H, node u, NodeArray<node> & copy, List<node> &cnode, int &length)
{
  assert(u);
  if (copy[u]) return;

  node nn = H.newNode();
  assert(nn);
  copy[u] = nn;
  cnode.pushBack(u);
  length++;
}




void transform(Graph & G, KuratowskiWrapper & K, KuratowskiSubdivision & S)
{
  NodeArray<int> count(G, 0);
  EdgeArray<int> ecount(G, 0);
  BM.transform(K, S, count, ecount);
}

List<edge> & concat(KuratowskiSubdivision & S, const int * edges, int count)
{
  List<edge> & res = *(new List<edge>);
  for (int i=0; i<count; i++) {
    List<edge> tmp = List<edge>(S[edges[i]]);
    res.conc(tmp);
  }
  return res;
}

int cycle_cmp(const void * a, const void * b)
{
  return ((List<edge>*)a)->size() - ((List<edge>*)b)->size();
}

int extract_cycles(Graph & G, KuratowskiWrapper & K, List<edge> * cycles, int &num, int &cnum)
{
  KuratowskiSubdivision S;
  transform(G, K, S);
  if (K.isK33()) {
    CONSTRUCT(S, 0, 4, 0, 1, 3, 4);
    CONSTRUCT(S, 1, 4, 0, 2, 3, 5);
    CONSTRUCT(S, 2, 4, 1, 2, 4, 5);
    cnum = 2;
//     for (int a=0; a<3; a++) 
//       for (int b=0; b<3; b++) {
// 	int i = 3*a+b;
// 	cycles[i] = List<edge>();
// 	List<edge> tmp;
	
// 	for (int c = 0; c<2; c++) 
// 	  for (int d = 0; d<2; d++) {
// 	    int x = (a+c)%3;
// 	    int y = (b+d)%3;
// 	    tmp = List<edge>(S[x*3+y]);
// 	    cycles[i].conc(tmp);
// 	  }
//       }

    return 3;
  } else {
    CONSTRUCT(S, 0, 3, 0, 1, 4);
    CONSTRUCT(S, 1, 3, 0, 2, 5);
    CONSTRUCT(S, 2, 3, 1, 2, 7);
    CONSTRUCT(S, 3, 3, 4, 5, 7);
    cnum = 3;

    return 4;
  }
}

int eval_cycles(Graph & G, KuratowskiWrapper & K, List<edge> * cycle, int & cnum)
{
  int num = extract_cycles(G, K, cycle, cnum);

  qsort(cycle, num, sizeof(List<edge>), cycle_cmp);

  if (DEBUG) {
    for (int i=0; i<num; i++) {
      printf("Cycle of length %d found: ", cycle[i].size());
      print_edge_list(cycle[i]);
    }
  }

  int val = 0;

  for (int i=0; i<cnum; i++)
    val += 1 << (cycle[i].size()-3);

  return val;
}

int best_cycles(Graph & G, KuratowskiWrapper & K)
{
  List<edge> cycles[MAXC];
  int cnum = 0;

  int val = eval_cycles(G, K, cycles, cnum);
  if (DEBUG)
    printf("Kuratowski subgraph of value %d and cycles %d\n", val, cnum);

  for (int i = 0; i<cnum; i++)
    if (cut_along_cycle(G, cycles[i]))
      return 1;
  
  return 0;
}
