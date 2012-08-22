#include "ogdfbase.h"
#include "json/json.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace ogdf;

#if STATISTICS
int planarcount = 0;
#endif

//------------------------------------ small functions ------------------------------

int int_cmp(const void * a, const void * b)
{
  return *((int*)a) - *((int*)b);
}

int cycle_cmp(const void * a, const void * b)
{
  return ((List<edge>*)a)->size() - ((List<edge>*)b)->size();
}

int value_function(int x)
{
  //1 << (cycles[i]-3); old exponential function
  return x*x*x;
}

bool cycle_cmp_bool(const Cycle & a, const Cycle & b)
{
  return a.size() < b.size();
}


int edgecmp(const void * px, const void * py) {
  edge x = *((edge *)(px));
  edge y = *((edge *)(py));
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

string int2string(int x) {
  stringstream strm;
  string num;
  strm << x;
  strm >> num;
  return num;
}

//--------------------- Input & output -----------------------------

void print_edge(edge e, string delim)
{

  int a = e->source()->index(), b = e->target()->index(), t;
  if (a > b) 
    SWAP(a, b, t);
  
  printf("%s%d %d", delim.c_str(), a, b);
}

void print_ord(int a, int b)
{

  int t;
  if (a > b) 
    SWAP(a, b, t);
  
  printf("  %d %d", a, b);
}

char buf[50];
char * print_edge_str(edge e)
{

  int a = e->source()->index(), b = e->target()->index(), t;
  if (a > b) 
    SWAP(a, b, t);
  
  sprintf(buf, "%d %d", a, b);
  return buf;
}

string print_edge_string(edge e)
{

  int a = e->source()->index(), b = e->target()->index(), t;
  if (a > b) 
    SWAP(a, b, t);
  
  sprintf(buf, "%d %d", a, b);
  
  string res = buf;
  return res;
}

void print_arc(adjEntry a)
{
  printf(" %d->%d", a->theNode()->index(), a->twinNode()->index());
}

char * print_arc_str(adjEntry a)
{
  sprintf(buf, " %d->%d", a->theNode()->index(), a->twinNode()->index());
  return buf;
}

string print_arc_string(adjEntry a)
{
  sprintf(buf, "%d->%d", a->theNode()->index(), a->twinNode()->index());

  string res = buf;
  return res;
}


void print_edge_list(SListPure<edge> & list)
{
  for (SListPure<edge>::iterator it = list.begin(); it.valid(); ++it)
    print_edge(*it);
  printf("\n");
}

void print_edge_list(const List<edge> & list)
{
  forall_listiterators(edge, it, list)
    print_edge(*it);
  printf("\n");
}

void print_node_list(const List<node> & list, int linebreak)
{
  forall_listiterators(node, it, list)
    printf(" %d", (*it)->index());
  if (linebreak)
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

void print_graph_array(Graph & G)
{
  int n = G.numberOfNodes();
  int mm = G.numberOfEdges();

  edge list[mm];

  int m = 0;
  edge e;
  forall_edges(e, G)
    list[m++] = e;

  printf("%d %d", n, m);

  qsort(list, m, sizeof(edge), edgecmp);

  for (int i=0; i<m; i++)
    print_edge(list[i]);

  printf("\n");

}

void print_graph_fast(Graph & G)
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

void print_graph(Graph & G, NodeArray<int> & ind)
{
  int n = G.numberOfNodes();
  int m = G.numberOfEdges();
  printf("%d %d", n, m);

  Array<edge> list(m);

  ind.init(G, 0);
  node u;
  int a = 0;
  forall_nodes(u, G)
    ind[u] = a++;

  int i = 0;
  edge e;
  forall_edges(e, G)
    list[i++] = e;

  EdgeCmp cmp;
  list.quicksort(cmp);

  for (i=0; i<m; i++)
    print_ord(ind[list[i]->source()], ind[list[i]->target()]);
  printf("\n");
}

void print_graph(Graph & G)
{
  NodeArray<int> ind(G, 0);
  print_graph(G, ind);
}

void print_graph_color(Graph & G, NodeArray<int> & ncolor, EdgeArray<int> & ecolor)
{
  printf("graph G {\n");
  printf("node [shape=circle, color=red, width=0.2, label=\"?\"];\n");
  node v;
  forall_nodes(v, G) {
    printf("n%d [color=%s, label=\"%d\"];\n", v->index(), strcol[ncolor[v]].c_str(), v->index());
  }
  edge e;
  forall_edges(e, G) {
    printf("n%d -- n%d [color=%s];\n", e->source()->index(), e->target()->index(), strcol[ecolor[e]].c_str());
  }
  printf("}\n");
}

void print_kuratowski(Graph & G, KuratowskiWrapper & K)
{
  NodeArray<int> ncolor(G, 0);
  EdgeArray<int> ecolor(G, 0);

  forall_slistiterators(edge, it, K.edgeList) 
    ecolor[*it] = 1;

  print_graph_color(G, ncolor, ecolor);
}

void print_local_emb(node v)
{
  printf("%d:[", v->index());
  adjEntry a;
  forall_adj(a, v) {
    printf("%d", a->twinNode()->index());
    if (a->succ())
      printf(", ");
  }
  printf("]\n");
}

void print_emb(Graph & G, EdgeArray<int> & orient, int genus)
{
  printf("Embedding of genus %d:\n", genus);

  printf("{");
  node v;
  forall_nodes(v, G) {
    adjEntry a;
    printf("\"%d\":[", v->index());
    if (v->degree())
      forall_adj(a, v) {
	string sign = "-";
	if (orient[a->theEdge()] > 0)
	  sign = "";
	printf("\"%s%d\"", sign.c_str(), a->twinNode()->index());
	if (a->succ())
	  printf(", ");
      }
    printf("]");
    if (v != G.lastNode())
      printf(", ");
  }

  printf("}\n");
}

void print_emb(Graph & G)
{
  EdgeArray<int> signature(G, 1);

  print_emb(G, signature, 0);
}

int read_emb(Graph & G, EdgeArray<int> & signature)
{
  G.clear();

  vector<node> nodes(MAXN, NULL);

  string s;
  if (feof(stdin))
    return 0;
  getline(cin, s);
#if DEBUG
  cout << "Reading embedding: " << s << endl;
#endif

  Json::Value root;
  Json::Reader reader;
  bool succ = reader.parse(s, root);

  if (!succ) {
#if DEBUG
    cout << "Reader failed to parse the string: " << reader.getFormatedErrorMessages() << endl;
#endif
    return 0;
  }

  trace(it, root) {
    Json::Value v = *it;
    int x = atoi(it.memberName());
    if (!nodes[x])
      nodes[x] = G.newNode(x);

    trace(eit, v) {
      Json::Value u = *eit;
      int sign = 1;
      const char * str = u.asCString();
      assert(str);
      if (str[0] == '-') {
	sign = -1;
	str++;
      }

      int y = atoi(str);
      if (!nodes[y])
	nodes[y] = G.newNode(y);
      edge e = G.searchEdge(nodes[x], nodes[y]);
      if (!e) {
	e = G.newEdge(nodes[x], nodes[y]);
	signature[e] = sign;
      }
      assert(signature[e] == sign);

      adjEntry a = e->adjSource();
      if (a->theNode() != nodes[x])
	a = a->twin();

      if (a != nodes[x]->lastAdj())
	G.moveAdjAfter(a, nodes[x]->lastAdj());
    }
  }

  return 1;
}

int index(node u) {
  if (u)
    return u->index();
  else
    return -1;
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

#if DEBUG
    if (!(v < n && w < n))
      printf("Node index %d out of bounds (%d)\n", max(v, w), n);
#endif

    G.newEdge(nodes[v%n], nodes[w%n]);
  }

  return 1;
}

int read_graph_graph6(Graph & G)
{
  G.clear();

  int n, m;
  unsigned char c;
  scanf("%c", &c);
  n = c - 63;
  m = (n*(n-1))/2;

  if (feof(stdin))
    return 0;

  vector<node> nodes;

  for (int i = 0; i<n; i++)
    nodes.push_back(G.newNode(i));

  int b = 1;
  while (6*b < m) 
    b++;

  int x = 0, y = 1;
  for (int i = 0; i < b; i++) {
    scanf("%c", &c);
    c -= 63;
    for (int j = 0; j < 6 && y < n; j++) {
      if (c & (0x20 >> j))
	G.newEdge(nodes[x], nodes[y]);
      x++;
      if (x == y) {
	x = 0;
	y++;
      }
    }
  }
  scanf("\n");
  
  return 1;
}

void print_graph_graph6(Graph & G)
{
  int n = G.numberOfNodes();
  printf("%c", n+63);
  
  vector<node> nodes;

  node v;
  forall_nodes(v, G)
    nodes.push_back(v);

  int b = 0;
  unsigned char c = 0;
  for (int y=1; y<n; y++)
    for (int x=0; x<y; x++) {
      c = 2*c + (!!G.searchEdge(nodes[x], nodes[y]));
      b++;
      if (b == 6) {
	printf("%c", c+63);
	b = 0;
	c = 0;
      }
    }
  if (b > 0) 
    printf("%c", (c << (6 - b)) + 63);
  printf("\n");
}

//------------------------- Graph Operations ----------------------------------

void remove_isolated(Graph & G)
{
  List<node> del;

  node v;
  forall_nodes(v, G)
    if (!v->degree())
      del.pushBack(v);

  forall_listiterators(node, it, del)
    G.delNode(*it);
}

node subdivide_edge(Graph & G, edge e)
{
  edge f = G.split(e);
  node w = e->commonNode(f);

  return w;
}

edge cubic_new_edge(Graph & G, edge e, edge f)
{
  edge ce = e;
  edge cf = f;
  edge ne = G.split(ce);
  edge nf = G.split(cf);
  node en = ce->commonNode(ne);
  node fn = cf->commonNode(nf);
  return G.newEdge(en, fn);
}

//------------------------- Planarity tests -----------------------------------

int test_planarity(Graph & G)
{
  bool planar;

  //  assert(isSimpleUndirected(G));

#if DEBUG
  printf("Test planarity\n");
  assert(G.consistencyCheck());
  if(!isConnected(G))
    printf("Warning: Graph is not connected!\n");
#endif

  BoyerMyrvold BM;
  planar = BM.planar(G);

#if STATISTICS
  planarcount++;
#endif

  return planar;
}

int test_planarity_with_embedding(Graph & G)
{
  bool planar;

  assert(isSimpleUndirected(G));

#if DEBUG
  printf("Test planarity with embedding\n");
  assert(G.consistencyCheck());
  if(!isConnected(G))
    printf("Warning: Graph is not connected!\n");
#endif

  SList<KuratowskiWrapper> list;
  BoyerMyrvold BM;
  //planar = BM.planarEmbed(G, list, 0, false);
  planar = BM.planarEmbed(G, list, -2);

#if STATISTICS
  planarcount++;
#endif

  return planar;
}

int test_planarity_bounded(Graph & G, int c, SList<KuratowskiWrapper> &output)
{
  bool planar;

  assert(isSimpleUndirected(G));

#if DEBUG
  assert(G.consistencyCheck());
  if(!isConnected(G))
    printf("Warning: Graph is not connected!\n");
#endif

  BoyerMyrvold BM;
   planar = BM.planarEmbed(G, output, c, false);

#if STATISTICS
  planarcount++;
#endif

  return planar;
}

//----------------------- Various functions ----------------------------------

adjEntry get_adj(edge e, node v)
{
  if (e->source() == v)
    return e->adjSource();
  if (e->target() == v)
    return e->adjTarget();

  return 0;
}

inline int getFlow(edge e, node v, EdgeArray<int> & flow)
{
  return (flow[e]*(2*(e->source() == v) - 1));
}

int flow_BFS(Graph & G, node a, node b, NodeArray<int> & visited, NodeArray<edge> & reached, NodeArray<edge> & hreached, NodeArray<int> & used, EdgeArray<int> & flow)
{
  int n = G.numberOfNodes()*2;
  Array<node> queue(n);
  Array<int> head(n);
  NodeArray<int> vhead(G, 0);
  int ql, qr;

  queue[0] = a;
  head[0] = 1;
  visited[a] = 1;
  vhead[a] = 1;
  ql = 0;
  qr = 1;
  while (ql < qr && !visited[b]) {
    int h = head[ql];
    node u = queue[ql++];

#if DEBUG_FLOW
    printf("At node %d head %d, used %d, visited %d, vhead %d\n", u->index(), h, used[u], visited[u], vhead[u]);
#endif

    edge e;
    forall_adj_edges(e, u) {
      node v = e->opposite(u);

#if DEBUG_FLOW 
      printf("Probing node %d, visited %d, vhead %d, used %d, with flow %d\n", v->index(), visited[v], vhead[v], used[v], getFlow(e, u, flow)); 
#endif
      
      int f = getFlow(e, u, flow);
      // if (f > 0)
      // 	continue;

      if (h && (visited[v] || f))
	continue;

      if (!h && (vhead[v] || f >= 0))
	continue;

      // if (used[u] && !getFlow(reached[u], u, flow) && !getFlow(e, u, flow))
      // 	continue;

      head[qr] = !h;
      queue[qr++] = v;
      if (h) {
	visited[v] = 1;
	reached[v] = e;
      } else {
	vhead[v] = 1;
	hreached[v] = e;
      }

#if DEBUG 
      printf("Adding node %d\n", v->index()); 
#endif
    }

    if (h && !visited[u]) {
      visited[u] = 1;
      head[qr] = 0;
      queue[qr++] = u;
      reached[u] = NULL;
    }

    if (!h && !vhead[u] && !used[u]) {
      vhead[u] = 1;
      head[qr] = 1;
      queue[qr++] = u;
      hreached[u] = NULL;
    }
  }

  return visited[b];
}

int shortest_path(Graph & G, node a, node b, NodeArray<int> & used, EdgeArray<int> & flow)
{
  if (DEBUG)
    printf("Shortest path from %d to %d\n", a->index(), b->index());

  NodeArray<int> visited(G, 0);
  NodeArray<edge> reached(G, 0);
  NodeArray<edge> hreached(G, 0);

  flow_BFS(G, a, b, visited, reached, hreached, used, flow);

  if (!visited[b])
    return 0;

  node u = reached[b]->opposite(b);
  int head = 1;
  while (u != a) {
    edge e;
    if (head) 
      e = hreached[u];
    else
      e = reached[u];

    if (!e) {
      used[u] = !used[u];
      head = !head;
      continue;
    }

    if (flow[e]) 
      flow[e] = 0;
    else 
      flow[e] += 2*(e->target() == u) - 1;
    
#if DEBUG_FLOW
    printf("Flow %d %d: %d\n", e->source()->index(), e->target()->index(), flow[e]); 
#endif

    u = e->opposite(u);
    if (head) 
      e = hreached[u];
    else 
      e = reached[u];

    head = !head;
  }

  return 1;
}

int min_cut(Graph & G, node a, node b, NodeArray<int> & cut)
{
  NodeArray<int> used(G, 0);
  EdgeArray<int> flow(G, 0);

  int s = 0;
  while (shortest_path(G, a, b, used, flow))
    s++;

#if DEBUG
  printf("Determining min cut of size %d\n", s); 
#endif

  NodeArray<edge> reached(G, 0);
  NodeArray<edge> hreached(G, 0);
  cut.init(G, 0);

  flow_BFS(G, a, b, cut, reached, hreached, used, flow);
  
  return s;
}

int k_connected(Graph & G, node a, node b, int k = 3)
{
  NodeArray<int> used(G, 0);
  EdgeArray<int> flow(G, 0);

  int i;
  for (i=0; i<k; i++)
    if (!shortest_path(G, a, b, used, flow))
      break;

#if DEBUG
  printf("The graph is %d-connected\n", i);
#endif
  
#if DEBUG_FLOW
  node u;
  forall_nodes(u, G) {
    printf("[%d: %d] ", u->index(), used[u]); 
  }
  printf("\n");
  edge e;
  forall_edges(e, G) {
    printf("[%d %d: %d] ", e->source()->index(), e->target()->index(), flow[e]); 
  }
  printf("\n");
#endif

  return i == k;
}

int color_comp(NodeArray<int> & nodes, EdgeArray<int> & edges, int subg, node v, int c)
{
  if (nodes[v] != subg)
    return 0;

#if DEBUG
  printf("In color_comp at node %d color %d\n", v->index(), c);
#endif

  assert(c);
  nodes[v] = c;

  edge e;
  forall_adj_edges(e, v) {
    if (edges[e] != subg)
      continue;

    edges[e] = c;

    node u = e->opposite(v);

    color_comp(nodes, edges, subg, u, c);
  }

  return 1;
}

int two_con_comp_parent(NodeArray<int> & nodes, int subg, NodeArray<int> & parent, node v, node from, int depth)
{
#if DEBUG
  printf("In two_con_comp_parent at node %d depth %d orig %d\n", v->index(), depth, parent[v]);
#endif

  if (parent[v])
    return parent[v];

  parent[v] = depth;

  int m = depth;
  edge e;
  forall_adj_edges(e, v) {
    node u = e->opposite(v);

    if (nodes[u] != subg || u == from)
      continue;

    int d = two_con_comp_parent(nodes, subg, parent, u, v, depth+1);
    m = min(d, m);
  }
  
  parent[v] = m;

  return m;
}

int two_con_comp_set(NodeArray<int> & nodes, int subg, NodeArray<int> & parent, NodeArray<int> & comps, node v, node from, int depth, int cur, int & lastc)
{
  if (parent[v] < depth)
    comps[v] = cur;
  else 
    comps[v] = ++lastc;

#if DEBUG
  printf("In two_con_comp_set at node %d depth %d parent %d comp %d\n", v->index(), depth, parent[v], comps[v]);
#endif

  edge e;
  forall_adj_edges(e, v) {
    node u = e->opposite(v);

    if (nodes[u] != subg || u == from || comps[u])
      continue;

    two_con_comp_set(nodes, subg, parent, comps, u, v, depth+1, comps[v], lastc);
  }

  return lastc;
}

int two_con_comp(Graph & G, NodeArray<int> & nodes, int subg, NodeArray<int> & comps, node v)
{
  NodeArray<int> parent(G, 0);
  two_con_comp_parent(nodes, subg, parent, v, NULL, 1);
  int c = 0;
  two_con_comp_set(nodes, subg, parent, comps, v, NULL, 1, c, c);

  return c;
}

node BFS(Graph & G, node source, NodeArray<int> & visited, NodeArray<int> & target, NodeArray<adjEntry> & path)
{
  vector<node> queue(G.numberOfNodes());
  int ql = 0, qr = 0;

  visited[source] = 1;
  queue[qr++] = source;

  while (ql < qr) {
    node u = queue[ql++];

// #if DEBUG
//     printf("In BFS at %d\n", u->index());
// #endif

    adjEntry a;
    forall_adj(a, u) {
      node v = a->twinNode();
      if (visited[v])
	continue;

      path[v] = a->twin();
      if (target[v])
	return v;

      assert(qr < G.numberOfNodes());
      queue[qr++] = v;
      visited[v] = 1;
    }
  }

  return 0;
}

void construct_path(node source, node sink, NodeArray<adjEntry> & dir, List<edge> & path)
{
  node u = source;
  int count = 0;
  while (u != sink) {
    assert(dir[u]);
    path.pushBack(dir[u]->theEdge());
    u = dir[u]->twinNode();

    assert(count++ < 1000); //Infinite cycle protection
  }
}

bool node_on_path(node u, List<edge> & path) 
{
  forall_listiterators(edge, it, path)
    if ((*it)->isIncident(u))
      return true;
  return false;
}

node BFS_subgraph(Graph & G, EdgeArray<int> & div, int subg, node source, NodeArray<int> & visited, int color, NodeArray<int> & target, NodeArray<adjEntry> & path, int shortest_only = 1)
{
// #if DEBUG
//   printf("In BFS_subgraph, source: %d, color: %d\n", source->index(), color);
// #endif

  assert(color);
  node shortest = 0;

  vector<node> queue(G.numberOfNodes());
  int ql = 0, qr = 0;

  visited[source] = color;
  queue[qr++] = source;

  while (ql < qr) {
    node u = queue[ql++];

// #if DEBUG
//     printf("In BFS at %d\n", u->index());
// #endif

    adjEntry a;
    forall_adj(a, u) {
      edge e = a->theEdge();
      if (div[e] != subg)
	continue;

      node v = a->twinNode();

      if (visited[v])
	continue;
      visited[v] = color;

      path[v] = a->twin();

      if (target[v]) {
	if (!shortest)
	  shortest = v;
	if (shortest_only)
	  return v;
	
      } else {
	assert(qr < G.numberOfNodes());
	queue[qr++] = v;
      }
    }
  }

  return shortest;
}

int DFS(Graph & G, NodeArray<int> & visited, EdgeArray<int> & span, node v)
{
  visited[v] = 1;
  
  edge e;
  forall_adj_edges(e, v) {
    node u = e->opposite(v);

    if (visited[u]) continue;

    span[e] = 1;
    DFS(G, visited, span, u);
  }

  return 0;
}

void all_paths_DFS(Graph & G, EdgeArray<int> & subg, int subgi, node act, NodeArray<int> & target, List<Path> & res, NodeArray<int> & visited, List<edge> & path)
{
  edge e;
  forall_adj_edges(e, act) {
    if (subg[e] != subgi)
      continue;

    node u = e->opposite(act);

    if (visited[u]) 
      continue;

    visited[u] = 1;
    path.pushBack(e);
  
    if (target[u]) {
      res.pushBack(Path(u, path));
    } else
      all_paths_DFS(G, subg, subgi, u, target, res, visited, path);

    path.popBack();
    visited[u] = 0;
  }
  
}

void all_paths(Graph & G, EdgeArray<int> & subg, int subgi, node source, NodeArray<int> & target, List<Path> & res)
{
  NodeArray<int> visited(G, 0);
  NodeArray<adjEntry> paths(G, 0);
  List<edge> path;

  visited[source] = 1;
  all_paths_DFS(G, subg, subgi, source, target, res, visited, path);
}


void transform(Graph & G, KuratowskiWrapper & K, KuratowskiSubdivision & S)
{
  NodeArray<int> count(G, 0);
  EdgeArray<int> ecount(G, 0);

  BoyerMyrvold BM;
  BM.transform(K, S, count, ecount);
}

void concat(Cycle & cycle, KuratowskiSubdivision & S, const int * edges, int count)
{
  for (int i=0; i<count; i++)
    forall_listiterators(edge, it, S[edges[i]]) {
      cycle.pushBack(*it);
    }
}

void append_cycle(Cycle & C, KuratowskiSubdivision & S, int e, int rev) //needs debugging
{
#if DEBUG2
  printf("Appending %d, reverse %d\n", e, rev);
#endif
  if (rev)
    forall_rev_listiterators(edge, it, S[e])
      C.pushBack(*it);
  else
    forall_listiterators(edge, it, S[e])
      C.pushBack(*it);
}

int is_cycle(Cycle & C)
{
  forall_listiterators(edge, it, C) {
    edge e = *it;
    edge f = *(C.cyclicSucc(it));
    if (!e->commonNode(f))
      return 0;
  }

  return 1;
}

int sum_lengths(KuratowskiSubdivision & S, const int * edges, int count)
{
  int res = 0;
  for (int i=0; i<count; i++) 
    res += S[edges[i]].size();

  return res;
}


//------------------------ Non-contractible cycles ----------------------------------


void kuratowski_nodes(KuratowskiSubdivision & S, vector<node> & nodes, int isK33)
{
#if (DEBUG)
  printf("In kuratowski_nodes\n");
#endif

  if (isK33) {
    nodes.resize(6);
    for (int i = 0; i<3; i++) {
      edge e = S[3*i].front();
      edge f = S[3*i+1].front();
      nodes[i] = e->commonNode(f);
    }

    for (int i = 0; i<3; i++)
      nodes[3+i] = S[i].back()->commonNode(S[i+3].back());

    for (int i = 0; i<6; i++)
      assert(nodes[i]);
  } else {
    nodes.resize(5);

    nodes[0] = S[0].front()->commonNode(S[1].front());
    nodes[1] = S[4].front()->commonNode(S[5].front());
    nodes[2] = S[7].front()->commonNode(S[8].front());
    nodes[3] = S[2].back()->commonNode(S[7].back());
    nodes[4] = S[3].back()->commonNode(S[6].back());

    for (int i = 0; i<5; i++)
      assert(nodes[i]);
  }
}

int eval_cycles(Graph & G, KuratowskiWrapper & K)
{
  KuratowskiSubdivision S;
  transform(G, K, S);

  int minval = MAXVALUE;//1 << (G.numberOfNodes()-3); //G.numberOfEdges() + 1;

  int cycles[MAXC];
  if (K.isK33()) {
    int cnum = 2;
    int num = 3;

    for (int i = 0; i<6; i++) {
      int a = (i / 3)*2 + 1;
      int b = 3 - (i / 3)*2;
      int c = i % 3;
      
      for (int j = 0; j<3; j++) {
#if DEBUG2
	printf("Evaluating cycle out of: %d, %d, %d, %d\n", (c*b + j*a)%9, (c*b + ((j+1)%3)*a)%9, (((c+1)%3)*b + j*a)%9, (((c+1)%3)*b + ((j+1)%3)*a)%9);
#endif
	EVAL(S, cycles[j], 4, (c*b + j*a)%9, (c*b + ((j+1)%3)*a)%9, (((c+1)%3)*b + j*a)%9, (((c+1)%3)*b + ((j+1)%3)*a)%9);
      }

      qsort(cycles, num, sizeof(int), int_cmp);


      int val = 0;
      for (int j=0; j<cnum; j++)
	val += value_function(cycles[j]);

#if DEBUG2
      printf("Cycles of value %d, lengths:", val);
      for (int j=0; j<num; j++) 
	printf(" %d", cycles[j]);
      printf("\n");
#endif

      if (val < minval) 
	minval = val;
    }

  } else {
    int cnum = 3;
    int num = 4;

    EVAL(S, cycles[0], 3, 0, 1, 4);
    EVAL(S, cycles[1], 3, 0, 2, 5);
    EVAL(S, cycles[2], 3, 1, 2, 7);
    EVAL(S, cycles[3], 3, 4, 5, 7);

    qsort(cycles, num, sizeof(int), int_cmp);

    int val = 0;
    for (int i=0; i<cnum; i++)
      val += value_function(cycles[i]);
    
    if (val < minval) 
      minval = val;
  }

  return minval;
}

int best_k_graph(Graph & G, KuratowskiWrapper & R)
{
#if DEBUG
  printf("In best_k_graph:\n");
#endif

  SList<KuratowskiWrapper> output;

  int c = MAXK;
  int planar = test_planarity_bounded(G, c, output);

#if DEBUG
  printf("%d Kuratowski subdivision found\n", output.size());
  printf("The graph is %s.\n", str[planar].c_str());
#endif

  if (planar)
    return 1;
  
  int m = MAXVALUE;//1 << (G.numberOfNodes()-3); //G.numberOfEdges() + 1;
  KuratowskiWrapper * K = 0;

  for (SListIterator<KuratowskiWrapper> it = output.begin(); it != output.end(); it++) {
    KuratowskiWrapper & L = *it;

#if DEBUG
    int size = L.edgeList.size();
    
    printf("%s, subtype %s, of size %d at node %d\n", strkur[L.isK33()].c_str(), strsubtype[L.subdivisionType].c_str(), size, L.V->index());

    printf("Edges:");
    print_edge_list(L.edgeList);
#endif

    int value = eval_cycles(G, L);

#if DEBUG
    printf("Value: %d, minimum: %d\n", value, m);
#endif

    if (value < m) {
      K = &L;
      m = value;
    }
  }

  assert(K);
  R = *K;

  return 0;
}

//------------------------- Graph Genus ----------------------------------

int compute_faces(Graph & G, EdgeArray<int> & orient)
{
#if DEBUG
  printf("In compute_faces\n");
#endif

  AdjEntryArray<int> visited(G, 0);
  
  int faces = 0;
  
  node v;
  forall_nodes(v, G) {

// #if DEBUG
//     printf("Starting at node %d\n", v->index());
// #endif

    adjEntry a;
    forall_adj(a, v) {
      if (visited[a]) 
	continue;

      adjEntry b = a;
      int right = 1;
      while (right < 0 || !visited[b]) {

#if DEBUG2
	printf("At edge %d->%d oriented %d on side %d\n", b->theNode()->index(), b->twinNode()->index(), orient[b->theEdge()], right);
#endif

	if (right > 0)
	  visited[b] = 1;

	adjEntry c = b->twin();
	right = right * orient[b->theEdge()];
	
	if (right < 0)
	  visited[c] = 1;

// #if DEBUG
// 	printf("Adjacency of %d:", c->theNode()->index());
// 	adjEntry a;
// 	forall_adj(a, c->theNode()) 
// 	  printf(" %d(%d)", a->twinNode()->index(), a->cyclicSucc()->twinNode()->index());
// 	printf("\n");
// #endif

	if (right > 0)
	  b = c->cyclicSucc();
	else
	  b = c->cyclicPred();
      }

      faces++;
#if DEBUG
      printf("Face %d finished\n", faces);
#endif

    }
  }
  
#if DEBUG
  printf("Total %d faces\n", faces);
#endif

  return faces;
}

int genus_rec(Graph & G, EdgeArray<int> & orient, node v) //!!! Only cubic graphs!
{
#if DEBUG
  printf("In genus_rec at node %d\n", v->index());
#endif

  int g;
  if (!v->succ()) {
    int faces = compute_faces(G, orient);
    g = G.numberOfEdges() + 2 - G.numberOfNodes() - faces;

#if DEBUG
    printf("Embedding computed of genus %d with %d faces\n", g, faces);
#endif

// #if VERBOSE
//     if (g + orientable < mingenus) {
//       mingenus = g + orientable;
//       print_emb(G, orient, g);
//     }
// #endif

    return g;
  }

  g = genus_rec(G, orient, v->succ());

#if DEBUG
  printf("Coming back to node %d\n", index(v));
  print_emb(G);
#endif

  G.moveAdjAfter(v->lastAdj(), v->firstAdj());
  //  G.swapAdjEdges(v->firstAdj(), v->lastAdj());

#if DEBUG
  printf("Swap:\n");
  print_emb(G);
#endif

  int g2 = genus_rec(G, orient, v->succ());

  G.moveAdjAfter(v->lastAdj(), v->firstAdj());
  //  G.swapAdjEdges(v->firstAdj(), v->lastAdj());

  return (g2 < g)? g2 : g;  
}

int graph_genus(Graph & G)
{
#if DEBUG
  printf("Computing oriantable genus\n");
#endif
  EdgeArray<int> orient(G, 1);

// #if VERBOSE
//   mingenus = G.numberOfEdges();
// #endif

  int g = genus_rec(G, orient, G.firstNode());

  return g;
}

int genus_edge(Graph & G, EdgeArray<int> & span, EdgeArray<int> & orient, edge next)
{
  int g;
  if (!next) {
    
#if DEBUG
    printf("Signature:");
    edge f;
    forall_edges(f, G) {
      printf("%s", strsig[orient[f] < 0].c_str());
    }
    forall_edges(f, G) {
      printf(" %d-%d", f->source()->index(), f->target()->index());
  }
    printf("\n");
#endif
    
    g = genus_rec(G, orient, G.firstNode());
    
    g += 1;
    edge e;
    forall_edges(e, G) {
      if (orient[e] < 0) {
	g -= 1;
	break;
      }
    }

    return g;
  }

  g = genus_edge(G, span, orient, next->succ());
  if (!span[next]) {
    orient[next] = -1;
    int g2 = genus_edge(G, span, orient, next->succ());
    orient[next] = 1;
    g = min(g, g2);
  }

  return g;
}


int graph_genus_nonorientable(Graph & G)
{
#if DEBUG
  printf("Computing non-oriantable genus\n");
#endif

  EdgeArray<int> orient(G, 1);
  NodeArray<int> visited(G, 0);
  EdgeArray<int> span(G, 0);

  DFS(G, visited, span, G.firstNode());

// #if VERBOSE
//   mingenus = G.numberOfEdges();
// #endif

  return genus_edge(G, span, orient, G.firstEdge());
}



