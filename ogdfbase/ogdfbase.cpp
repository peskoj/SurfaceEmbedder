#include "ogdfbase.h"

using namespace std;
using namespace ogdf;

BoyerMyrvold BM;

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

  node v;
  forall_nodes(v, G) {
    adjEntry a;
    printf("%d:[", v->index());
    if (v->degree())
      forall_adj(a, v) {
	printf("%d", a->twinNode()->index());
	if (orient[a->theEdge()] < 0)
	  printf("-");
	if (a->succ())
	  printf(", ");
      }
    printf("], ");
  }

  printf("\n");
}

void print_emb(Graph & G)
{
  EdgeArray<int> signature(G, 1);

  print_emb(G, signature, 0);
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
      if (c & (1 << j))
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

  SList <KuratowskiWrapper> list;
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

  all_paths_DFS(G, subg, subgi, source, target, res, visited, path);
}

int findpairs(Graph & G, vector<adjEntry> &halfedges, NodeArray<int> & cnode, vector< Tuple2<int,int> > & pairs)
{
#if DEBUG
  printf("In findpairs...\n");
#endif

  int c = 0;
  NodeArray<int> visited(G, 0);
  AdjEntryArray<int> ord(G, 0);
  
  for (unsigned int h = 0; h < halfedges.size(); h++) {
    adjEntry a = halfedges[h];
    assert(a);
    ord[a] = c++;
  }
    
  NodeArray<adjEntry> path(G);
  for (unsigned int h = 0; h < halfedges.size(); h++) {
    adjEntry a = halfedges[h];
    assert(a);
    node u = a->theNode();
    node v = a->twinNode();
    assert(u);

    if (visited[u])
      continue;

    visited[u] = 1;

    if (DEBUG)
      printf("Searching halfedge %d-%d\n", u->index(), v->index());

    if (cnode[v]) {
      pairs.push_back(Tuple2<int,int>(ord[a], ord[a->twin()]));
      if (DEBUG)
	printf("Chord added as a path: %d-%d\n", a->theNode()->index(), a->twinNode()->index());
      visited[v] = 1;
    }

    if (visited[v])
      continue;

//     adjEntry b;
//     forall_adj(b, v) {
//       node w = b->twinNode();

//       if (visited[w])
// 	continue;

//       if (cnode[w]) {
// 	pairs.pushBack(Tuple2<int,int>(ord[a], ord[b->twin()]));
// 	visited[w] = 1;
// 	visited[v] = 1;
// 	break;
//       }
//     }

    NodeArray<int> viscopy(visited);

    path.fill(0);
    node w = BFS(G, v, viscopy, cnode, path);
    if (w) {
      assert(path[w]);

      pairs.push_back(Tuple2<int, int>(ord[a], ord[path[w]]));
 
      if (DEBUG)
	printf("Path found: %d-", w->index());
      visited[w] = 1;
      while (path[w]) {
	w = path[w]->twinNode();
	visited[w] = 1;
	if (DEBUG)
	  printf("%d-", w->index());
      }

      if (DEBUG)
	printf("%d\n", u->index());
    }
  }
  
  return pairs.size();
}


void transform(Graph & G, KuratowskiWrapper & K, KuratowskiSubdivision & S)
{
  NodeArray<int> count(G, 0);
  EdgeArray<int> ecount(G, 0);
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


// void concat(List<edge> & cycle, KuratowskiSubdivision & S, const int * edges, int count)
// {
//   for (int i=0; i<count; i++) {
//     List<edge> tmp = List<edge>(S[edges[i]]);
//     cycle.conc(tmp);
//   }
// }

int sum_lengths(KuratowskiSubdivision & S, const int * edges, int count)
{
  int res = 0;
  for (int i=0; i<count; i++) 
    res += S[edges[i]].size();

  return res;
}

void duplicate(GraphCopySimple & H, node u, NodeArray<node> & ncopy, NodeArray<int> & cnode, List<node> & nodes)
{
  assert(u);

  if (cnode[u])
    return;

  cnode[u] = 1;
  nodes.pushBack(u);

  node nc = H.copy(u);
  node nn = H.newNode();
  ncopy[nc] = nn;
  ncopy[nn] = nc;
}

void duplicate_cycle(GraphCopySimple & H, NodeArray<node> & ncopy, NodeArray<int> & cnode, EdgeArray<int> & cedge, List<node> & nodes, vector<adjEntry> & halfedges, List<edge> & out, List<edge> & cycle)
{
  forall_listiterators(edge, it, cycle) {
    edge e = *it;
    cedge[e] = 1;
    
    duplicate(H, e->source(), ncopy, cnode, nodes);
    duplicate(H, e->target(), ncopy, cnode, nodes);
  }

  forall_listiterators(node, it, nodes) {
    node v = *it;

    adjEntry a;
    forall_adj(a, v) {
      edge e = a->theEdge();
      if (!cedge[e]) {
	halfedges.push_back(a);
	if (cnode[a->twinNode()] && a->index() < a->twin()->index()) {
#if DEBUG
	  printf("Chord between %d-%d\n", a->theNode()->index(), a->twinNode()->index());
#endif
	  continue;
	}
#if DEBUG
	printf("Adding edge %d-%d\n", a->theNode()->index(), a->twinNode()->index());
#endif
	out.pushBack(e);
      }
    }
  }

#if DEBUG
  printf("%d/%lu edges/halfedges on the boundary\n", out.size(), halfedges.size());
#endif

  forall_listiterators(edge, delit, out) {
    H.delEdge(H.copy(*delit));
  }
}

int right_orientation(node a, node b, EdgeArray<edge> & ncopy)
{
  adjEntry e, f;
  e = a->firstAdj();
  f = e->cyclicPred();

  adjEntry ec, fc;
  edge ee, fe;
  ee = ncopy[e->theEdge()];
  fe = ncopy[f->theEdge()];

  assert(ee && fe);
  ec = ee->adjTarget();
  fc = fe->adjTarget();

  if (DEBUG)
    printf("Right orientation %d->%d, %d->%d (should go to %d)\n", e->theEdge()->source()->index(), f->theEdge()->source()->index(), ec->theEdge()->source()->index(), 
	   ec->cyclicSucc()->theEdge()->source()->index(), fc->theEdge()->source()->index());

  return ec->cyclicSucc() == fc;
}


int checkpair(vector<adjEntry> &halfedges, NodeArray<int> & order, IntPair & a, IntPair & b, int code)
{
  int a1 = order[halfedges[a.x1()]->theNode()];
  int a2 = order[halfedges[a.x2()]->theNode()];
  int b1 = order[halfedges[b.x1()]->theNode()];
  int b2 = order[halfedges[b.x2()]->theNode()];

  int p = 0;
  if (a1 < b1) p++;
  if (a1 < b2) p++;
  if (a2 < b1) p++;
  if (a2 < b2) p++;

  return (p % 2 == 1) && BIT(code, a.x1()) == BIT(code, a.x2()) && BIT(code, b.x1()) == BIT(code, b.x2()) &&  BIT(code, a.x1()) == BIT(code, b.x1());
}

int checkpairs(vector<IntPair> & pairs, vector<adjEntry> &halfedges, NodeArray<int> & corder, int code)
{
  for (unsigned int i = 0; i<pairs.size(); i++)
    for (unsigned int j = 0; j<i; j++)
      if (checkpair(halfedges, corder, pairs[i], pairs[j], code)) {
#if DEBUG
	printf("Code %d skipped by pair %d-%d, %d-%d\n", code, halfedges[pairs[i].x1()]->theNode()->index(), halfedges[pairs[i].x2()]->theNode()->index(), 
	       halfedges[pairs[j].x1()]->theNode()->index(), halfedges[pairs[j].x2()]->theNode()->index());
#endif
	return 1;
      }
  return 0;
}


//------------------------ Non-contractible cycles ----------------------------------


void kuratowski_nodes(KuratowskiSubdivision & S, vector<node> & nodes, int isK33)
{
#if (DEBUG)
  printf("In kuratowski_nodes\n");
#endif

  if (isK33) {
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

int construct_cycles(Graph & G, KuratowskiWrapper & K, List<edge> * rcycles, int &cnum)
{
  KuratowskiSubdivision S;
  transform(G, K, S);

  int minval = MAXVALUE;//1 << (G.numberOfNodes()-3); //G.numberOfEdges() + 1;

  List<edge> cycles[MAXC];
  if (K.isK33()) {
    cnum = 2;
    int num = 3;

    for (int i = 0; i<6; i++) {
      int a = (i / 3)*2 + 1;
      int b = 3 - (i / 3)*2;
      int c = i % 3;
      
      for (int j = 0; j<3; j++) {
#if DEBUG2
	printf("Constructing cycle out of: %d, %d, %d, %d\n", (c*b + j*a)%9, (c*b + ((j+1)%3)*a)%9, (((c+1)%3)*b + j*a)%9, (((c+1)%3)*b + ((j+1)%3)*a)%9);
#endif
        CONSTRUCT(S, cycles[j], 4, (c*b + j*a)%9, (c*b + ((j+1)%3)*a)%9, (((c+1)%3)*b + j*a)%9, (((c+1)%3)*b + ((j+1)%3)*a)%9);
      }

//       CONSTRUCT(S, 0, 4, (c*b + 0*a)%9, (c*b + 1*a)%9, ((c+1)*b + 0*a)%9, ((c+1)*b + 1*a)%9)
//       CONSTRUCT(S, 1, 4, (c*b + 1*a)%9, (c*b + 2*a)%9, ((c+1)*b + 1*a)%9, ((c+1)*b + 2*a)%9);
//       CONSTRUCT(S, 2, 4, (c*b + 2*a)%9, (c*b + 0*a)%9, ((c+1)*b + 2*a)%9, ((c+1)*b + 0*a)%9);

      qsort(cycles, num, sizeof(List<edge>), cycle_cmp);


      int val = 0;
      for (int j=0; j<cnum; j++)
	val += value_function(cycles[j].size());

#if DEBUG
      printf("Cycles of value %d found:\n", val);
      for (int j=0; j<num; j++) {
	printf("Cycle of length %d found: ", cycles[j].size());
	print_edge_list(cycles[j]);
      }
#endif
      
      if (val < minval) {
	minval = val;
	for (int j=0; j<cnum; j++)
	  rcycles[j] = cycles[j];
      }
    }
  } else {
    cnum = 3;
    int num = 4;

    CONSTRUCT(S, cycles[0], 3, 0, 1, 4);
    CONSTRUCT(S, cycles[1], 3, 0, 2, 5);
    CONSTRUCT(S, cycles[2], 3, 1, 2, 7);
    CONSTRUCT(S, cycles[3], 3, 4, 5, 7);

    qsort(cycles, num, sizeof(List<edge>), cycle_cmp);

    int val = 0;
    for (int i=0; i<cnum; i++)
      val += value_function(cycles[i].size());
    
    if (val < minval) {
      minval = val;
      for (int i=0; i<cnum; i++)
	rcycles[i] = cycles[i];
    }
  }

  return minval;
}

int best_cycles(Graph & G, KuratowskiWrapper & K, List<edge> * cycles)
{
#if DEBUG
  printf("In best cycles:\n");
#endif

  //  List<edge> cycles[MAXC];
  int num = 0;

#if DEBUG
  int val = 
#endif
    construct_cycles(G, K, cycles, num);

#if DEBUG
  printf("Kuratowski subgraph of value %d and cycles %d\n", val, num);
#endif

  return num;
}


int get_k_graph(Graph & G, KuratowskiWrapper & K)
{
  SList<KuratowskiWrapper> output;

  int c = 1;
  int planar = test_planarity_bounded(G, c, output);

#if DEBUG
  printf("The graph is %s.\n", str[planar].c_str());
#endif

  if (planar)
    return 1;

  assert(output.size());
  K = output.front();
  assert(K.isK33());

#if DEBUG
  printf("%s, subtype %s, at node %d\n", strkur[K.isK33()].c_str(), strsubtype[K.subdivisionType].c_str(), K.V->index());

  printf("Edges:");
  print_edge_list(K.edgeList);
#endif
  
  return 0;
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

#if DEBUG
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


#if VERBOSE
static int mingenus;
static int orientable;
#endif

int genus_rec(Graph & G, EdgeArray<int> & orient, node v)
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

#if VERBOSE
    if (g + orientable < mingenus) {
      mingenus = g + orientable;
      print_emb(G, orient, g);
    }
#endif

    return g;
  }

  g = genus_rec(G, orient, v->succ());

#if DEBUG
  printf("Adjacency of %d:", v->index());
  adjEntry a;
  forall_adj(a, v) 
    printf(" %d(%d)", a->twinNode()->index(), a->cyclicSucc()->twinNode()->index());
#endif

  G.swapAdjEdges(v->firstAdj(), v->lastAdj());

#if DEBUG
  printf(" changed to");
  forall_adj(a, v) 
    printf(" %d(%d)", a->twinNode()->index(), a->cyclicSucc()->twinNode()->index());
  printf("\n");
#endif

  int g2 = genus_rec(G, orient, v->succ());

  G.swapAdjEdges(v->firstAdj(), v->lastAdj());

  return (g2 < g)? g2 : g;  
}

int graph_genus(Graph & G)
{
#if DEBUG
  printf("Computing oriantable genus\n");
#endif
  EdgeArray<int> orient(G, 1);

#if VERBOSE
  mingenus = G.numberOfEdges();
#endif

  int g = genus_rec(G, orient, G.firstNode());

  return g;
}

int genus_edge(Graph & G, EdgeArray<int> & span, EdgeArray<int> & orient, edge e, int nonor)
{
  int g;
  if (!e) {
#if VERBOSE
    orientable = !nonor;
#endif

#if DEBUG
  printf("Signature:");
  edge e;
  forall_edges(e, G) {
    printf("%s", strsig[orient[e] < 0].c_str());
  }
  forall_edges(e, G) {
    printf(" %d-%d", e->source()->index(), e->target()->index());
  }
  printf("\n");
#endif
    
    g = genus_rec(G, orient, G.firstNode());

    if (!nonor)
      g += 1;

    return g;
  }

  g = genus_edge(G, span, orient, e->succ(), nonor);
  if (!span[e]) {
    orient[e] = -1;
    int g2 = genus_edge(G, span, orient, e->succ(), 1);
    orient[e] = 1;
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

#if VERBOSE
  mingenus = G.numberOfEdges();
#endif

  return genus_edge(G, span, orient, G.firstEdge(), 0);
}



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


//----------------------- GraphCutter -----------------------------

void GraphCutter::duplicate(node u)
{
  assert(u);

  if (cnode[u])
    return;

  cnode[u] = 1;
  nodes.pushBack(u);

  node nc = copy(u);
  node nn = newNode();
  ncopy[nc] = nn;
  ncopy[nn] = nc;
}

//Development version of duplicate_cycle()
#if DISK_DEVEL
void GraphCutter::duplicate_cycle()
{
  forall_listiterators(edge, it, cycle) {
    edge e = *it;
    cedge[e] = 1;
    
    duplicate(e->source());
    duplicate(e->target());
  }

  if (disk)
    forall_listiterators(node, it, nodes) {
      if (node == disk->center) {
	disk_emb = 1;
	break;
      }
    }

  forall_listiterators(node, it, nodes) {
    node v = *it;

    if (disk_emb && indisk[v]) {
      adjEntry a;
      forall_adj(a, v) {
	edge e = a->theEdge();
	node u = a->twinNode();
	if (!cedge[e] && indisk[u]) {
	  
	}
      }      
    } else {
      adjEntry a;
      forall_adj(a, v) {
	edge e = a->theEdge();
	if (!cedge[e]) {
	  halfedges.push_back(a);
	  if (cnode[a->twinNode()] && a->index() < a->twin()->index()) {
#if DEBUG
	    printf("Chord between %d-%d\n", a->theNode()->index(), a->twinNode()->index());
#endif
	    continue;
	  }
#if DEBUG
	  printf("Adding edge %d-%d\n", a->theNode()->index(), a->twinNode()->index());
#endif
	  out.pushBack(e);
	}
      }
    }
  }

#if DEBUG
  printf("%d/%lu edges/halfedges on the boundary\n", out.size(), halfedges.size());
#endif

  //Add out edges to the copy
  forall_listiterators(edge, it, out) {
    edge e = *it;
    node u = copy(e->source());
    node v = copy(e->target());
    if (ncopy[u]) {
      eorient[e->adjSource()] = copy(e);
      forient[e->adjSource()] = newEdge(ncopy[u], v);
      
      if (ncopy[v]) {
	eorient[e->adjTarget()] = newEdge(ncopy[u], ncopy[v]);
	forient[e->adjTarget()] = newEdge(u, ncopy[v]);
      }
    } else {
      eorient[e->adjTarget()] = copy(e);
      forient[e->adjTarget()] = newEdge(u, ncopy[v]);
    }
#if DEBUG
    printf("Copies of");
    print_edge(e);
    printf(":");
    if (ncopy[u]) {
      print_edge(eorient[e->adjSource()]);
      print_edge(forient[e->adjSource()]);
    }
    if (ncopy[v]) {
      print_edge(eorient[e->adjTarget()]);
      print_edge(forient[e->adjTarget()]);
    }
    printf("\n");
#endif
    
  }
}
#else
//Normal version of duplicate_cycle()
void GraphCutter::duplicate_cycle()
{
  forall_listiterators(edge, it, cycle) {
    edge e = *it;
    cedge[e] = 1;
    
    duplicate(e->source());
    duplicate(e->target());
  }

  forall_listiterators(node, it, nodes) {
    node v = *it;

    adjEntry a;
    forall_adj(a, v) {
      edge e = a->theEdge();
      if (!cedge[e]) {
	halfedges.push_back(a);
	if (cnode[a->twinNode()] && a->index() < a->twin()->index()) {
#if DEBUG
	  printf("Chord between %d-%d\n", a->theNode()->index(), a->twinNode()->index());
#endif
	  continue;
	}
#if DEBUG
	printf("Adding edge %d-%d\n", a->theNode()->index(), a->twinNode()->index());
#endif
	out.pushBack(e);
      }
    }
  }

#if DEBUG
  printf("%d/%lu edges/halfedges on the boundary\n", out.size(), halfedges.size());
#endif

  //Add out edges to the copy
  forall_listiterators(edge, it, out) {
    edge e = *it;
    node u = copy(e->source());
    node v = copy(e->target());
    if (ncopy[u]) {
      eorient[e->adjSource()] = copy(e);
      forient[e->adjSource()] = newEdge(ncopy[u], v);
      
      if (ncopy[v]) {
	eorient[e->adjTarget()] = newEdge(ncopy[u], ncopy[v]);
	forient[e->adjTarget()] = newEdge(u, ncopy[v]);
      }
    } else {
      eorient[e->adjTarget()] = copy(e);
      forient[e->adjTarget()] = newEdge(u, ncopy[v]);
    }
#if DEBUG
    printf("Copies of");
    print_edge(e);
    printf(":");
    if (ncopy[u]) {
      print_edge(eorient[e->adjSource()]);
      print_edge(forient[e->adjSource()]);
    }
    if (ncopy[v]) {
      print_edge(eorient[e->adjTarget()]);
      print_edge(forient[e->adjTarget()]);
    }
    printf("\n");
#endif

  }

//   forall_listiterators(edge, delit, out) {
//     delEdge(copy(*delit));
//   }
}
#endif

void GraphCutter::init()
{
  duplicate_cycle();
}
 
#if DISK_DEVEL
int GraphCutter::set_fixed_disk(Disk * d)
{
  disk = d;
  indisk.init(G, 0);
}
#endif

void GraphCutter::create_one_disk()
{
  center = newNode();
  forall_listiterators(node, it, nodes) {
    node w = copy(*it);
    newEdge(w, center);
    newEdge(ncopy[w], center);
  }

  edge e = copy(cycle.front());
  assert(e);
  
  forall_listiterators(edge, it, cycle) {
    edge f = *it;
    if (copy(f) != e) {
      newEdge(ncopy[copy(f->source())], ncopy[copy(f->target())]);
    }
  }
  
  node u = e->target();
  node v = e->source();
  delEdge(e);
  
  newEdge(u, ncopy[v]);
  newEdge(v, ncopy[u]);
}

void GraphCutter::create_two_disks()
{
  // Create disk centers
  center = newNode();
  node centercopy = newNode();
  ncopy[center] = centercopy;
  ncopy[centercopy] = center;

  forall_listiterators(node, it, nodes) {
    node w = copy(*it);
    edge e = newEdge(w, center);
    edge f = newEdge(ncopy[w], centercopy);
    ecopy[e] = f;
    ecopy[f] = e;
  }

  // Copy the cut cycle
  forall_listiterators(edge, it, cycle) {
    edge f = *it;
    newEdge(ncopy[copy(f->source())], ncopy[copy(f->target())]);
  }
}

void GraphCutter::show_edge(edge e)
{
#if DEBUG
  print_edge(e);
#endif
  mod.pushBack(e);
  restoreEdge(e);
}

void GraphCutter::orient_edges(List<edge> & edges)
{
#if DEBUG
  printf("Orienting edges:");
#endif
  forall_listiterators(edge, it, edges) {
    edge e = *it;
    node u = e->source();
    
    adjEntry s = e->adjSource();
    adjEntry t = e->adjTarget();

#if DEBUG
    //    printf("Where cnode[u]=%d, orient[s]=%d, orient[t]=%d\n", cnode[u], orient[s], orient[t]);
    //     printf("Orienting edge u-v: %d-%d\n", e->source()->index(), e->target()->index());
    //     printf("eorient[s]=%d-%d\n", eorient[s]->source()->index(), forient[s]->target()->index());
#endif
    if (cnode[u]) {
      if (orient[t]) {
	if (orient[s]) 
	  show_edge(eorient[t]);
	else
	  show_edge(forient[t]);
      } else 
	if (orient[s]) 
	  show_edge(forient[s]);
	else
	  show_edge(eorient[s]);
    } else
      if (orient[t])
	show_edge(forient[t]);
      else
	show_edge(eorient[t]);
    
//     edge e = *it;
//     node u = copy(e->source());
//     node v = copy(e->target());
    
//     edge f = newEdge(u, v);
//     mod.pushBack(f);
    
//     node v = copy(e->target());
//     if (orient[e->adjSource()])
//       u = ncopy[u];
    
//     if (orient[e->adjTarget()])
//       v = ncopy[v];
    
//     edge f = newEdge(u, v);
//     mod.pushBack(f);
  }

#if DEBUG
  printf("\n");
#endif
}

void GraphCutter::hide_edges()
{
  for (unsigned int h = 0; h < halfedges.size(); h++) {
    adjEntry a = halfedges[h];
  
    hideEdge(eorient[a]);
    hideEdge(forient[a]);
  }
}

inline void GraphCutter::clear_edges()
{
//   forall_listiterators(edge, it, mod)
//     delEdge(*it);
//   mod.clear();

  forall_listiterators(edge, it, mod)
    hideEdge(*it);

  mod.clear();
}

int GraphCutter::test_all_codes(int (GraphCutter::*test)(), int (*validcode)(int))
{
  int low = 1;
  int high = (1<<(halfedges.size()-1))-1;

#if DEBUG
  printf("In test_all_codes(%d, %d)\n", low, high);
#endif

  hide_edges();

  for (int code = low; code < high; code++) {
    if (!(*validcode)(code))
      continue;

#if DEBUG
    printf("Embedding code '%d'\n", code);
#endif
    
    int i = 0;

    for (unsigned int h = 0; h < halfedges.size(); h++) {
      adjEntry a = halfedges[h];

      orient[a] = (code >> i) & 1;
      i++;
    }

    orient_edges(out);

#if DEBUG
    printf("Graph(%d,%d):\n", this->numberOfNodes(), this->numberOfEdges());
    print_graph(*this);
#endif

    int res = (this->*test)();

#if DEBUG
    printf("Graph is %s\n", stremb[res].c_str());
#endif

    if (res)
      return 1;

    clear_edges();
  }
  return 0;
}

int GraphCutter::test_jump(int (*test)(Graph & G))
{
#if DEBUG
  printf("Testing non-planar jumps\n");
#endif

  forall_listiterators(edge, it, out) {
    edge e = *it;
    
    orig.hideEdge(e);
    int planar = (*test)(orig);
    orig.restoreEdge(e);
    
#if DEBUG
    node u = e->source();
    node v = e->target();
    if (planar)
      printf("Edge between %d and %d is a jump\n", u->index(), v->index());
#endif

    if (planar) 
      return 1;
  }

  return 0;
}

int GraphCutter::test_tree_codes_recurse(List<edge>::iterator it, int (GraphCutter::*test)())
{

  if (!it.valid())
    return 1;

  edge e = *it;

#if DEBUG
  long long code = rcode;
  printf("Recursing on edge %s code %llu count %d\n", print_edge_str(e), code, rcount);
#endif

  adjEntry b[2] = {e->adjSource(), e->adjTarget()};

  int res = 0;
  for (int i = 0; i<2; i++) {
    adjEntry a = b[i];
    if (!cnode[a->theNode()]) 
      continue;

    restoreEdge(eorient[a]);
#if DEBUG
    print_graph(*this);
    rcount++;
    rcode = code * 10 + i*2;
#endif
    if ((this->*test)()) {
      res = test_tree_codes_recurse(it.succ(), test);
    }
    hideEdge(eorient[a]);
    if (res)
      return 1;

    restoreEdge(forient[a]);
#if DEBUG
    print_graph(*this);
    rcount++;
    rcode = code * 10 + i*2 + 1;
#endif
    if ((this->*test)()) {
      res = test_tree_codes_recurse(it.succ(), test);
    }
    hideEdge(forient[a]);
    if (res)
      return 1;
  }
  return 0;
}

int GraphCutter::test_tree_codes(int (GraphCutter::*test)())
{
#if DEBUG
  printf("GraphCutter::test_tree_codes\n");
#endif

  hide_edges();

  // Possible improvement: embed one of the edges arbitrary

#if DEBUG
  rcount = 0;
  rcode = 1;
#endif

  List<edge>::iterator it = out.begin();

#if DEVEL
#if DISK_DEVEL
  if (!disk_emb) {
#endif
    edge e = *it;
    adjEntry a;

    if (!cnode[(a = e->adjSource())->theNode()] || !cnode[(a = e->adjTarget())->theNode()]) {
      restoreEdge(eorient[a->twin()]);
      it = it.succ();
    }
#if DISK_DEVEL
  }
#endif
#endif

  int res = test_tree_codes_recurse(it, test);

  return res;
}

int GraphCutter::test_disks_torus()
{
#if DEBUG
  printf("In GraphCutter::test_disks_torus\n");
#endif

  int planar = test_planarity_with_embedding(*this);
  
#if DEBUG
  printf("The graph is %s\n", str[planar].c_str());
#endif

  if (planar) {
    // Test orientation of the embedding

#if DEBUG
    edge e;
    printf("Edges around orig center:");
    forall_adj_edges(e, center) {
      print_edge(e);
    }
    printf("\n");

    {
      printf("Edge around copy in orig:");
      forall_adj_edges(e, center) {
	print_edge(ecopy[e]);
      }
      printf("\n");
    }

    {
      printf("Edges around center copy:");
      forall_adj_edges(e, ncopy[center]) {
	print_edge(e);
      }
      printf("\n");
    }
#endif

    //!!! Possible bug - there can be an embedding with right orientation !!!
    if (right_orientation(center, ncopy[center], ecopy) || !k_connected(*this, center, ncopy[center], 3))
      return 1;
  }

  return 0;
}

int GraphCutter::planar()
{
  return test_planarity(*this);
}

int GraphCutter::test_disks_proj()
{
  return test_planarity(*this);
}


int GraphCutter::test_disks_klein1()
{
#if DEBUG
  printf("In GraphCutter::test_disks_klein1\n");
#endif

  return test_projective_cutter(*this);
}

int GraphCutter::test_disks_klein2()
{
#if DEBUG
  printf("In GraphCutter::test_disks_klein2\n");
#endif

  int planar = test_planarity_with_embedding(*this);

#if DEBUG
  printf("The graph is %s\n", str[planar].c_str());
#endif

  if (planar) {
    // Test orientation of the embedding

#if DEBUG
    edge e;
    printf("Edges around orig center:");
    forall_adj_edges(e, center) {
      print_edge(e);
    }
    printf("\n");
    
    {
      printf("Edge around copy in orig:");
      forall_adj_edges(e, center) {
	print_edge(ecopy[e]);
      }
      printf("\n");
    }
    
    {
      printf("Edges around center copy:");
      forall_adj_edges(e, ncopy[center]) {
	print_edge(e);
      }
      printf("\n");
    }
#endif

    //!!! Possible bug - there can be an embedding with right orientation !!!
    int orient = right_orientation(center, ncopy[center], ecopy);
#if DEBUG
    printf("Orientation matches: %d\n", orient);
#endif
    if (!orient)
      return 1;
    int con = k_connected(*this, center, ncopy[center], 3);
#if DEBUG
    printf("Graph is %s 3-connected\n", strneg[con].c_str());
#endif
    
    if (!con)
      return 1;
  }
  return 0;
}

int GraphCutter::cut_along_cycle_torus()
{
#if DEBUG
  printf("GraphCutter::Cutting along cycle on torus:");
  print_edge_list(cycle);
#endif
  duplicate_cycle();
  create_two_disks();

  // Test all possible directions of edges on the non-contractible cycle
  //  int res = test_all_codes(&GraphCutter::test_disks_torus);

  int res = 0;
  if (halfedges.size() >= TREEMIN) 
    res = test_tree_codes(&GraphCutter::test_disks_torus);
  else
    res = test_all_codes(&GraphCutter::test_disks_torus, &two_bits) || test_jump(&test_planarity);

  return res;
}

int GraphCutter::cut_along_cycle_proj()
{
#if DEBUG
  printf("GraphCutter::Cutting along cycle (projective):");
  print_edge_list(cycle);
#endif

#if DISK_DEVEL
  if (disk) {
    forall_listiterators(it, node, disk->cycle) {
      node u = *it;
      indisk[u] = 1;
      indisk[disk->center] = 1;
    }

    if (disk_contained())
      return 0;
  }
#endif

  duplicate_cycle();
  create_one_disk();

  // Test all possible directions of edges on the non-contractible cycle
  //  int res = test_all_codes(&GraphCutter::test_disks_proj);
  
  int res = 0;
  if (halfedges.size() >= TREEMIN) 
    res = test_tree_codes(&GraphCutter::test_disks_proj);
  else
    res = test_all_codes(&GraphCutter::test_disks_proj, &all_bits);

  return res;
}

int GraphCutter::cut_along_cycle_klein1()
{
#if DEBUG
  printf("GraphCutter::Cutting along cycle (klein bottle - along one cross cap):");
  print_edge_list(cycle);
#endif
  duplicate_cycle();
  create_one_disk();

  int res = 0;
  if (halfedges.size() >= TREEMIN) 
    res = test_tree_codes(&GraphCutter::test_disks_klein1);
  else
    res = test_all_codes(&GraphCutter::test_disks_klein1, &all_bits);

  return res;
}


int GraphCutter::cut_along_cycle_klein2()
{
#if DEBUG
  printf("GraphCutter::Cutting along cycle (klein bottle - along both crosscaps):");
  print_edge_list(cycle);
#endif
  duplicate_cycle();
  create_two_disks();

  int res = 0;
  if (halfedges.size() >= TREEMIN) 
    res = test_tree_codes(&GraphCutter::test_disks_klein2);
  else
    res = test_all_codes(&GraphCutter::test_disks_klein2, &all_bits);

  return res;
}


//------------------------- Torus ----------------------------------


void orient_edges(GraphCopySimple & H, NodeArray<node> & ncopy, AdjEntryArray<int> & orient, List<edge> & out, List<edge> & mod)
{
  forall_listiterators(edge, it, out) {
    edge e = *it;
    node u = H.copy(e->source());
    node v = H.copy(e->target());
    if (orient[e->adjSource()])
      u = ncopy[u];
    
    if (orient[e->adjTarget()])
      v = ncopy[v];
    
    edge f = H.newEdge(u, v);
    mod.pushBack(f);
  }
}

inline void del_edges(Graph & G, List<edge> & edges)
{
  forall_listiterators(edge, it, edges)
    G.delEdge(*it);
}


int test_all_codes(GraphCopySimple & H, NodeArray<node> & ncopy, EdgeArray<edge> & ecopy, node center, vector<adjEntry> & halfedges, AdjEntryArray<int> & orient, List<edge> & out, int low, int high, 
		   int test(Graph & G, NodeArray<node> & ncopy, EdgeArray<edge> & ecopy, node center))
{
  for (int code = low; code < high; code++) {
    if (!two_bits(code)) // More than one edge in both directions ???
      continue;

#if DEBUG
    printf("Embedding code '%d'\n", code);
#endif

    List<edge> mod;

    int i = 0;

    for (unsigned int h = 0; h < halfedges.size(); h++) {
      adjEntry a = halfedges[h];

      orient[a] = (code >> i) & 1;
      i++;
    }

    orient_edges(H, ncopy, orient, out, mod);

#if DEBUG
    print_graph(H);
#endif

    int res = test(H, ncopy, ecopy, center);

    if (res)
      return 1;

    del_edges(H, mod);
  }
  return 0;
}

int test_disks_torus(Graph & H, NodeArray<node> & ncopy, EdgeArray<edge> & ecopy, node center)
{
  int planar = test_planarity_with_embedding(H);
  
#if DEBUG
  printf("The graph is %s\n", str[planar].c_str());
#endif

  if (planar) {
    // Test orientation of the embedding
#if DEBUG
    edge e;
    printf("Edges around orig center:");
    forall_adj_edges(e, center) {
      print_edge(e);
    }
    printf("\n");

    {
      printf("Edge around copy in orig:");
      forall_adj_edges(e, center) {
	print_edge(ecopy[e]);
      }
      printf("\n");
    }

    {
      printf("Edges around center copy:");
      forall_adj_edges(e, ncopy[center]) {
	print_edge(e);
      }
      printf("\n");
    }
#endif

    //!!! Possible bug - there can be an embedding with right orientation !!!
    if (right_orientation(center, ncopy[center], ecopy) || !k_connected(H, center, ncopy[center], 3))
      return 1;
  }

  return 0;
}

int test_jump(Graph & G, List<edge> & del)
{
#if DEBUG
  printf("Testing non-planar jumps\n");
#endif

  forall_listiterators(edge, it, del) {
    edge e = *it;
    
    G.hideEdge(e);
    int planar = test_planarity(G);
    G.restoreEdge(e);
    
#if DEBUG
    node u = e->source();
    node v = e->target();
    if (planar)
      printf("Edge between %d and %d is a jump\n", u->index(), v->index());
#endif

    if (planar) 
      return 1;
  }

  return 0;
}

void create_two_disks(GraphCopySimple & H, List<node> & nodes, List<edge> & cycle, NodeArray<node> & ncopy, EdgeArray<edge> & ecopy, node & center)
{
  // Create disk centers
  center = H.newNode();
  node centercopy = H.newNode();
  ncopy[center] = centercopy;
  ncopy[centercopy] = center;

  forall_listiterators(node, it, nodes) {
    node w = H.copy(*it);
    edge e = H.newEdge(w, center);
    edge f = H.newEdge(ncopy[w], centercopy);
    ecopy[e] = f;
    ecopy[f] = e;
  }

  // Copy the cut cycle
  forall_listiterators(edge, it, cycle) {
    edge f = *it;
    H.newEdge(ncopy[H.copy(f->source())], ncopy[H.copy(f->target())]);
  }
}

int cut_along_cycle_torus(Graph & G, List<edge> & cycle)
{
#if DEBUG
  printf("Cutting along cycle:");
  print_edge_list(cycle);
#endif

  vector<adjEntry> halfedges;

  AdjEntryArray<int> orient(G, 0);
  EdgeArray<int> cedge(G, 0);
  NodeArray<int> cnode(G, 0);

  //  NodeArray< List<node> > neighbors(H, List<node>());
  List<node> nodes;

  GraphCopySimple H(G);
  EdgeArray<edge> ecopy(H, 0);
  NodeArray<node> ncopy(H, 0);

  List<edge> out;
  duplicate_cycle(H, ncopy, cnode, cedge, nodes, halfedges, out, cycle);

#if OPT_PATHS
  NodeArray<int> corder(G, 0);
  {
    int ord = 0;
    edge e = cycle.front();
    node v = e->source();

    node u = v;
    while (e->opposite(u) != v) {
      corder[u] = ord++;
      node w = e->opposite(u);
      edge f;
      forall_adj_edges(f, w) 
	if (cedge[f] && u != f->opposite(w)) {
	  e = f;
	  break;
	}
      u = w;
    }
    corder[u] = ord++;
  }

#if DEBUG
  printf("Cycle order:");
  forall_listiterators(node, it, nodes) {
    node u = *it;
    printf(" %d(%d)", u->index(), corder[u]);
  }
  printf("\n");
#endif

  // Construct disjoint paths
  vector< IntPair > pairs;
  findpairs(G, halfedges, cnode, pairs);

#if DEBUG
  printf("Disjoint paths found: %lu\n", pairs.size());
  for (unsigned int i = 0; i<pairs.size(); i++) {
    adjEntry a = halfedges[pairs[i].x1()];
    adjEntry b = halfedges[pairs[i].x2()];
    printf("Path: %d-%d\n", a->theNode()->index(), b->theNode()->index());
  }
#endif
#endif

  node center;
  create_two_disks(H, nodes, cycle, ncopy, ecopy, center);

  // Filter for codes, not in use right now
  // #if OPT_PATHS
  //     if (checkpairs(pairs, halfedges, corder, code)) {
  // #if !DEBUG
  //       continue;
  // #endif
  //     }
  // #endif

  // Test all possible directions of edges on the non-contractible cycle
  int res = test_all_codes(H, ncopy, ecopy, center, halfedges, orient, out, 0, (1<<(halfedges.size()-1))-1, &test_disks_torus);

  if (res)
    return 1;

  if (test_jump(G, out))
    return 1;


  return 0;
}


int test_torus(Graph & G)
{
#if DEBUG
  printf("In test_torus:\n");
#endif

  KuratowskiWrapper K;
  int planar = get_k_graph(G, K);

  if (planar)
    return 2;
  
  List<edge> cycles[MAXC];
  int num = best_cycles(G, K, cycles);

  int torus = 0;
  for (int i = 0; i<num; i++)
    if (cut_along_cycle_torus(G, cycles[i])) {
      torus = 1;
      break;
    }

  if (DEBUG)
    printf("The graph is %s on the torus.\n", stremb[torus].c_str());

  return torus;
}

int test_torus_multiple(Graph & G)
{
#if DEBUG
  printf("In test_torus_multiple:\n");
#endif

  KuratowskiWrapper K;
  int planar = best_k_graph(G, K);

  if (planar)
    return 2;

#if DEBUG
  printf("Chosen k-graph:");
  print_edge_list(K.edgeList);
#endif
  
  List<edge> cycles[MAXC];
  int num = best_cycles(G, K, cycles);

  int torus = 0;
  for (int i = 0; i<num; i++)
    if (cut_along_cycle_torus(G, cycles[i])) {
      torus = 1;
      break;
    }

#if DEBUG
  printf("The graph is %s on the torus.\n", stremb[torus].c_str());
#endif

  return torus;
}

int test_torus_cutter(Graph & G)
{
#if DEBUG
  printf("In test_torus_multiple:\n");
#endif

  KuratowskiWrapper K;
  int planar = best_k_graph(G, K);

  if (planar)
    return 2;

#if DEBUG
  printf("Chosen k-graph:");
  print_edge_list(K.edgeList);
#endif
  
  List<edge> cycles[MAXC];
  int num = best_cycles(G, K, cycles);

  int torus = 0;
  for (int i = 0; i<num; i++) {
    GraphCutter GC(G, cycles[i]);
    if (GC.cut_along_cycle_torus()) {
      torus = 1;
      break;
    }
  }

#if DEBUG
  printf("The graph is %s on the torus.\n", stremb[torus].c_str());
#endif

  return torus;
}

//--------------------- Projective plane ---------------------------------

int test_disks_proj(Graph & H, NodeArray<node> & ncopy, EdgeArray<edge> & ecopy, node center)
{
  return test_planarity(H);
}

void create_one_disk(GraphCopySimple & H, List<node> & nodes, List<edge> & cycle, NodeArray<node> & ncopy, EdgeArray<edge> & ecopy, node & center)
{
  center = H.newNode();
  forall_listiterators(node, it, nodes) {
    node w = H.copy(*it);
    H.newEdge(w, center);
    H.newEdge(ncopy[w], center);
  }

  {
    edge e = H.copy(cycle.front());
    assert(e);
 
    forall_listiterators(edge, it, cycle) {
      edge f = *it;
      if (H.copy(f) != e) {
	H.newEdge(ncopy[H.copy(f->source())], ncopy[H.copy(f->target())]);
      }
    }

    node u = e->target();
    node v = e->source();
    H.delEdge(e);

    H.newEdge(u, ncopy[v]);
    H.newEdge(v, ncopy[u]);
  }
}

int cut_along_cycle_proj(Graph & G, List<edge> & cycle)
{
#if DEBUG
  printf("Cutting along cycle (projective):");
  print_edge_list(cycle);
#endif

  vector<adjEntry> halfedges;

  AdjEntryArray<int> orient(G, 0);
  EdgeArray<int> cedge(G, 0);
  NodeArray<int> cnode(G, 0);

  //  NodeArray< List<node> > neighbors(H, List<node>());
  List<node> nodes;

  GraphCopySimple H(G);
  EdgeArray<edge> ecopy(H, 0);
  NodeArray<node> ncopy(H, 0);

  List<edge> out;

  duplicate_cycle(H, ncopy, cnode, cedge, nodes, halfedges, out, cycle);

  node center;
  create_one_disk(H, nodes, cycle, ncopy, ecopy, center);


  // Test all possible directions of edges on the non-contractible cycle
  int res = test_all_codes(H, ncopy, ecopy, center, halfedges, orient, out, 0, (1<<(halfedges.size()-1))-1, &test_disks_proj);
  
  return res;
}

int test_projective(Graph & G)
{
#if DEBUG
  printf("In test_projective\n");
#endif

  KuratowskiWrapper K;
  int planar = get_k_graph(G, K);

  if (planar)
    return 2;
  
  List<edge> cycles[MAXC];
  int num = best_cycles(G, K, cycles);

  int proj = 0;
  for (int i = 0; i<num; i++)
    if (cut_along_cycle_proj(G, cycles[i])) {
      proj = 1;
      break;
    }

  if (DEBUG)
    printf("The graph is %s on the projective plane.\n", stremb[proj].c_str());

  return proj;
}

int test_projective_multiple(Graph & G)
{
#if DEBUG
  printf("In test_projective_multiple:\n");
#endif

  KuratowskiWrapper K;
  int planar = best_k_graph(G, K);

  if (planar)
    return 2;

#if DEBUG
    printf("Chosen k-graph:");
    print_edge_list(K.edgeList);
#endif
  
  List<edge> cycles[MAXC];
  int num = best_cycles(G, K, cycles);

  int proj = 0;
  for (int i = 0; i<num; i++)
    if (cut_along_cycle_proj(G, cycles[i])) {
      proj = 1;
      break;
    }

  if (DEBUG)
    printf("The graph is %s in the projective plane.\n", stremb[proj].c_str());

  return proj;
}

int test_projective_cutter(Graph & G)
{
#if DEBUG
  printf("In test_projective_cutter:\n");
#endif

  KuratowskiWrapper K;
  int planar = best_k_graph(G, K);

  if (planar)
    return 2;

#if DEBUG
    printf("Chosen k-graph:");
    print_edge_list(K.edgeList);
#endif
  
  List<edge> cycles[MAXC];
  int num = best_cycles(G, K, cycles);

  int proj = 0;
  for (int i = 0; i<num; i++) {
    GraphCutter GC(G, cycles[i]);
    if (GC.cut_along_cycle_proj()) {
      proj = 1;
      break;
    }
  }

  if (DEBUG)
    printf("The graph is %s in the projective plane.\n", stremb[proj].c_str());

  return proj;
}

//----------------------- Klein bottle --------------------------------------

int cut_along_cycle_klein1(Graph & G, List<edge> & cycle)
{
#if DEBUG
  printf("Cutting along cycle (klein bottle - along one cross cap):");
  print_edge_list(cycle);
#endif

  vector<adjEntry> halfedges;

  AdjEntryArray<int> orient(G, 0);
  EdgeArray<int> cedge(G, 0);
  NodeArray<int> cnode(G, 0);

  //  NodeArray< List<node> > neighbors(H, List<node>());
  List<node> nodes;

  GraphCopySimple H(G);

  EdgeArray<edge> ecopy(H, 0);
  NodeArray<node> ncopy(H, 0);

  List<edge> out;

  duplicate_cycle(H, ncopy, cnode, cedge, nodes, halfedges, out, cycle);

  node center;
  create_one_disk(H, nodes, cycle, ncopy, ecopy, center);


  // Test all possible directions of edges on the non-contractible cycle
  int maxcode = (1<<(halfedges.size()-1))-1;
  for (int code = 0; code < maxcode; code++) {
    if (!two_bits(code)) // More than one edge in both directions ???
      continue;

#if DEBUG
    printf("Embedding code '%d'\n", code);
#endif
    

    List<edge> mod;

    int i = 0;

    for (unsigned int h = 0; h < halfedges.size(); h++) {
      adjEntry a = halfedges[h];

// #if DEBUG
//       printf("Code %d, w=%d, ncopy[w]=%d, neigh[w]=%d\n", code >> i, a->theNode()->index(), H.copy(a->theNode())->index(), a->twinNode()->index());
// #endif

      orient[a] = (code >> i) & 1;
      i++;
    }

    orient_edges(H, ncopy, orient, out, mod);

#if DEBUG
    print_graph(H);
#endif
    
    int proj = test_projective_multiple(H);

#if DEBUG
    printf("The graph is %s in the projective plane\n", stremb[proj>0].c_str());
#endif

    if (proj) 
      return 1;

    del_edges(H, mod);
  }

  return 0;
}


int cut_along_cycle_klein2(Graph & G, List<edge> & cycle)
{
#if DEBUG
  printf("Cutting along cycle (klein bottle - along both crosscaps):");
  print_edge_list(cycle);
#endif

  vector<adjEntry> halfedges;

  AdjEntryArray<int> orient(G, 0);
  EdgeArray<int> cedge(G, 0);
  NodeArray<int> cnode(G, 0);

  //  NodeArray< List<node> > neighbors(H, List<node>());
  List<node> nodes;

  GraphCopySimple H(G);

  EdgeArray<edge> ecopy(H, 0);
  NodeArray<node> ncopy(H, 0);

  List<edge> out;

  duplicate_cycle(H, ncopy, cnode, cedge, nodes, halfedges, out, cycle);

  node center;
  create_two_disks(H, nodes, cycle, ncopy, ecopy, center);


  // Test all possible directions of edges on the non-contractible cycle
  int maxcode = (1<<(halfedges.size()-1))-1;
  for (int code = 0; code < maxcode; code++) {
    if (!two_bits(code)) // More than one edge in both directions ???
      continue;

#if DEBUG
    printf("Embedding code '%d'\n", code);
#endif
    

    List<edge> mod;

    int i = 0;

    for (unsigned int h = 0; h < halfedges.size(); h++) {
      adjEntry a = halfedges[h];

// #if DEBUG
//       printf("Code %d, w=%d, ncopy[w]=%d, neigh[w]=%d\n", code >> i, a->theNode()->index(), H.copy(a->theNode())->index(), a->twinNode()->index());
// #endif

      orient[a] = (code >> i) & 1;
      i++;
    }

    forall_listiterators(edge, it, out) {
      edge e = *it;
      node u = H.copy(e->source());
      node v = H.copy(e->target());
      if (orient[e->adjSource()])
	u = ncopy[u];

      if (orient[e->adjTarget()])
	v = ncopy[v];

      edge f = H.newEdge(u, v);
      mod.pushBack(f);
    }
    
#if DEBUG
    print_graph(H);
#endif
    
    int planar = test_planarity_with_embedding(H);

    if (DEBUG)
      printf("The graph is %s\n", str[planar].c_str());

    if (planar) {
      // Test orientation of the embedding
      edge e;
      if (DEBUG) {
	printf("Edges around orig center:");
	forall_adj_edges(e, center) {
	  print_edge(e);
	}
	printf("\n");

	{
	printf("Edge around copy in orig:");
	forall_adj_edges(e, center) {
	  print_edge(ecopy[e]);
	}
	printf("\n");
	}

	{
	printf("Edges around center copy:");
	forall_adj_edges(e, ncopy[center]) {
	  print_edge(e);
	}
	printf("\n");
	}
      }

      //!!! Possible bug - there can be an embedding with right orientation !!!
      if (!right_orientation(center, ncopy[center], ecopy) || !k_connected(H, center, ncopy[center], 3))
	return 1;
    }

    del_edges(H, mod);
  }

  return 0;
}


int cut_along_cycle_klein3(Graph & G, List<edge> & cycle)
{
#if DEBUG
  printf("Cutting along cycle (klein bottle - around one crosscap):");
  print_edge_list(cycle);
#endif

  vector<adjEntry> halfedges;

  AdjEntryArray<int> orient(G, 0);
  EdgeArray<int> cedge(G, 0);
  NodeArray<int> cnode(G, 0);

  //  NodeArray< List<node> > neighbors(H, List<node>());
  List<node> nodes;

  GraphCopySimple H(G);

  EdgeArray<edge> ecopy(H, 0);
  NodeArray<node> ncopy(H, 0);

  List<edge> out;

  duplicate_cycle(H, ncopy, cnode, cedge, nodes, halfedges, out, cycle);

  node center;
  create_two_disks(H, nodes, cycle, ncopy, ecopy, center);


  // Test all possible directions of edges on the non-contractible cycle
  int maxcode = (1<<(halfedges.size()-1))-1;
  for (int code = 0; code < maxcode; code++) {
    if (!two_bits(code)) // More than one edge in both directions ???
      continue;

#if DEBUG
    printf("Embedding code '%d'\n", code);
#endif
    

    List<edge> mod;

    int i = 0;

    for (unsigned int h = 0; h < halfedges.size(); h++) {
      adjEntry a = halfedges[h];

// #if DEBUG
//       printf("Code %d, w=%d, ncopy[w]=%d, neigh[w]=%d\n", code >> i, a->theNode()->index(), H.copy(a->theNode())->index(), a->twinNode()->index());
// #endif

      orient[a] = (code >> i) & 1;
      i++;
    }

    forall_listiterators(edge, it, out) {
      edge e = *it;
      node u = H.copy(e->source());
      node v = H.copy(e->target());
      if (orient[e->adjSource()])
	u = ncopy[u];

      if (orient[e->adjTarget()])
	v = ncopy[v];

      edge f = H.newEdge(u, v);
      mod.pushBack(f);
    }
    
#if DEBUG
    print_graph(H);
#endif
    
    if (isConnected(H)) {
#if DEBUG
      printf("Cycle not separating\n");
#endif
    } else {
      Graph H1, H2;
      NodeArray<node> sn(H, 0);
      ConnectedSubgraph<int> CS;
      CS.call(H, H1, center, sn);

#if DEBUG
      print_graph(H1);
#endif
      
      int proj1 = test_projective_multiple(H1);

#if DEBUG
      printf("The graph 1 is %s in the projective plane\n", stremb[proj1>0].c_str());
#endif
      
#if DEBUG
      print_graph(H2);
#endif
      

      CS.call(H, H2, ncopy[center], sn);
      int proj2 = test_projective_multiple(H2);
      
#if DEBUG
      printf("The graph 2 is %s in the projective plane\n", stremb[proj2>0].c_str());
#endif

      if (proj1 && proj2) 
	return 1;
    }

    del_edges(H, mod);
  }

  return 0;
}

int test_klein_multiple(Graph & G)
{
#if DEBUG
  printf("In test_klein_multiple:\n");
#endif

  KuratowskiWrapper K;
  int planar = best_k_graph(G, K);

  if (planar)
    return 5;

#if DEBUG
    printf("Chosen k-graph:");
    print_edge_list(K.edgeList);
#endif
  
  List<edge> cycles[MAXC];
  int num = best_cycles(G, K, cycles);

  int proj = 0;
  for (int i = 0; i<num; i++) 
    if (cut_along_cycle_proj(G, cycles[i])) {
      proj = 4;
      goto END;
    }
      
  for (int i = 0; i<num; i++) 
    if (cut_along_cycle_klein1(G, cycles[i])) {
      proj = 1;
      goto END;
    }

  for (int i = 0; i<num; i++) 
    if (cut_along_cycle_klein2(G, cycles[i])) {
      proj = 2;
      goto END;
    }

  // This shouldn't happen: one of the cycles should go through a crosscap     
  //   for (int i = 0; i<num; i++) 
  //     if (cut_along_cycle_klein3(G, cycles[i])) {
  //       proj = 3;
  //       goto END;
  //     }

 END:
#if DEBUG
  printf("The graph is %s in the klein bottle (type %d).\n", stremb[proj>0].c_str(), proj);
#endif

  return proj;
}


int test_klein_cutter(Graph & G)
{
#if DEBUG
  printf("In test_klein_multiple:\n");
#endif

  KuratowskiWrapper K;
  int planar = best_k_graph(G, K);

  if (planar)
    return 5;

#if DEBUG
    printf("Chosen k-graph:");
    print_edge_list(K.edgeList);
#endif
  
  List<edge> cycles[MAXC];
  int num = best_cycles(G, K, cycles);

  int klein = 0;
  for (int i = 0; i<num; i++) {
    GraphCutter GC(G, cycles[i]);
    if (GC.cut_along_cycle_proj()) {
      klein = 4;
      goto END;
    }
  }

  for (int i = 0; i<num; i++) {
    GraphCutter GC(G, cycles[i]);
    if (GC.cut_along_cycle_klein1()) {
      klein = 1;
      goto END;
    }
  }

  for (int i = 0; i<num; i++) {
    GraphCutter GC(G, cycles[i]);
    if (GC.cut_along_cycle_klein2()) {
      klein = 2;
      goto END;
    }
  }

 END:
#if DEBUG
  printf("The graph is %s in the klein bottle (type %d).\n", stremb[klein>0].c_str(), klein);
#endif

  return klein;
}
