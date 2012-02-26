#include <stdio.h>
#include <assert.h>

#define MAXN 100
#define MAXC 4
#define MAXD 3
#define DEBUG 0

int n, m;

int edges[MAXN][MAXD];
int ec[MAXN];

int orient[MAXN][MAXD];
int head[MAXN][MAXD];
int flipped[MAXN];

int next_edge(int v, int e)
{
  return (e + 1 + flipped[v]) % 3;
}

int compute_faces()
{
  int v, e;

  int visited[MAXN][MAXD];
  for (v=0; v<n; v++)
    for (e=0; e<ec[v]; e++)
      visited[v][e] = 0;

  int faces = 0;
  for (v=0; v<n; v++)
    for (e=0; e<ec[v]; e++) {
      if (visited[v][e]) continue;

      int u = v;
      int f = e;
      int rep = 0;
      while (!visited[u][f]) {
	visited[u][f] = 1;
	if (u == v) rep++;

	int nu, nf;
	nu = edges[u][f];
	nf = next_edge(nu, head[u][f]);
	u = nu;
	f = nf;
      }
      if (rep == 3) return 0;
      faces++;
    }
  return faces;
}

int genus_rec(int v)
{
  int g;
  if (v == n) {
    int faces = compute_faces();
    g = (m + 2 - n - faces) / 2;
    if (DEBUG)
      printf("Embedding computed of genus %d with %d faces\n", g, faces);
    return g;
  }

  flipped[v] = 0;
  g = genus_rec(v+1);
  if (g <= 1) return g;

  flipped[v] = 1;
  int g2 = genus_rec(v+1);
  return (g2 < g)? g2 : g;  
}

int genus()
{
  return genus_rec(1);
}

void print_graph()
{
  printf("%d %d", n, m);
  int i, j;
  for (i=0; i<n; i++)
    for (j=0; j<ec[i]; j++)
      if (i < edges[i][j])
	printf(" %d %d", i, edges[i][j]);
  printf("\n");
}

int main()
{
  while (!feof(stdin)) {
    scanf("%d %d", &n, &m);
    if (feof(stdin)) return 0;

    assert(n > 0 && m > 0);

    int i;
    for (i=0; i<n; i++) 
      ec[i] = 0;

    for (i=0; i<m; i++) {
      int x, y;
      scanf("%d %d", &x, &y);
      head[x][ec[x]] = ec[y];
      head[y][ec[y]] = ec[x];
      edges[x][ec[x]++] = y;
      edges[y][ec[y]++] = x;
    }

    int g = genus();
    printf("Genus %d: ", g);
    print_graph();
  }

  
  

  return 0;
}
