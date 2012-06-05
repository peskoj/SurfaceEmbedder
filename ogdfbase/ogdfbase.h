#ifndef INC_OGDFBASE
#define INC_OGDFBASE

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/planarity/BoyerMyrvold.h>
#include <ogdf/graphalg/MinCostFlowReinelt.h>
#include <ogdf/basic/tuples.h>
#include <ogdf/internal/planarity/ConnectedSubgraph.h>
#include <vector>
#include <assert.h>

#ifndef MAXK
#define MAXK 10
#endif

#ifndef MAXN
#define MAXN 100
#endif

#ifndef MAXC
#define MAXC 9
#endif

#ifndef MAXVALUE
#define MAXVALUE 1000000
#endif

#ifndef KURSIZE
#define KURSIZE 6
#endif

#ifndef MAXGENUS
#define MAXGENUS 6
#endif


#ifndef MAXKS
#define MAXKS 20
#endif

#ifndef TREEMIN

#if DEVEL
#define TREEMIN 0
#else
#define TREEMIN 6
#endif

#endif

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef VERBOSE
#define VERBOSE 0
#endif

#define trace(ITERATOR, CONTAINER) for (typeof((CONTAINER).begin()) ITERATOR = (CONTAINER).begin(); ITERATOR != (CONTAINER).end(); ++ITERATOR)
#define trace_while(ITERATOR, CONTAINER) typeof((CONTAINER).begin()) ITERATOR = (CONTAINER).begin(); while (ITERATOR != (CONTAINER).end())
#define SWAP(X, Y, T) {T = X; X = Y; Y = T;}
#define BIT(X, I) (((X) >> (I)) & 1)

//#define forall_slistiterators(type, it, L)  for (SListConstIterator< type > it = (L).begin(); it.valid(); ++it)
#define PRINT_BIN(X, N) for(int _i=0; _i< (N); _i++) {printf("%d", X & 1); X /= 2;}

typedef unsigned int uint;

using namespace std;

const string str[2] = {"non-planar", "planar"};
const string stremb[2] = {"non-embeddable", "embeddable"};
const string strkur[2] = {"K5", "K33"};
const string strsubtype[16] = { "A", "AB", "AC", "AD", "AE1", "AE2", "AE3", "AE4", "B", "C", "D", "E1", "E2", "E3", "E4", "E5" };
const string strmin[2] = {"reducible", "minimal"};
const string strres[2] = {"accepted", "rejected"};
const string strcol[16] = {"Black", "Green", "Silver", "Lime", "Gray", "Olive", "White", "Yellow", "Maroon", "Navy", "Red", "Blue", "Purple", "Teal", "Fuchsia", "Aqua"};
const string strneg[2] = {"not", ""};
const string strsig[2] = {"+", "-"};
const string strcorrect[2] = {"wrong", "correct"};

//Construction of cycles in Kuratowski subgraphs
#define CONSTRUCT(W, X, Y, Z...)  { int _a[2*Y] = {Z}; X.clear(); concat(X, W, _a, Y); }
#define CONSTRUCT_CYCLE(W, X, Y, EDGES, REV, Z...)  {			\
    int _a[2*Y] = {Z};							\
    X.clear();								\
    for (int _j=0; _j < Y; _j++)					\
      append_cycle(X, W, EDGES[_a[_j*2]][_a[_j*2+1]], REV[_a[_j*2]][_a[_j*2+1]]); \
  }
#define CONSTRUCT_CYCLE_LIST(W, X, Y, EDGES, REV, Z...)  {		\
    int _a[2*Y] = {Z};							\
    Cycle * C = new Cycle();						\
    X.push_back(C);							\
    for (int _j=0; _j < Y; _j++)					\
      append_cycle(*C, W, EDGES[_a[_j*2]][_a[_j*2+1]], REV[_a[_j*2]][_a[_j*2+1]]); \
  }
#define EVAL(W, X, Y, Z...)  { int _a[Y] = {Z}; X = sum_lengths(W, _a, Y); }

const int k33_edges[6][3] = {
  {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}
};

const int k33_rev[6][3] = {
  {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}
};

const int k5_edges[5][4] = {
  {0, 1, 2, 3}, {0, 4, 5, 6}, {1, 4, 7, 8}, {2, 5, 7, 9}, {3, 6, 8, 9}
};

const int k5_rev[5][4] = {
  {0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {1, 1, 1, 0}, {1, 1, 1, 1}
};

using namespace ogdf;

typedef Tuple2<int,int> IntPair;

inline int min(int a, int b) 
{
  return (a<b)?a:b;
}

inline int max(int a, int b) 
{
  return (a<b)?b:a;
}

inline int two_bits(int x)
{
  return (x-1)&x;
}

inline int all_bits(int x)
{
  return 1;
}


class EdgeCmp: public VComparer < edge >
{
  int compare(const edge &x, const edge &y) const {
    int ax = x->source()->index();
    int bx = x->target()->index();
    int ay = y->source()->index();
    int by = y->target()->index();
    
    if (min(ax, bx) != min(ay, by)) 
      return min(ax, bx) - min(ay, by);

    if (max(ax, bx) != max(ay, by)) 
      return max(ax, bx) - max(ay, by);
    
    return 0;
  }
};

template <typename A>
int intersect(List<A> & a, List<A> & b)
{
  ListIterator<A> ita = a.begin();
  ListIterator<A> itb = b.begin();
  while (ita.valid()) {
    while (itb.valid() && *itb < *ita)
      ++itb;
    if (itb.valid() && *ita == *itb)
      return 1;
    ++ita;
  }
  return 0;
}

template <typename A>
A * first_intersect(List<A*> & a, List<A*> & b)
{
  ListIterator<A*> ita = a.begin();
  ListIterator<A*> itb = b.begin();
  while (ita.valid()) {
    while (itb.valid() && *itb < *ita)
      ++itb;
    if (itb.valid() && *ita == *itb)
      return *ita;
    ++ita;
  }
  return 0;
}

template <typename A>
int contains(List<A> & L, A & x)
{
  forall_listiterators(A, it, L)
    if (*it == x)
      return 1;

  return 0;
}

template <typename E>
int list_remove(List<E> & L, E x)
{
  ListIterator<E> it = L.begin();
  while (it.valid()) {
    if (*it == x) {
      L.del(it);
      return 1;
    }
    it++;
  }

  return 0;
}

template <typename E>
void list_append(List<E> & to, List<E> & from)
{
  ListIterator<E> it = from.begin();
  forall_listiterators(E, it, from)
    to.pushBack(*it);
}

template <typename E>
void set_array(E * a, int count, E x)
{
  for (int i=0; i<count; i++)
    a[i] = x;
}

/* template<class T> void swap(T & a, T & b) */
/* { */
/*   std::swap(a,b); */
/* } */

//------------------------- printing -----------------------------------


template <typename E>
void print_item(E & item)
{
  printf(" ???");
}

template <typename LIST, typename IT > 
  void print_list(LIST & list, int breakline = 1, string delim = " ")
{
  for (IT it = list.begin(); it.valid(); ++it) {
    if (it != list.begin())
      printf("%s", delim.c_str());
    print_item(*it);
  }
  if (breakline)
    printf("\n");
}

//------------------------ typedefs -----------------------------------

typedef List<edge> Cycle;

class GraphCutter: public GraphCopySimple
{
 private:

#if DEBUG
  int rcount;
  long long rcode;
#endif

  Graph & orig;

  vector<adjEntry> halfedges;
  AdjEntryArray<int> orient;
  AdjEntryArray<edge> eorient;
  AdjEntryArray<edge> forient;
  EdgeArray<int> cedge;
  NodeArray<int> cnode;
  List<node> nodes;

  EdgeArray<edge> ecopy;
  NodeArray<node> ncopy;

  List<edge> out;
  List<edge> cycle;

  node center;
  List<edge> mod;

#if DISK_DEVEL
  Disk * disk;
  int disk_emb;
  NodeArray<int> indisk;
#endif

  void duplicate(node u);
  void duplicate_cycle();
  void create_one_disk();
  void create_two_disks();

  void orient_edges(List<edge> & edges);
  inline void show_edge(edge e);
  inline void clear_edges();
  inline void hide_edges();

  int test_all_codes(int (GraphCutter::*test)(), int (*validcode)(int));
  int test_tree_codes(int (GraphCutter::*test)());
  int test_tree_codes_recurse(List<edge>::iterator it, int (GraphCutter::*test)());

  int planar();

  int test_disks_proj();
  int test_disks_klein1();
  int test_disks_klein2();
  int test_disks_torus();
  int test_jump(int (*test)(Graph &));

  void init();
 public:
 GraphCutter(Graph & G, List<edge> & c) : GraphCopySimple(G), orig(G), orient(G, 0), eorient(G, 0), forient(G, 0), 
    cedge(G, 0), cnode(G, 0), ecopy(*this, 0), ncopy(*this, 0), cycle(c)
#if DISK_DEVEL
    ,disk(0), disk_emb(0)
#endif
      {}

  int cut_along_cycle_torus();
  int cut_along_cycle_proj();
  int cut_along_cycle_klein1();
  int cut_along_cycle_klein2();

#if DISK_DEVEL
  int set_fixed_disk(Disk * d);
#endif

};

//---------------- small functions ---------------------------------------

int int_cmp(const void * a, const void * b);
int cycle_cmp(const void * a, const void * b);
int value_function(int x);
bool cycle_cmp_bool(const Cycle & a, const Cycle & b);


//------------------- Exported functions ---------------------------------


void print_edge(edge e, string delim = "  ");
char * print_edge_str(edge e);
string print_edge_string(edge e);
void print_arc(adjEntry a);
char * print_arc_str(adjEntry a);
string print_arc_string(adjEntry a);
void print_edge_list(SListPure<edge> & list);
void print_edge_list(const List<edge> & list);
void print_node_list(const List<node> & list, int linebreak = 1);
void print_subdivision(KuratowskiSubdivision & S);
void print_graph(Graph & G);
void print_graph(Graph & G, NodeArray<int> & node_index);
void print_graph_fast(Graph & G);
void print_graph_graph6(Graph & G);
void print_local_emb(node v);
void print_emb(Graph & G);
void print_emb(Graph & G, EdgeArray<int> & orient, int genus);
int index(node u);

int read_graph(Graph & G);
int read_graph_graph6(Graph & G);

node subdivide_edge(Graph & G, edge e);
edge cubic_new_edge(Graph & G, edge e, edge f);

int test_planarity(Graph & G);
int test_planarity_with_embedding(Graph & G);
int test_planarity_bounded(Graph & G, int c, SList<KuratowskiWrapper> &output);


void remove_isolated(Graph & G);
adjEntry get_adj(edge e, node v);
int min_cut(Graph & G, node a, node b, NodeArray<int> & cut);
int shortest_path(Graph & G, node a, node b, NodeArray<int> & used, EdgeArray<int> & flow);
int k_connected(Graph & G, node a, node b, int k);
int color_comp(NodeArray<int> & nodes, EdgeArray<int> & edges, int subg, node v, int c);
int two_con_comp(Graph & G, NodeArray<int> & nodes, int subg, NodeArray<int> & comps, node v);
node BFS_subgraph(Graph & G, EdgeArray<int> & div, int subg, node source, NodeArray<int> & visited, int color, NodeArray<int> & target, NodeArray<adjEntry> & path, int shortest_only);
void construct_path(node source, node sink, NodeArray<adjEntry> & dir, List<edge> & path);
node BFS(Graph & G, node source, NodeArray<int> & visited, NodeArray<int> & target, NodeArray<adjEntry> & path);

//----------- Kuratowski graphs ---------------------------
void kuratowski_nodes(KuratowskiSubdivision & S, vector<node> & nodes, int isK33);
int best_k_graph(Graph & G, KuratowskiWrapper & R);


void duplicate(GraphCopySimple & H, node u, NodeArray<node> & copy, NodeArray<int> & cnode, List<node> & nodes);

void transform(Graph & G, KuratowskiWrapper & K, KuratowskiSubdivision & S);

void concat(Cycle & C, KuratowskiSubdivision & S, const int * edges, int count);
void append_cycle(Cycle & C, KuratowskiSubdivision & S, int edge, int rev);
int is_cycle(Cycle & C);

int findpairs(Graph & G, vector<adjEntry> &halfedges, NodeArray<int> & cnode, vector< Tuple2<int,int> > & pairs);

int eval_cycles(Graph & G, KuratowskiWrapper & K);
int construct_cycles(Graph & G, KuratowskiWrapper & K, List<edge> * cycle, int & cnum);
int best_cycles(Graph & G, KuratowskiWrapper & K, Cycle * cycles);

int test_minimality(Graph & G);
int test_edge_deletion(Graph & G, int (*test)(Graph & G));

int test_torus(Graph & G);
int test_torus_multiple(Graph & G);
int test_torus_cutter(Graph & G);

int test_projective(Graph & G);
int test_projective_multiple(Graph & G);
int test_projective_cutter(Graph & G);

int test_klein(Graph & G);
int test_klein_multiple(Graph & G);
int test_klein_cutter(Graph & G);

int graph_genus(Graph & G);
int graph_genus_nonorientable(Graph & G);


template <typename E>
int traverse_faces(Graph & G, AdjEntryArray<E> & visited, NodeArray< List<E> > & faces)
{
  visited.init(G, NULL);
  faces.init(G);

  int fc = 1;
  node u;
  forall_nodes(u, G) {
    adjEntry a;
    forall_adj(a, u) {
      if (visited[a])
	continue;

#if DEBUG
      printf("Traversing face %d:", fc);
#endif

      E f = E(fc++);
      traverse_face(G, f, a, visited, faces);

#if DEBUG
      printf("\n");
#endif

    }
  }
  return fc;
}

template <typename E>
void traverse_face(Graph & G, E face, adjEntry a, AdjEntryArray<E> & visited, NodeArray< List<E> > & faces)
{
  if (visited[a])
    return;
  assert(face);
  visited[a] = face;

  node u = a->theNode();
  faces[u].pushBack(face);

#if DEBUG
  printf(" %d", u->index());
#endif

  adjEntry next = a->twin()->cyclicSucc();
  traverse_face(G, face, next, visited, faces);
}

#endif
