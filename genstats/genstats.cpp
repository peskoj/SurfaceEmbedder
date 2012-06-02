#include "ogdfbase.h"
#include "embedder.h"


using namespace std;
using namespace ogdf;

#if STATISTICS
extern int planarcount;
#endif

int tcont = 0; //test contraction
int tdel = 1; //test deletion
int oPrint = 0; //print the graph
int genus = 2; //euler genus (2x orientable genus)
int orientable = 1; //+1 orientable, -1 non-orientable, 0 Euler genus
pair<int,int> terminals;
vector< vector<int> > stats;

int test_embeddable(Graph & G)
{
  Embedder E(G);
  int res = E.embed(genus, orientable);
#if VERBOSE
  if (res) {
    E.set_embedding();
    print_emb(E, genus);
    int g = E.compute_genus();
    printf("The graph is embeddable into the surface of genus %d orientable %d (required %d,%d)\n", g, E.orientable_emb(), genus, orientable);
    if (g >= genus)
      assert(g == genus);
      if (orientable)
	assert(E.orientable_emb() == orientable);
    }
#endif
  return res;
}

int min_genus(Graph & G)
{
  Embedder E(G);
  return E.min_genus(orientable, genus);
}    

void init_stats() {
  stats.resize(genus+1);
  for (uint i=0; i<stats.size(); i++) 
    stats[i].resize(genus+1);
}

void clear_stats() {
  for (uint i=0; i<stats.size(); i++) 
    for (uint j=0; j<stats[i].size(); j++)
      stats[i][j] = 0;
}

void print_stats(int g, int gp) {
#if DEBUG
  printf("Printing stats\n");
#endif

  printf("Stats: g(G) = %d, g^+(G) = %d, ", g, gp);
  for (uint i=0; i<stats.size(); i++) 
    for (uint j=0; j<stats[i].size(); j++)
      if (stats[i][j])
	printf("[%d, %d]: %d, ", i, j, stats[i][j]);
  printf("\n");
}

void get_stats(Embedder & E, node x, node y) {
#if DEBUG
  printf("Getting all stats, terminals %d,%d\n", index(x), index(y));
#endif

  edge f;
  forall_edges(f, E) {
    if (tcont) {
#if DEBUG
      printf("Contraction of %s\n", print_edge_str(f));
#endif
      GraphCopySimple G(E);
      edge e = G.copy(f);
      G.contract(e);
      makeSimpleUndirected(G);

      int g = min_genus(G);

      node xc = G.copy(x);
      node yc = G.copy(y);
      G.newEdge(xc, yc);
      makeSimpleUndirected(G);

      int gp = min_genus(G);

#if VERBOSE
      printf("Contraction of %s: g(G / e) = %d, g^+(G / e) = %d\n", print_edge_str(f), g, gp);
#endif
      stats[g][gp]++;
    }

    if (tdel) {
#if DEBUG
      printf("Deletion of %s\n", print_edge_str(f));
#endif
      GraphCopySimple G(E);
      edge e = G.copy(f);
      G.delEdge(e);

      int g = min_genus(G);

      node xc = G.copy(x);
      node yc = G.copy(y);
      G.newEdge(xc, yc);

      int gp = min_genus(G);

#if VERBOSE
      printf("Deletion of %s: g(G - e) = %d, g^+(G - e) = %d\n", print_edge_str(f), g, gp);
#endif
      stats[g][gp]++;
    }
  }
}


int main(int argc, char ** argv)
{

  int c;
  while ((c = getopt (argc, argv, "pcdCDg:o:e:")) != -1)
    switch (c)
      {
      case 'e':
	int x, y;
	sscanf(optarg, "%d %d", &x, &y);
	
	terminals = pair<int,int>(x,y);
	break;
      case 'c':
	tcont = 1;
	break;
      case 'd':
	tdel = 1;
	break;
      case 'C':
	tcont = 0;
	break;
      case 'D':
	tdel = 0;
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
      case 'p':
	oPrint = 1;
      case '?':
	printf("Unknown argument '-%c'\n", optopt);
	return 1;
      default:
	abort();
      }


  fprintf(stderr, "Testing for minimality: genus %d, orientable: %d, testing deletion: %d, testing contraction: %d, terminals: %d %d\n", 
	  genus, orientable, tdel, tcont, terminals.first, terminals.second);

  if (terminals.first == terminals.second) {
    fprintf(stderr, "Terminals not specified correctly: Not distinct\n");
    return 1;
  }

  init_stats();

  Graph G;

  while (read_graph(G)) {
#if VERBOSE
    print_graph(G);
#endif
    clear_stats();

    Embedder E(G);
    int g = E.min_genus(orientable, genus);
    
#if VERBOSE
    printf("The graph has genus %d.\n", g);
#endif

    vector<node> nodes;
    node v;
    forall_nodes(v, E)
      nodes.push_back(v);
    node x = nodes[terminals.first];
    node y = nodes[terminals.second];

    edge xy = E.newEdge(x, y);
    int gp = E.min_genus(orientable, genus);
    E.delEdge(xy);

#if VERBOSE
    printf("The graph has genus+ %d.\n", g);
#endif

    get_stats(E, x, y);    

    if (oPrint)
      print_graph(G);
    print_stats(g, gp);

#if DEBUG
    printf("Graph done\n");
#endif
  }

#if STATISTICS
  fprintf(stderr, "Planarity tested %d-times\n", planarcount);
#endif
  
  return 0;
}
