#include "ogdfbase.h"

using namespace std;
using namespace ogdf;

extern BoyerMyrvold BM;

extern string str[2];
extern string stremb[2];
extern string strkur[2];
extern string strsubtype[16];
extern string strmin[2];
extern string strres[2];

int g, ng;

#if MIN_TEST
int smaller_genus(Graph & G)
{
  int res = genus_nonorientable(G);
  return ng - res;
}
#endif

int main()
{
  Graph G;

  while (read_graph(G)) {
#if VERBOSE
    print_graph(G);
#endif

    g = genus(G);

#if VERBOSE
    printf("The graph has orientable genus %d.\n", g);
#endif

    ng = genus_nonorientable(G);

#if VERBOSE
    printf("The graph has non-orientable genus %d.\n", ng);
#endif

#if !VERBOSE
    print_graph(G);
    printf("Graph of genus %d, non-orientable genus %d\n", g, ng);
#endif

#if MIN_TEST
    int min = test_edge_deletion(G, &smaller_genus);
    printf("Graph is %s\n", strmin[!min].c_str());
#endif
    
  }

  return 0;
}
