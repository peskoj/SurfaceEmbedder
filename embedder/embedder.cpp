#include "ogdfbase.h"
#include "embedder.h"


using namespace std;
using namespace ogdf;


//----------------------- Disk ---------------------------------

template<> 
void print_item(Disk* & item)
{
  printf(" %d", item->id());
}


Disk::Disk(Slice & S, int symmetric = 1)
{
#if DEBUG
  printf("Disk::Empty constructor, symmetric: %d\n", symmetric);
#endif

  mSlice = &S;
  mId = 0;
  mGroup = 0;
  mPair = 0;
  mCenter = 0;
  mCenterVisible = 0;
  mSymmetry = symmetric;
  //  mOutEdges = 0;

  mNodePairs.init(S, 0);
  mSide.init(S, 0);
  mNext.init(S, 0);
  mPrev.init(S, 0);
}

Disk::Disk(Slice & S, const Disk & D, NodeArray<node> & vCopy, EdgeArray<edge> & eCopy)
{
#if DEBUG
  printf("Disk::Copy constructor\n");
#endif

  mSlice = &S;
  mId = D.mId;
  mGroup = D.mGroup;
  mSymmetry = D.mSymmetry;
  mPair = 0;

  mNodePairs.init(S, 0);
  mSide.init(S, 0);
  mNext.init(S, 0);
  mPrev.init(S, 0);

  forall_listiterators(adjEntry, it, D.mOrient) {
    adjEntry b = *it;
    edge e = b->theEdge();
    adjEntry a;
    if (e->adjSource() == b)
      a = eCopy[e]->adjSource();
    else
      a = eCopy[e]->adjTarget();

    mOrient.pushBack(a);

    node u = a->theNode();
    node v = b->theNode();

    assert(D.mNodePairs[v]);
    mNodePairs[u] = vCopy[D.mNodePairs[v]];
    mSide[u] = D.mSide[v];
  }

  mCenter = 0;

  recompute();
}

void Disk::set_symmetry(int s)
{
  mSymmetry = s;
  if (mPair)
    mPair->mSymmetry = s;
}

int Disk::id()
{
  if (!this)
    return 0;
  
  return mId;
}

int Disk::has_pair()
{
  return !!mPair;
}

Disk * Disk::pair()
{
  assert(mPair);
  
  return mPair;
}


int Disk::check_symmetry(node target)
{
  if (target->index() < mNodePairs[target]->index())
    return 0;

  forall_listiterators(adjEntry, it, mOrient) {
    node u = (*it)->theNode();
    
    if (u->degree() > 2)
      return 0;
  }

  if (has_pair())
    forall_listiterators(adjEntry, it, pair()->mOrient) {
      node u = (*it)->theNode();
      
      if (u->degree() > 2)
	return 0;
    }

  return 1;
}

void Disk::recompute()
{
  forall_listiterators(adjEntry, it, mOrient) {
    adjEntry a = *it;
    node u = a->theNode();
    mNext[u] = a;
    mPrev[a->twinNode()] = u;
  }
}
 
void Disk::exchange_node(node oldNode, node newNode)
{
  //moveAdjEntry(arc(oldNode), newNode);
  //moveAdjEntry(arc(prev(oldNode))->twin(), newNode);
  mNext[newNode] = mNext[oldNode];
  mNext[oldNode] = NULL;

  recompute();
}

node Disk::next(node u)
{
  assert(arc(u));
  return arc(u)->twinNode();
}

adjEntry Disk::arc(node u)
{
  return mNext[u];
}
 
adjEntry Disk::prev_arc(node u)
{
  return arc(prev(u))->twin();
}

adjEntry Disk::center_arc(node u)
{
  assert(mCenterVisible);

  adjEntry b = arc(u);
  while (b->twinNode() != center()) {
    b = b->cyclicSucc();
  }

  return b;
}
 
node Disk::prev(node u)
{
  return mPrev[u];
}

int Disk::adjacent_nodes(node u, node v)
{
  return prev(u) == v || prev(v) == u;
}

void Disk::boundary(List<edge> & boundary, node u, node v) //!!!?
{

}

int Disk::alternating(List<node> & a, node * b) //!!!?
{
  return 0;
}

int Disk::pairing_sign()
{
  if (!pair())
    return 1;

  adjEntry a = mOrient.front();
  node u = a->theNode();
  node v = mNodePairs[u];
  node nu = next(u);
  node nv = pair()->next(v);

  if (mNodePairs[nu] == nv)
    return 1;
  else
    return -1;
}

node Disk::center()
{
  return mCenter;
}

//----------------------- Slice --------------------------------

Slice::Slice(): mOrig(0)
{
}


Slice::Slice(const Slice & slice)
{
  NodeArray<node> vCopy;
  EdgeArray<edge> eCopy;
  
  Slice(slice, vCopy, eCopy);
}

Slice::Slice(const Slice & slice, NodeArray<node> & vCopy, EdgeArray<edge> & eCopy) :
  mOrig(slice.mOrig), 
  mDiskNum(slice.mDiskNum), mSingle(slice.mSingle), mDouble(slice.mDouble),
  mGenus(slice.mGenus), mOrientable(slice.mOrientable), mCentersVisible(0)
{
#if DEBUG
  printf("Slice::Copy constructor\n");
#endif

  Graph::construct(slice,vCopy,eCopy);
  remove_isolated(*this);

#if DEBUG
  printf("Graph constructed\n");
  assert(consistencyCheck());
#endif

  mNodeOrig.init(*this, 0);

  node u;
  forall_nodes(u, slice) {
    mNodeOrig[vCopy[u]] = slice.mNodeOrig[u];
  }

  mEdgeOrig.init(*this, 0);

  edge e;
  forall_edges(e, slice) {
    mEdgeOrig[eCopy[e]] = slice.mEdgeOrig[e];
  }

  mEdgeCopies.init(*mOrig);

#if DEBUG
  printf("Copying possibilities\n");
#endif

  mPoss.init(*this);
  forall_nodes(u, slice) {
    forall_listiterators(node, it, slice.mPoss[u])
      mPoss[vCopy[u]].pushBack(vCopy[*it]);
  }

#if DEBUG
  printf("Setting information about edge copies\n");
#endif

  forall_edges(e, *mOrig) {
    //    mEmbedded[e] = slice.mEmbedded[e];
    
    forall_listiterators(edge, it, slice.mEdgeCopies[e]) {
      edge f = eCopy[*it];

#if DEBUG
      if (!f)
	printf("Edge %s not copied\n", print_edge_str(*it));
#endif

      if (!f)
	continue;

#if DEBUG
      printf("Edge %s copied with orig %s (disks %d,%d)\n", print_edge_string(f).c_str(), print_edge_string(e).c_str(), slice.mDiskInc[(*it)->adjSource()]->id(), slice.mDiskInc[(*it)->adjTarget()]->id());
#endif

      mEdgeCopies[e].pushBack(f);
//       if (!mEmbedded[e]) {
// 	mOut[f->source()].pushBack(f);
// 	mOut[f->target()].pushBack(f);
//       }
    }
  }

#if DEBUG
  printf("Copying disk information\n");
#endif

  mDisks.init(mDiskNum);
  for (int i = 0; i<mDiskNum; i++) {
    Disk * D = new Disk(*this, *(slice.mDisks[i]), vCopy, eCopy);
    mDisks[i] = D;
  }
  
  for (int i=0; i<mDiskNum; i++) {
    Disk * D = slice.mDisks[i];
    if (D->mPair) {
      mDisks[i]->mPair = mDisks[D->mPair->mId-1]; //mId -1? !!!
    } else
      mDisks[i]->mPair = 0;
  }

  compute_disk_inc();
  compute_copies();
}

Slice::Slice(Graph & G)
{
  init(G);
}

void Slice::init(Graph & G)
{
  mOrig = &G;
  mNodeOrig.init(*this, 0);
  mEdgeOrig.init(*this, 0);
  mDiskNum = 0;
  mSingle = 0;
  mDouble = 0;
  mGenus = 0; 
  mOrientable = 0;
  mUnselected = 0;
  mCentersVisible = 0;
  //  mUnselectedHidden = 1;
  
  NodeArray<node> vCopy;
  EdgeArray<edge> eCopy;

  Graph::construct(G, vCopy, eCopy);

  mNodeOrig.init(*this, 0);
  
  node u;
  forall_nodes(u, G) {
    mNodeOrig[vCopy[u]] = u;
  }

  mEdgeOrig.init(*this, 0);
  mEdgeCopies.init(G);
  //  mEmbedded.init(G, 1);

  edge e;
  forall_edges(e, G) {
    mEdgeOrig[eCopy[e]] = e;
    mEdgeCopies[e].pushBack(eCopy[e]);
  }

  mDiskInc.init(*this, 0);
  mDiskNodes.init(*this);
  mPoss.init(*this);
  //  mOut.init(*this);

  //  mCycleData.mDisks.init(MAXGENUS*2+1);
}

void Slice::read_slice(Graph & G) 
{
  mOrig = &G;

  node u;
  Array<node> gnodes(0, G.maxNodeIndex()+1, 0);
  forall_nodes(u, G) 
    gnodes[u->index()] = u;

  read_graph(*this);
  mNodeOrig.init(*this, 0);
  mEdgeOrig.init(*this, 0);

  Array<node> snodes(0, maxNodeIndex()+1, 0);
  forall_nodes(u, *this) 
    gnodes[u->index()] = u;

  forall_nodes(u, *this) { //u not important, allow for any order
    int x, y;
    scanf("%d->%d", &x, &y);
    assert(snodes[x]);
    assert(gnodes[y]);
    mNodeOrig[snodes[x]] = gnodes[y];
  }

  forall_nodes(u, *this) { //u not important, allow for any order
    int x, y, c, i;
    scanf("%d:%d", &x, &c);
    assert(snodes[x]);

    for (i=0; i<c; i++) {
      scanf("%d", &y);
      mPoss[snodes[x]].pushBack(snodes[y]);
    }
  }

  edge e;
  forall_edges(e, *this) { 
    mEdgeOrig[e] = G.searchEdge(mNodeOrig[e->source()->index()], mNodeOrig[e->target()->index()]);
  }

#if DEBUG
  printf("Reading disk information\n");
#endif

  scanf("%d", &mDiskNum);
  mDisks.init(mDiskNum);
  for (int i = 0; i<mDiskNum; i++) {
    Disk * D = new Disk(*this);
    mDisks[i] = D;
  }

  for (int i = 0; i<mDiskNum; i++) {
    read_disk(mDisks[i], snodes);
  }

  compute_disk_inc();
  compute_copies();
}

void Slice::print_slice()
{
  NodeArray<int> ind;
  print_graph(*this, ind);

  node u;
  forall_nodes(u, *this) {
    if (!mNodeOrig[u]) {
#if DEBUG
      printf("(%d) ", ind[u]);
#endif
      continue;
    }
      
    printf("%d->%d ", ind[u], ind[mNodeOrig[u]]);
  }
  printf("\n");

  forall_nodes(u, *this) {
    if (mPoss[u].size()) {
      printf("%d:%d(", ind[u], mPoss[u].size());
      forall_listiterators(node, it, mPoss[u]) {
	printf("%d ", ind[*it]);
      }
      printf(") ");
    }
  }
  printf("\n");

  printf("%d\n", mDiskNum);
  for (int i = 0; i<mDiskNum; i++) 
    print_disk(mDisks[i], ind);

  printf("\n");
}

void Slice::read_disk(Disk * D, Array<node> & nodes)
{
  char c;
  scanf("Disk %c", &c);
  if (c == 'P') {
    int pair;
    scanf("%d", &pair);
    D->mPair = mDisks[pair];
  }

  int count;
  scanf("%d", &count);
  
  for (int i=0; i<count; i++) {
    int x, y;
    scanf("%d-%d", &x, &y);
    assert(nodes[x]);
    assert(nodes[y]);
    adjEntry a = find_adj(nodes[x], nodes[y]);
    assert(a);
    D->mOrient.pushBack(a);
  }
}

void Slice::print_disk(Disk * D, NodeArray<int> & ind)
{
  if (D->has_pair())
    printf("Disk P %d", D->pair()->mId);
  else
    printf("Disk S");

  printf(" %d", D->mOrient.size());
  
  forall_listiterators(adjEntry, it, D->mOrient) {
    adjEntry a = *it;
    printf(" %d-%d", ind[a->theNode()], ind[a->twinNode()]);
  }
  printf("\n");
}


Slice::~Slice()
{
  forall_disks(it, *this)
    delete *it;
}

void Slice::compute_disk_inc()
{
#if DEBUG
  printf("In Slice::compute_disk_inc\n");
#endif
  mDiskInc.init(*this, 0);
  mDiskNodes.init(*this);

  forall_disks(it, *this) {
    Disk * D = *it;
    forall_listiterators(adjEntry, ait, D->mOrient) {
      adjEntry a = *ait;
      mDiskInc[a] = D;
      mDiskNodes[a->theNode()].pushBack(D);
    }
  }
}

// void Slice::flood_fill_disk_pairs(node v, int part)
// {
//   mCopiesPart[v] = part;
  
//   forall_listiterators(*Disk, dit, mDiskNodes[v]) {
//     Disk * D = *dit;
//     node w = D->mNodePairs[v];
//     assert(w);
    
//     if (!mCopiesPart[w])
//       flood_fill_disk_pairs(w, part);
//   }
// }

void Slice::compute_copies()
{
#if DEBUG
  printf("In compute_copies (%d vertices, %d edges)\n", numberOfNodes(), numberOfEdges());
#endif

  mNodeCopies.init(*mOrig);
  node u;
  forall_nodes(u, *this) {
    assert(mNodeOrig[u]);
    mNodeCopies[mNodeOrig[u]].pushBack(u);
  }

  mEdgeCopies.init(*mOrig);
  edge e;
  forall_edges(e, *this) {
    if (mEdgeOrig[e])
      mEdgeCopies[mEdgeOrig[e]].pushBack(e);
  }

#if DEBUG
  printf("Computing partitions of copies\n");
#endif


#if DEBUG
  printf("Computing disk attachments\n");
#endif
  
  //  mDiskCuts.clear();

  forall_disks(it, *this) {
    Disk * D = *it;
    forall_listiterators(adjEntry, ait, D->mOrient) {
      adjEntry a = *ait;
      node u = a->theNode();
      if (mPoss[u].empty()) 
	continue;

      ///!!!???
    }
  }

#if DEBUG
  printf("Computing node attachments\n");
#endif
  
  ///to be continued !!!
}

int Slice::compute_unselected()
{
#if DEBUG
  printf("Computing unselected attachments:");
#endif

  mUnselected = 0;

  node u;
  forall_nodes(u, *this) {
    if (!mPoss[u].empty()) {
#if DEBUG 
    printf("%d:(", u->index());
    print_node_list(mPoss[u], 0);
    printf(") ");
#endif
      mUnselected++;
    }
  }

#if DEBUG
  printf("%d total\n", mUnselected);
#endif

  return mUnselected;
}

Disk * Slice::disk(edge e)
{
  if (mDiskInc[e->adjSource()])
    return mDiskInc[e->adjSource()];

  if (mDiskInc[e->adjTarget()])
    return mDiskInc[e->adjTarget()];

  return 0;
}

int Slice::disk_group(edge e)
{
  Disk * D = disk(e);

  if (D)
    return D->mGroup;

  return 0;
}

int Slice::disk_edge(edge e)
{
  return mDiskInc[e->adjSource()] || mDiskInc[e->adjTarget()];
}

void Slice::change_signature(Disk * D, EdgeArray<int> & signature, NodeArray<int> & flipped) //!!!
{
}

void Slice::pair_disk(Disk * D, int oA, int oB, AdjEntryArray<adjEntry> & pairs) //!!!
{
}

void Slice::sign_disk(Disk * D, AdjEntryArray<int> & signs) //!!!
{

}

void Slice::pair_adj(adjEntry a, adjEntry b, int oA, int oB, AdjEntryArray<adjEntry> & pairs) //!!!
{

}



//--------------------------- node & edge modifications ------------------------------------

edge Slice::newEdgeCopy(edge e, node u, node v)
{
  edge f = newEdge(u, v);
  mEdgeOrig[f] = mEdgeOrig[e];

#if DEBUG
  printf("New edge %s origin %s\n", print_edge_string(f).c_str(), print_edge_string(e).c_str());
#endif
  assert(mEdgeOrig[f]);
  mEdgeCopies[mEdgeOrig[f]].pushBack(f);

  return f;
}

node Slice::newNodeCopy(node u)
{
  node v = newNode();
  mNodeOrig[v] = mNodeOrig[u];

  assert(mNodeOrig[v]);
  mNodeCopies[mNodeOrig[v]].pushBack(v);
  return v;
}

node Slice::newNodeCopy(node u, int newid)
{
  node v = newNode(newid);
  mNodeOrig[v] = mNodeOrig[u];

  assert(mNodeOrig[v]);
  mNodeCopies[mNodeOrig[v]].pushBack(v);
  return v;
}

edge Slice::split(edge e)
{
  edge eNew  = Graph::split(e);
  edge eOrig = mEdgeOrig[e];
  
  mEdgeOrig[eNew] = eOrig;

  return eNew;
}

edge Slice::split_adj(adjEntry a)
{
  split(a->theEdge());

  return a->theEdge();
}

node Slice::move_edges(node u, List<adjEntry> & inc, node v) {
  forall_listiterators(adjEntry, it, inc) {
    adjEntry a = *it;
    
    moveAdjEntry(a, v);
  }
  return v;
}

node Slice::split_node(node u, List<adjEntry> & inc, int newid)
{
  node v = newNodeCopy(u, newid);

  forall_listiterators(adjEntry, it, inc) {
    adjEntry a = *it;
    
    moveAdjEntry(a, v);
  }
  return v;
}

node Slice::split_node(node u, List<adjEntry> & inc)
{
  node v = newNodeCopy(u);

  forall_listiterators(adjEntry, it, inc) {
    adjEntry a = *it;
    
    moveAdjEntry(a, v);
  }
  return v;
}

void Slice::moveAdjEntry(adjEntry a, node u)
{
  edge e = a->theEdge();
  if (e->adjSource() == a)
    moveSource(e, u);
  else
    moveTarget(e, u);
}

void Slice::moveAdjacencies(node v, List<adjEntry> & inc)
{
  forall_listiterators(adjEntry, it, inc) {
    adjEntry a = *it;
    moveAdjEntry(a, v);
  }
}

void Slice::copy_inc(node u, List<adjEntry> & inc)
{
  inc.clear();
  adjEntry a;
  forall_adj(a, u) {
    inc.pushBack(a);
  }
}

//---------------------------

void Slice::copy_cycle(NodeArray<node> & vCopy, EdgeArray<edge> & eCopy, Cycle & C, Cycle & Copy)
{
#if DEBUG
  printf("Copying cycle\n");
#endif
  Copy.clear();
  
  forall_listiterators(edge, it, C) {
    edge e = *it;
#if DEBUG
    print_edge(e);
#endif
    assert(eCopy[e]); //will fail if the cycle is going through the centre
    Copy.pushBack(eCopy[e]);
  }
  //  mCycleData = data;

#if DEBUG
  printf("\n");
#endif
  
//   if (!mCycleData.mSymmetry) { //if there are some disks touching
//     mDiskLeft = Array<int>(mDiskNum+1);
//     int last = 0;
//     int first = 0;
//     int i = 0;
//     forall_listiterators(edge, it, C) {
//       edge e = *it;
//       int g = disk_group(e);
//       if (g && (!first || g != last)) {
// 	last = g;
// 	if (first)
// 	  break;
// 	if (!first)
// 	  first = g;

// 	mDiskLeft[g-1] = mCycleData.mCode && (1<<i);
// #if DEBUG
// 	printf("Setting side to disk group %d to %d\n", g, 1<<i);
// #endif
// 	i++;
//       }
//     }
//   }
}

// void Slice::show_unselected()
// {
//   if (!mUnselectedHidden)
//     return;

// #if DEBUG
//   printf("Showing edges:");
// #endif
  
//   node u;
//   forall_nodes(u, *this) {
//     forall_listiterators(edge, it, mOut[u]) {
//       edge e = *it;
//       edge f = mEdgeOrig[e];
//       assert(f);

//       if (mEmbedded[f])
// 	continue;

//       if (e->source() != u)
// 	continue;

//       restoreEdge(e);
// #if DEBUG
//       print_edge(e);
// #endif
//     }
//   }

// #if DEBUG
//   printf("\n");
// #endif

//   mUnselectedHidden = 0;
// }

// void Slice::hide_unselected()
// {
//   if (mUnselectedHidden)
//     return;

// #if DEBUG
//   printf("Hiding edges:");
// #endif

//   node u;
//   forall_nodes(u, *this) {
//     forall_listiterators(edge, it, mOut[u]) {
//       edge e = *it;
//       edge f = mEdgeOrig[e];
//       assert(f);
//       if (mEmbedded[f])
// 	continue;

//       if (e->source() != u)
// 	continue;

//       hideEdge(e);
// #if DEBUG
//       print_edge(e);
// #endif
//     }
//   }

// #if DEBUG
//   printf("\n");
// #endif

//   mUnselectedHidden = 1;
// }

void Slice::kuratowski_analysis(KuratowskiSubdivision & S, int isK33, CycleData & cycledata)
{
#if DEBUG
  printf("Slice::In kuratowski_analysis\n");
#endif
  kuratowski_nodes(S, cycledata.mNodes, isK33);

#if DEBUG
  printf("cycledata high %d, disks %d\n", cycledata.mCycleDisks.high(), mDiskNum);
#endif
  assert(cycledata.mCycleDisks.high() >= mDiskNum);

  cycledata.mIncNumber = 0;
  cycledata.mCycleDisks.fill(0, mDiskNum-1, 0);

  for (int i=0; i<9; i++) {
#if DEBUG
    printf("Edge %d:", i);
#endif
    forall_listiterators(edge, it, S[i]) {
      edge e = *it;
      int g = disk_group(e);
      if (g){
	cycledata.mCycleDisks[g-1]++;
#if DEBUG
	printf("%s in %d,%d", print_edge_str(e), mDiskInc[e->adjSource()]->id(), mDiskInc[e->adjTarget()]->id());
#endif
      }
    }
#if DEBUG
    printf("\n");
#endif
  }

#if DEBUG
  printf("Incident to disks:");
#endif

  for (int i = 0; i<mDiskNum; i++) {
    if (cycledata.mCycleDisks[i])
      cycledata.mIncNumber++;

#if DEBUG
    if (cycledata.mCycleDisks[i])
      printf(" %d", i);
#endif
  }

#if DEBUG
  printf("\n");
#endif
}

static Cycle cycles[MAXC];
static int att[MAXC];

int special_cycle_cmp(const void * pa, const void * pb)
{
  int a = *(int *)pa;
  int b = *(int *)pb;
  int d = att[a] - att[b];

  if (d)
    return d;

  return cycles[a].size() - cycles[b].size();
}

int Slice::count_inc(AdjEntryArray<Disk*> & inc, List<edge> & sg, node & center) //!!! center added
{
  int indisk = 0;
  int res = 0;
#if DEBUG
  printf("Cycle:");
#endif
  forall_listiterators(edge, it, sg) {
    edge e = *it;
#if DEBUG
    print_edge(e);
#endif

    if (is_center(e->source()) || is_center(e->target())) {
#if DEBUG
      printf(" ...goes through the center\n");
#endif
      return 10*sg.size(); //Arbitrary high constant - cycle goes through the center
    }
      
    if (disk_edge(e)) {
#if DEBUG
      printf("(%d)", disk_edge(e));
#endif
      indisk = 1;
    } else 
      if (indisk) {
	res++; 
	indisk = 0;
      }
  }

  if (indisk && !disk_edge(sg.front()))
    res++;

  if (indisk && !res)
    res = sg.size(); //Arbitrary high constant - the cycle cannot be picked!

#if DEBUG
  printf(" vertices:");
#endif    

  forall_listiterators(edge, it, sg) {
    edge e = *it;
    node u = e->source();
#if DEBUG
    printf(" %d", u->index());
#endif    
    adjEntry a;
    forall_adj(a, u) {  
      if (inc[a])
	res++;
    }

    u = e->target();
#if DEBUG
    printf(" %d", u->index());
#endif    
    forall_adj(a, u) {  
      if (inc[a])
	res++;
    }
  }

#if DEBUG
  printf(" [Att %d]\n", res);
#endif

  return res;
}

void Slice::push_disk_boundary(Disk * D, Cycle & C) { //!!!
}

void Slice::cycle_attachment(Disk * D, Cycle & C, List<node> & attachment) { //!!!
}

Disk * Slice::common_disk(node u, node v) { 
  return first_intersect(mDiskNodes[u], mDiskNodes[v]);
}

int Slice::disk_neighbors(List<node> a, List<node> b) { //!!!
  return 0;
}

int Slice::construct_cycles(KuratowskiSubdivision & S, int isK33, Cycle * rcycles, int &cnum, CycleData & cycledata)
{
#if DEBUG
  printf("In Slice::construct_cycles\n");
#endif
  int minval = 1 << (numberOfNodes()-3); //G.numberOfEdges() + 1;
  int minatt = numberOfNodes();

  int order[MAXC];
  if (isK33) {
    cnum = 2;
    int num = 3;

    for (int i = 0; i<6; i++) {
      
#if DEBUG
      printf("At %dth node of index %d in %d disks\n", i, cycledata.mNodes[i]->index(), mDiskNodes[cycledata.mNodes[i]].size());
#endif

      int disk_node = 0;
      if (mDiskNodes[cycledata.mNodes[i]].size()) {
	if (mDiskNodes[cycledata.mNodes[i]].front()->mCenter == cycledata.mNodes[i]) {
#if DEBUG
	  printf("The node is the center of disk %d\n", mDiskNodes[cycledata.mNodes[i]].front()->id());
#endif
	  
	  disk_node = 1;
	}
      }
      

      for (int j=0; j<3; j++) { //needs debugging
	CONSTRUCT_CYCLE(S, cycles[j], 4, k33_edges, k33_rev, (i+1)%3 + (i/3)*3, j, j + ((i/3 + 1)%2)*3, (i+2)%3, (i+2)%3 + (i/3)*3, (j+1)%3, (j+1)%3 + ((i/3+1)%2)*3, (i+1)%3);
	//CONSTRUCT_CYCLE(S, cycles[j], 4, k33_edges, k33_rev, (i+1)%3 + (i/3)*3, j, (i+2)%3 + (i/3)*3, j, (i+2)%3 + (i/3)*3, (j+1)%3, (i+1)%3 + (i/3)*3, (j+1)%3);
	
	order[j] = j;
	node center;
	att[j] = count_inc(mDiskInc, cycles[j], center); //!!! center not used
      }

      qsort(order, num, sizeof(int), special_cycle_cmp);

      int val = 0;
      for (int j=0; j<cnum; j++)
	val += 1 << (cycles[order[j]].size()-3);

#if DEBUG
      printf("Cycles of value %d found:\n", val);
      for (int j=0; j<num; j++) {
	printf("Cycle of length %d found: ", cycles[order[j]].size());
	print_edge_list(cycles[order[j]]);
      }
#endif

      if (disk_node || att[order[1]] < minatt || (att[order[1]] == minatt && val < minval)) {
	minval = val;
	minatt = att[order[1]];
	for (int j=0; j<cnum; j++)
	  rcycles[j] = cycles[order[j]];
      }
      
      if (disk_node)
	break;
    }
  } else {
#if DEBUG
  printf("It is K_5 subdivision, not tested!\n");
#endif
    cnum = 3;
    int num = 4;
    
    CONSTRUCT_CYCLE(S, cycles[0], 3, k5_edges, k5_rev, 0, 0, 1, 1, 2, 0);
    CONSTRUCT_CYCLE(S, cycles[1], 3, k5_edges, k5_rev, 0, 0, 1, 2, 3, 0);
    CONSTRUCT_CYCLE(S, cycles[2], 3, k5_edges, k5_rev, 0, 1, 2, 2, 3, 0);
    CONSTRUCT_CYCLE(S, cycles[3], 3, k5_edges, k5_rev, 1, 1, 2, 2, 3, 1);
//     CONSTRUCT(S, cycles[0], 3, 0, 1, 4);
//     CONSTRUCT(S, cycles[1], 3, 0, 2, 5);
//     CONSTRUCT(S, cycles[2], 3, 1, 2, 7);
//     CONSTRUCT(S, cycles[3], 3, 4, 5, 7);
#if DEBUG
  printf("Cycles constructed:\n");
  for(int i=0; i<num; i++)
    print_edge_list(cycles[i]);
#endif


    qsort(cycles, num, sizeof(List<edge>), cycle_cmp);
    
    int val = 0;
    for (int i=0; i<cnum; i++)
      val += 1 << (cycles[i].size()-3);
    
    if (val < minval) {
      minval = val;
      for (int i=0; i<cnum; i++)
	rcycles[i] = cycles[i];
    }
  }

  return minval;
}

int Slice::choose_cycles(KuratowskiSubdivision & S, int isK33, Cycle * cycles, CycleData & cycledata)
{
#if DEBUG
  printf("Slice::In choose cycles:\n");
#endif

  int num = 0;

#if DEBUG
  int val = 
#endif
    construct_cycles(S, isK33, cycles, num, cycledata);

#if DEBUG
  printf("Kuratowski subgraph of value %d and cycles %d\n", val, num);
  for(int i=0; i<num; i++)
    print_edge_list(cycles[i]);
#endif

  return num;
}

void Slice::clear_orientation(Disk * D)
{
  forall_listiterators(adjEntry, it, D->mOrient)
    mDiskInc[*it] = 0;

  D->mOrient.clear();
}

int Slice::orient_disk(List<edge> & cycle, AdjEntryArray<Disk*> & color, Disk * D, edge start)
{
#if DEBUG
  if (start)
    printf("In Slice::orient_disk starting at %s\n", print_edge_str(start));
  else
    printf("In Slice::orient_disk (no start)\n");
  print_edge_list(cycle);
#endif
  D->mOrient.clear();

  ListIterator<edge> sit;
  if (start) {
    int pos = cycle.search(start);
    assert(pos >= 0);
    sit = cycle.get(pos);
  } else
    sit = cycle.begin();

  adjEntry a = (*sit)->adjSource();
  if (color[a])
    a = a->twin();
  assert(!color[a]);

  D->mOrient.pushBack(a);

  for(ListIterator<edge> it = cycle.cyclicSucc(sit); *it != *sit; it = cycle.cyclicSucc(it)) {
    edge e = *it;
    assert(e);

    node v = e->commonNode(a->theEdge());
    assert(v);

    adjEntry b = e->adjSource();
    
    if ((a->theNode() == v) == (b->theNode() == v))
      b = b->twin();

    D->mOrient.pushBack(b);
    a = b;
  }

#if DEBUG
  forall_listiterators(adjEntry, it, D->mOrient)
    printf("%d ", (*it)->theNode()->index());
  printf("\n");
#endif

  forall_listiterators(adjEntry, it, D->mOrient) {
    assert(!color[*it]);
    color[*it] = D;
  }

  D->recompute();

  return 0;
}

// int Slice::orient_cycle(edge s, EdgeArray<int> & color, List<adjEntry> & orient)
// {
// #if DEBUG
//   printf("In Slice::orient_cycle\n");
// #endif
//   orient.pushBack(s->adjSource());

//   edge e;
//   node u = s->source();

//   node v = u;
//   node w = s->target();
//   while (w != u) {
//     node x = 0;
//     forall_adj_edges(e, w) {
//       if (color[e] == color[s]) {
// 	x = e->opposite(w);
// 	if (x != v)
// 	  break;
//       }
//     }
//     assert(x);

//     if (e->source() != w)
//       reverseEdge(e);
//     orient.pushBack(e->adjSource());
//     v = w;
//     w = x;
//   }

// #if DEBUG
//   forall_listiterators(adjEntry, it, orient)
//     printf("%d ", (*it)->theNode()->index());
//   printf("\n");
// #endif

//   return 0;
// }

void Slice::regroup(Disk * D)
{
  forall_listiterators(adjEntry, it, D->mOrient) {
    adjEntry a = *it;
    node u = a->theNode();
    adjEntry b;
    forall_adj(b, u) {
      int g = disk_group(b->theEdge());
      if (g && g != D->mGroup) {
	forall_disks(it, *this) {
	  Disk * E = *it;
	  if (E->mGroup == g)
	    E->mGroup = D->mGroup;
	}
      }
	
    }
  }
}

void Slice::cut_off_disk(Disk * D, EdgeArray<int> & disk, Disk * newD)
{
#if DEBUG
  printf("Slice::cut_off_disk, disk %d from disk %d\n", D->id(), newD->id());
#endif

  NodeArray<node> copy(*this, 0);
  Cycle newcycle;
  forall_listiterators(adjEntry, it, D->mOrient) {
    adjEntry a = *it;
    node v = a->theNode();
    node w = v;
    if (incident(v, newD)) {
      w = newNodeCopy(v);

      assert(D->mNodePairs[v]);
      D->mNodePairs[D->mNodePairs[v]] = w;
      D->mNodePairs[w] = D->mNodePairs[v];

      mPoss[w].pushBack(v);
      mPoss[w].pushBack(newD->mNodePairs[v]);
    }
    copy[v] = w;
  }

  forall_listiterators(adjEntry, it, D->mOrient) {
    adjEntry a = *it;
    edge e = a->theEdge();
    edge f = e;
    if (copy[e->source()] != e->source() && copy[e->target()] != e->target()) {
      f = newEdgeCopy(e, copy[e->source()], copy[e->target()]);
    } else {
      moveSource(e, copy[e->source()]);
      moveTarget(e, copy[e->target()]);
    }

    newcycle.pushBack(f);
    mDiskInc[a] = 0;
  } 

  orient_disk(newcycle, mDiskInc, D);
}

void Slice::duplicate(node u, List<node> & nodes, Disk * D1, Disk * D2)
{
  assert(u);

  if (D1->mNodePairs[u])
    return;

  nodes.pushBack(u);

  node v = newNode();
  mNodeOrig[v] = mNodeOrig[u];

  D1->mNodePairs[u] = v;
  D1->mSide[u] = 0;

  D2->mNodePairs[v] = u;
  D2->mSide[v] = 1;

  mDiskNodes[u].pushBack(D1);
  mDiskNodes[v].pushBack(D2);
}

void Slice::duplicate_cycle(Cycle & cycle, Disk * D1, Disk * D2)
{
#if DEBUG
  printf("Slice::duplicate_cycle\n");
#endif

  List<node> nodes;
  EdgeArray<int> einc(*this, 0);
  List<edge> del;

  forall_listiterators(edge, it, cycle) {
    edge e = *it;
    einc[e] = 1;

    duplicate(e->source(), nodes, D1, D2);
    duplicate(e->target(), nodes, D1, D2);
  }

#if DEBUG
  printf("cycle duplicated\n");
#endif


  forall_listiterators(node, it, nodes) {
    node v = *it;

    List<adjEntry> adj;
    adjEntries(v, adj);

    forall_listiterators(adjEntry, it, adj) {
      adjEntry a = *it;
      edge e = a->theEdge();
      if (einc[e]) 
	continue;
      
      if (disk_edge(e)) {
#if DEBUG
	printf("Edge %s is a disk edge (disk %d), group %d\n", print_edge_str(e), disk_edge(e), disk_group(e));
#endif

	cut_off_disk(disk(e), einc, D1);
      } else {
	node w = newNodeCopy(v);
	moveAdjEntry(a, w);

	mPoss[w].pushBack(v);
	mPoss[w].pushBack(D1->mNodePairs[v]);

#if DEBUG
	printf("New node %d created - origin %d, poss: %d, %d\n", w->index(), mNodeOrig[w]->index(), v->index(), D1->mNodePairs[v]->index());
#endif
      }
    }
  }
  
  //remove duplicate edges
  forall_listiterators(edge, it, del) {
    delEdge(*it);
  }
}

void Slice::create_one_disk(Cycle & cycle)
{
#if DEBUG
  printf("Slice::create_one_disk - cycle of length %d\n", cycle.size());
#endif
  
  Disk * D = new Disk(*this);

  add_disk(D);

  Cycle disk;
  Cycle inv;

  duplicate_cycle(cycle, D, D);
  
  edge e = 0;
  forall_listiterators(edge, it, cycle) {
    if (!disk_edge(*it)) {
      e = *it;
#if DEBUG
      printf("Edge %s chosen with signature -1\n", print_edge_str(e));
#endif

      for (it = cycle.cyclicSucc(it); *it != e; it = cycle.cyclicSucc(it)) {
	edge f = *it;
    
	disk.pushBack(f);
	
	edge g = newEdgeCopy(f, D->mNodePairs[f->source()], D->mNodePairs[f->target()]);
	inv.pushBack(g);
      }
      break;
    }
  }
  assert(e);

  
  node u = e->target();
  node v = e->source();
  node w;
  if (cycle.back()->isIncident(u))
    SWAP(u, v, w);

  list_remove(mEdgeCopies[mEdgeOrig[e]], e);
  assert(mEdgeOrig[e]);

  edge f;
  f = newEdgeCopy(e, u, D->mNodePairs[v]);
  inv.pushBack(f);

  f = newEdgeCopy(e, v, D->mNodePairs[u]);
  disk.pushBack(f);

  delEdge(e);

#if DEBUG
  printf("Disk boundary created:\n");
  print_graph_fast(*this);
  consistencyCheck();
#endif


  disk.conc(inv);
#if DEBUG
  printf("Disk edges:");
  print_edge_list(disk);
#endif

  orient_disk(disk, mDiskInc, D);
}

void Slice::create_two_disks(Cycle & cycle)
{
#if DEBUG
  printf("Slice::create_two_disks\n");
#endif

  Disk * D1 = new Disk(*this);
  Disk * D2 = new Disk(*this);
  D1->mPair = D2;
  D2->mPair = D1;

  add_disk(D1);
  add_disk(D2);

  Cycle disk2;

  duplicate_cycle(cycle, D1, D2);

  // Copy the cut cycle
  edge fc = 0;
  forall_listiterators(edge, it, cycle) {
    edge f = *it;

    edge g = newEdge(D1->mNodePairs[f->source()], D1->mNodePairs[f->target()]);
    assert(mEdgeOrig[f]);
    mEdgeOrig[g] = mEdgeOrig[f];
    mEdgeCopies[mEdgeOrig[f]].pushBack(g);
    disk2.pushBack(g);

    if (!fc) 
      fc = g;
  }

#if DEBUG
  printf("Disk 2 edges:");
  print_edge_list(disk2);
#endif

  assert(fc);
  orient_disk(cycle, mDiskInc, D1);
  orient_disk(disk2, mDiskInc, D2);
  assert(D1->mNodePairs[D1->mOrient.front()->theNode()] == D2->mOrient.front()->theNode());
  assert(D1->mNodePairs[D1->mOrient.front()->twinNode()] == D2->mOrient.front()->twinNode());
}

int Slice::add_disk(Disk * D)
{
#if DEBUG
  printf("Slice::add_disk - color %d\n", mDiskNum+1);
#endif

  if (D->has_pair())
    mDouble++;
  else
    mSingle++;

  mDisks.grow(1, D);
  D->mId = ++mDiskNum;
  D->mGroup = D->mId;
  return mDiskNum;
}

node Slice::add_center(Disk * D)
{
  if (D->mCenter) {
    forall_listiterators(edge, it, D->mCenterEdges) {
      restoreEdge(*it);
    }
  } else {
    D->mCenter = newNode();
    mDiskNodes[D->mCenter].pushBack(D);

    forall_listiterators(adjEntry, it, D->mOrient) {
      edge e = newEdge(D->mCenter, (*it)->theNode());
      mDiskInc[e->adjSource()] = D;
      D->mCenterEdges.pushBack(e);
    }
  }

  D->mCenterVisible = 1;
  return D->mCenter;
}

void Slice::add_centers()
{
  for(int i=0; i<mDiskNum; i++)
    add_center(mDisks[i]);

  mCentersVisible = 1;
}

void Slice::remove_center(Disk * D)
{
  assert(D->mCenter);

  forall_listiterators(edge, it, D->mCenterEdges) {
    hideEdge(*it);
  }
  D->mCenterVisible = 0;
}

void Slice::remove_centers()
{
  for(int i=0; i<mDiskNum; i++)
    remove_center(mDisks[i]);

  mCentersVisible = 0;
}

Disk * Slice::is_center(node v)
{
  if (mDiskNodes[v].size() != 1)
    return NULL;
  else
    return mDiskNodes[v].front();
}


int Slice::cut_along_twosided(Cycle & cycle)
{
#if DEBUG
  printf("Slice::Cutting along two-sided cycle:");
  print_edge_list(cycle);
  assert(consistencyCheck());
#endif
  //  show_unselected();
  create_two_disks(cycle);
  mGenus -= 2;

  return 0;
}

int Slice::cut_along_onesided(Cycle & cycle)
{
#if DEBUG
  printf("Slice::Cutting along one-sided cycle:");
  print_edge_list(cycle);
  assert(consistencyCheck());
#endif
  //  show_unselected();
  create_one_disk(cycle);
  mGenus--;

  return 0;
}

int Slice:: disk_orientation(Disk * D, int withcenters)
{
#if DEBUG
  printf("In Slice::disk_orientation of disk %d with centers %d\n", D->id(), withcenters);
  assert(withcenters == mCentersVisible);
#endif

  if (withcenters) {
    adjEntry a = D->mOrient.front();
    node u = a->theNode();
    adjEntry b = D->prev_arc(u);
    
    adjEntry c = a;
    while (c != b) {
      c = c->cyclicSucc();
      if (c->twinNode() == D->center())
	return 1;
    }
    return -1;
  }
    
  forall_listiterators(adjEntry, it, D->mOrient) {
    adjEntry a = *it;
    adjEntry pred = a->cyclicPred();
    adjEntry next = a->cyclicSucc();

#if DEBUG
    printf("at %d, left: %d, right: %d\n", a->theNode()->index(), pred->twinNode()->index(), next->twinNode()->index());
#endif

    if (mDiskInc[pred->twin()] != D) 
      return -1;

    if (mDiskInc[next->twin()] != D) 
      return 1;
  }

  return 0;
}

int Slice::test_disks_orientation(int orient, Disk * A, Disk * B)
{
#if DEBUG
  printf("In Slice::test_disks_orientation, orientation: %d, disks %d, %d\n", orient, A->mId, B->mId);
#endif

//   if (A->mOutEdges < 3 || B->mOutEdges < 3) 
//     return 1;

  int oA = disk_orientation(A);

#if DEBUG
  printf("Disk A orientation: %d\n", oA);
#endif

  if (!oA)
    return 1;

  int oB = disk_orientation(B);

#if DEBUG
  printf("Disk B orientation: %d\n", oB);
#endif

  if (!oB)
    return 1;

  int pairing = A->pairing_sign();
#if DEBUG
  printf("Disks pairing sign: %d\n", pairing);
#endif

  if ((oA == oB*pairing) == (orient < 0))
    return 1;

//   add_center(A);
//   add_center(B);
  int conn = k_connected(*this, A->mCenter, B->mCenter, 3);
//   remove_center(A);
//   remove_center(B);

  if (!conn)
    return 1;

  return 0;
}

int Slice::is_orientable(Cycle * cycles, int & num)
{
#if DEBUG
  printf("In Slice::is_orientable, mDiskNum %d, mSingle %d, mDouble %d, mOrientable %d\n", mDiskNum, mSingle, mDouble, mOrientable);
#endif
  if (!mDiskNum)
    return 1;
  
  if (mSingle || !mOrientable)
    return 1;

  if (mOrientable > 0) {
    for (int i = 0; i<mDiskNum; i++) {
      if (mDisks[i]->mId < mDisks[i]->pair()->mId) {
	int res = test_disks_orientation(mOrientable, mDisks[i], mDisks[i]->pair());
	if (!res)
	  return 0;
      }
    }
  }

  if (mOrientable < 0) {
    for (int i = 0; i<mDiskNum; i++) {
      if (mDisks[i]->mId < mDisks[i]->pair()->mId) {
	int res = test_disks_orientation(mOrientable, mDisks[i], mDisks[i]->pair());
	if (res)
	  return 1;
      }
    }
    return 0;
  }
  
  return 1;
}

int Slice::incident(edge e, Disk * D)
{
  return mDiskInc[e->adjSource()] == D || mDiskInc[e->adjTarget()] == D;
}

int Slice::incident(node v, Disk * D)
{
  return mDiskNodes[v].search(D) >= 0;
}

void Slice::join_disks(Disk * D1, Disk * D2, node source, node target) //!!!source added
{
#if DEBUG
  printf("Joining disks %d, %d\n", D1->id(), D2->id());
#endif

  //we assume that two disks share at most one segment

  NodeArray<node> merge(*this, 0);
    
  forall_listiterators(adjEntry, it, D1->mOrient) {
    adjEntry a = *it;

    node u = a->theNode();
    forall_listiterators(node, nit, mPoss[u]) {
      node v = *nit;
      if (incident(v, D2) && (D2->has_pair() || D2->mSide[target] == D2->mSide[v])) {
	merge[u] = v;
	merge[v] = u;
      }
    }
  }

  Cycle C;
  edge start = 0;

  forall_listiterators(adjEntry, it, D1->mOrient) {
    adjEntry a = *it;
    node v = a->theNode();
    node w = a->twinNode();
#if DEBUG
    printf("At %d, %d, %d, %d\n", v->index(), w->index(), index(merge[v]), index(merge[w]));
#endif
    if (merge[v] && merge[w]) {
      edge e = searchEdge(merge[v], merge[w]);  
      assert(e);
      C.pushBack(e);
      start = e;
      delEdge(a->theEdge());
    } else
      C.pushBack(a->theEdge());
  }

  clear_orientation(D1);

  forall_listiterators(adjEntry, it, D2->mOrient) {
    adjEntry a = *it;
    node v = a->theNode();
    if (merge[v])
      identify_nodes(merge[v], v);
  }

  assert(start);
  orient_disk(C, mDiskInc, D1, start);
}

int Slice::disk_choices()
{
#if DEBUG
  printf("In disk_choices\n");
#endif

  if (!mDiskNum || (mDiskNum <= 2 && mDisks[0]->has_pair()))
    return 0;

  forall_disks(dit, *this) {
    Disk * D = *dit;
    int count = 0; //!!!Wrong, need to count for each disk separately
    
    forall_listiterators(adjEntry, it, D->mOrient) {
      adjEntry a = *it;
      node u = a->theNode();

      if (!mPoss[u].empty())
	count++;
    }

#if DEBUG
    if (count)
      printf("Disk %d has %d vertices to be attached\n", D->id(), count);
#endif
    if (count > 1)
      return 1;
  }

  return 0;
}

node Slice::identify_nodes(node u, node v)
{
#if DEBUG
  printf("Identifying nodes %d, %d\n", u->index(), v->index());
#endif

  adjEntry a;
  forall_adj(a, u) {
    moveAdjEntry(a, v);
  }

  forall_listiterators(Disk*, it, mDiskNodes[u]) {
    Disk * D = *it;
    mDiskNodes[v].pushBack(D);
    D->mNodePairs[v] = D->mNodePairs[u];
  }
  delNode(u); //! Should we?

  return v;
}

node Slice::identify_nodes_reversible(node u, node v, List<adjEntry> & inc)
{
#if DEBUG
  printf("Identifying nodes %d, %d\n", u->index(), v->index());
#endif

  adjEntry a;
  forall_adj(a, u) {
    moveAdjEntry(a, v);
    inc.pushBack(a);
  }

  forall_listiterators(Disk*, it, mDiskNodes[u]) {
    Disk * D = *it;
    mDiskNodes[v].pushBack(D);
    D->mNodePairs[v] = D->mNodePairs[u];
  }
  delNode(u); //! Should we?
  
  return v;
}

Slice * Slice::test_choice(node u, node v)
{
  NodeArray<node> nCopy;
  EdgeArray<edge> eCopy;
  Slice S(*this, nCopy, eCopy);
  identify_nodes(nCopy[u], nCopy[v]);
  
  return embed();
}

Slice * Slice::test_choice_inside(node & u, node v)
{
#if DEBUG
  printf("In test_choice_inside at nodes %d, %d\n", u->index(), v->index());
#endif

  List<adjEntry> inc;
  int oldid = u->index();
  node w = identify_nodes_reversible(u, v, inc);

  Slice * res = embed();

  if (!res)
    u = split_node(w, inc, oldid);

  return res;
}

/* Tests all possible combinations of a node */
Slice * Slice::test_choices(node w)
{
#if DEBUG
  printf("In test_choices at node %d\n", w->index());
#endif

  Slice * res;
  List<node> choices(mPoss[w]); // copy mPoss[w] so we can delete w
  forall_listiterators(node, it, choices) {
    node u = *it;
    res = test_choice_inside(w, u);
#if DEBUG
    printf("Coming back (succ=%d) with node %d\n", !!res, w->index());
#endif
    if (res)
      return res;
  }
  assert(w);
  mPoss[w] = choices;
  return 0;
}


node Slice::disk_attachment(Disk * D, edge & e)
{
  if (incident(e->source(), D))
    return e->target();

  if (incident(e->target(), D))
    return e->source();

  assert(0);
  return 0;
}


Slice * Slice::test_all_choices()
{
#if DEBUG
  printf("In test_all_choices\n");
#endif

  Slice * res = this;
  node u;
  forall_nodes(u, *this) {
    if (mPoss[u].empty())
      continue;
      
    res = test_choices(u);
    break;
  }

  return res;
}

Slice * Slice::test_all_disk_choices()
{
#if DEBUG
  printf("In Slice::test_all_disk_choices\n");
#endif

  forall_disks(dit, *this) {
    Disk * D = *dit;
    int count = 0; //!!!Wrong, need to count for each disk separately

    node source = 0;
    
    forall_listiterators(adjEntry, it, D->mOrient) {
      adjEntry a = *it;
      node u = a->theNode();

      if (!mPoss[u].empty()) {
	source = u;
	count++;
      }
    }

#if DEBUG
    if (count)
      printf("Disk %d has %d vertices to be attached\n", D->id(), count);
#endif

    if (count > 1) {
      
      forall_listiterators(node, it, mPoss[source]) {
	node target = *it;

#if DEBUG
	print_list< List<Disk*>, ListIterator<Disk *> >(mDiskNodes[target]);
#endif

	Disk * D2 = mDiskNodes[target].front();
	assert(D2);
	assert(D != D2);
	if (D2->check_symmetry(target)) {
#if DEBUG
	  printf("Target %d skipped by symmetry\n", target->index());
#endif
	  continue;
	}

#if DEBUG
	printf("Attaching disk %d onto %d, source %d target %d\n", D->id(), D2->id(), source->index(), target->index());
#endif

	Slice * emb;
	NodeArray<node> vCopy;
	EdgeArray<edge> eCopy;

	Slice * S = new Slice(*this, vCopy, eCopy);
	S->join_disks(S->mDisks[D->id()-1], S->mDisks[D2->id()-1], vCopy[source], vCopy[target]);
	S->compute_unselected();
	emb = S->embed();
	
	if (emb != S)
	  delete S;
	
	if (emb)
	  return emb;
      }
      return 0;
    }
  }

  assert(0);
  return 0;
}

Slice * Slice::extend_embedding()
{
#if DEBUG
  printf("In Slice::extend embedding\n");
#endif

  Slice * emb;
  if (disk_choices()) 
    emb = test_all_disk_choices();
  else
    emb = test_all_choices();
//   Slice * emb = test_extend_smart();

  return emb;
}

int Slice::noncontractible_cycles(Cycle * cycles, int & num)
{
#if DEBUG
  printf("In Slice::noncontractible_cycle, genus %d\n", mGenus);
#endif

  num = 0;

  //  num = test_ears(cycles);

  int planar = 1;
  if (mGenus) {
    KuratowskiWrapper K;
    planar = best_k_graph(*this, K);

    if (!planar) {
	
#if DEBUG
      printf("Chosen k-graph:");
      print_edge_list(K.edgeList);
#endif

      CycleData cycledata;
      cycledata.mCycleDisks.init(2*MAXGENUS+1);
      KuratowskiSubdivision S;
      int isK33 = K.isK33();
      transform(*this, K, S);

      //if (mDiskNum) {
      kuratowski_analysis(S, isK33, cycledata);

#if DEBUG
      printf("Kuratowski graph incident to %d disks\n", cycledata.mIncNumber);
#endif
      //}

      num = choose_cycles(S, isK33, cycles, cycledata);
      return !num;

//       num = best_cycles(*this, K, cycles);
//       return !num;
    }
  } else {
    //    if (!mDiskNum) 
    //      return test_planarity(*this);
    
#if DEBUG
    printf("Embedding needed\n");
    print_graph(*this);
    print_graph_graph6(*this);
#endif
    
    planar = test_planarity_with_embedding(*this);
  }

  if (!planar) 
    return 0;

#if DEBUG
  printf("Slice is planar\n");
#endif

  int orient = is_orientable(cycles, num);

#if DEBUG
  printf("The embedding has %s orientation\n", strcorrect[orient].c_str());
#endif

  return orient;
}

Slice * Slice::cut_cycles(Cycle * cycles, int num)
{
#if DEBUG
  printf("In cut_cycles, genus=%d\n", mGenus);
#endif

  if (!mGenus)
    return 0;
  
  assert(num);
  
  NodeArray<node> vCopy;
  EdgeArray<edge> eCopy;
  
  Slice * emb = 0;
  Cycle act;

  for (int i = 0; i<num; i++) {
    if (mGenus && mOrientable <= 0) {
#if DEBUG
      printf("Testing one-sided curves:\n");
#endif
      Slice * S = new Slice(*this, vCopy, eCopy);
      S->copy_cycle(vCopy, eCopy, cycles[i], act);
      S->cut_along_onesided(act);
      S->compute_unselected();
      S->compute_disk_inc();
      emb = S->embed();
	
      if (emb != S)
	delete S;
      
      if (emb)
	goto END;
    }
    
    if (mGenus > 1) {
#if DEBUG
      printf("Testing two-sided curves (%d out of %d):\n", i+1, num);
#endif
      
      Slice * S = new Slice(*this, vCopy, eCopy);
      S->copy_cycle(vCopy, eCopy, cycles[i], act);
      S->cut_along_twosided(act);
      S->compute_unselected();
      S->compute_disk_inc();
      emb = S->embed();
      
      if (emb != S)
	delete S;
      
      if (emb)
	goto END;
    }
  }
  
  
 END:
  return emb;
}

Slice * Slice::embed()
{
  compute_unselected();
#if DEBUG
  printf("In Slice::embed - genus %d, orientable: %d, %d unselected edges\n", mGenus, mOrientable, mUnselected);
  print_slice();
  assert(consistencyCheck());
#endif

  //hide unselected edges
  //  hide_unselected();

  Cycle cycles[MAXC];
  int planar = 0;
  int num = 0;

  add_centers();

  planar = noncontractible_cycles(cycles, num);

  remove_centers();

#if DEBUG
  printf("Slice is %s with %d unselected edges\n", str[planar].c_str(), mUnselected);
#endif

  Slice * emb = 0;
  if (planar) {
    if (!mUnselected)
      emb = this;
    else
      emb = extend_embedding();
  } else {
    emb = cut_cycles(cycles, num);
  }

#if DEBUG
  if (!emb)
    printf("Graph cannot be embedded\n");
#endif
  return emb;
}

Slice * Slice::embed_in_surface(int genus, int orientable)
{
  mGenus = genus;
  mOrientable = orientable;
    
  return embed();
}

void Slice::flip_disk(Disk * D)
{
#if DEBUG
  printf("Slice::flip_disk, Disk %d\n", D->mCenter->index());
#endif      

  NodeArray<int> cut;

  //  add_center(D);
  //  add_center(D->pair());
  int k = min_cut(*this, D->mCenter, D->pair()->mCenter, cut);
#if DEBUG
  printf("Mincut between %d and %d of size %d found\n", D->mCenter->index(), D->pair()->mCenter->index(), k);
#endif      
  //  remove_center(D);
  //  remove_center(D->pair());

  assert(k < 3);

  node u;
  forall_nodes(u, *this) {
    if (!cut[u]){
#if DEBUG
      printf("Reversing local rotation at %d\n", u->index());
#endif      
      reverseAdjEdges(u);
      Disk * F;
      if ((F = is_center(u)))
	F->mClockwise = -F->mClockwise;
      continue;
    }

    adjEntry a;
    forall_adj(a, u) {
      node v = a->twinNode();
      if (!cut[v]) {
	adjEntry first = a;
	while (!cut[first->cyclicPred()->twinNode()])
	  first = first->cyclicPred();

#if DEBUG
	printf("Untwisting local rotation at %d\n", u->index());
	printf("Endpoints: %d", first->twinNode()->index());
#endif      
	adjEntry b = first;
	a = first->cyclicSucc();
	while (!cut[a->twinNode()]) {
	  adjEntry c = a->cyclicSucc();
	  moveAdjBefore(a, b);
#if DEBUG
	  printf(" %d", a->twinNode()->index());
#endif      
	  b = a;
	  a = c;
	}
#if DEBUG
	printf("\n");
#endif      

	break;
      }
    }
  }
}

void Slice::check_disk_embedding(Disk * D, int oD) 
{
#if DEBUG
  printf("Slice::check_disk_embedding disk %d orient %d\n", D->id(), oD);
  assert(consistencyCheck());
#endif      

  forall_listiterators(adjEntry, it, D->mOrient) {
    adjEntry a = *it;
    node u = a->theNode();
    adjEntry center = D->center_arc(u);
    adjEntry b = D->prev_arc(u);

#if DEBUG
    print_local_emb(u);
    printf("Arc %s, back arc %s, center arc %s\n", print_arc_string(a).c_str(), print_arc_string(b).c_str(), print_arc_string(center).c_str()); 
#endif      

    if (oD > 0) {
      moveAdjBefore(a, center);
      moveAdjAfter(b, center);
    } else {
      moveAdjAfter(a, center);
      moveAdjBefore(b, center);
    }

#if DEBUG
    print_local_emb(u);
#endif      
  }
}

void Slice::set_embedding(EdgeArray<int> & signature)
{
#if DEBUG
  printf("Slice::set_embedding\n");
#endif      
  signature.init(*mOrig, 1);

  add_centers();
  test_planarity_with_embedding(*this);

  AdjEntryArray<adjEntry> lpair(*this, 0);
  AdjEntryArray<adjEntry> rpair(*this, 0);
  AdjEntryArray<int> lsign(*this, 1);
  AdjEntryArray<int> rsign(*this, 1);

  for(int i=0; i<mDiskNum; i++) {
    Disk * D = mDisks[i];
    if (D->has_pair() && D->mId < D->pair()->mId) {
      D->mClockwise = disk_orientation(D);
#if DEBUG
      printf("Disk A orientation: %d\n", D->mClockwise);
#endif
      check_disk_embedding(D, D->mClockwise);

      D->pair()->mClockwise = disk_orientation(D->pair());
#if DEBUG
      printf("Disk B orientation: %d\n", D->pair()->mClockwise);
#endif
      check_disk_embedding(D->pair(), D->pair()->mClockwise);

      int pairing = D->pairing_sign();
      assert(D->mClockwise && D->pair()->mClockwise);

      int dsign;
      if (D->mClockwise != D->pair()->mClockwise*pairing)
	dsign = 1;
      else
	dsign = -1;

      if (mOrientable && dsign < 0) {
	flip_disk(D);
	dsign = 1;
      }

      if (dsign < 0) {
#if DEBUG
	printf("Changing signature of edges:");
#endif      
	
	forall_listiterators(adjEntry, it, D->pair()->mOrient) {
	  node u = (*it)->theNode();

	  edge e ;
	  forall_adj_edges(e, u) {
	    if (incident(e, D->pair()))
	      continue;
	    
	    edge f = mEdgeOrig[e];
	    assert(f);

#if DEBUG
	    print_edge(f);
#endif      

	    assert(signature[f] > 0); //Changing signature twice, is it ok?

	    signature[f] = -signature[f];
	  }
	}
#if DEBUG
	printf("\n");
#endif      
      }

#if DEBUG
      printf("Pairing disks %d, %d of orientations %d, %d => sign %d\n", D->mId, D->pair()->mId, D->mClockwise, D->pair()->mClockwise, dsign);
#endif      

      forall_listiterators(adjEntry, it, D->mOrient) {
	adjEntry a = *it;
	node u = a->theNode();
	node v = D->mNodePairs[u];
	adjEntry b;
	if (pairing > 0)
	  b = D->pair()->arc(v);
	else
	  b = D->pair()->prev_arc(v);

#if DEBUG
	printf("Pairing arcs %d->%d and %d->%d\n", a->theNode()->index(), a->twinNode()->index(), b->theNode()->index(), b->twinNode()->index());
#endif      

	if (D->mClockwise < 0) {
	  lpair[a] = b;
	  lsign[a] = dsign;
	  rpair[a->twin()] = b->twin();
	  rsign[a->twin()] = dsign;
	} else {
	  rpair[a] = b;
	  rsign[a] = dsign;
	  lpair[a->twin()] = b->twin();
	  lsign[a->twin()] = dsign;
	}

	if (D->pair()->mClockwise*pairing < 0) {
	  lpair[b] = a;
	  lsign[b] = dsign;
	  rpair[b->twin()] = a->twin();
	  rsign[b->twin()] = dsign;
	} else {
	  rpair[b] = a;
	  rsign[b] = dsign;
	  lpair[b->twin()] = a->twin();
	  lsign[b->twin()] = dsign;
	}
      }
    } 

    if (!D->has_pair()) {
      int o = disk_orientation(D);
#if DEBUG
      printf("Disk %d has orientation %d\n", D->id(), o);
#endif      
      
      int dsign = -1;
      node start = D->mOrient.front()->theNode();
      assert(start);

      if (dsign < 0) {
#if DEBUG
	printf("Changing signature of edges:");
#endif      

	adjEntry a = D->mOrient.front();
	edge start = mEdgeOrig[a->theEdge()];
	assert(start);

#if DEBUG
	print_edge(start);
#endif      
	assert(signature[start] > 0); //Changing signature twice, is it ok?
	signature[start] = -signature[start];
	
	forall_listiterators(adjEntry, it, D->mOrient) {
	  edge e = (*it)->theEdge();
	  edge f = mEdgeOrig[e];
	  if (f == start)
	    break;

	  node u = (*it)->twinNode();
	  if (u->index() > D->mNodePairs[u]->index())
	    continue;

	  edge h;
	  forall_adj_edges(h, u) {
	    if (incident(h, D))
	      continue;
	    
	    edge f = mEdgeOrig[h];
	    assert(f);

#if DEBUG
	    print_edge(f);
#endif      

	    assert(signature[f] > 0); //Changing signature twice, is it ok?

	    signature[f] = -signature[f];
	  }
	}
#if DEBUG
	printf("\n");
#endif      
      }

#if DEBUG
      printf("Pairing arcs in one-sided disk %d starting at %d\n", D->mId, start->index());
#endif      
      
      forall_listiterators(adjEntry, it, D->mOrient) {
	adjEntry a = *it;
	node u = a->theNode();
	node v = D->mNodePairs[u];
	adjEntry b = D->pair()->arc(v);

	if (D->mNodePairs[u] == start)
	  break;

#if DEBUG
	printf("Pairing arcs %d->%d and %d->%d\n", a->theNode()->index(), a->twinNode()->index(), b->theNode()->index(), b->twinNode()->index());
#endif      

	if (o < 0) {
	  lpair[a] = b;
	  lsign[a] = dsign;
	  rpair[a->twin()] = b->twin();
	  rsign[a->twin()] = dsign;
	} else {
	  rpair[a] = b;
	  rsign[a] = dsign;
	  lpair[a->twin()] = b->twin();
	  lsign[a->twin()] = dsign;
	}
      }
      
    }
  }

  remove_centers();

#if DEBUG
  printf("Setting embedding\n");
  print_emb(*this);
#endif

  NodeArray<int> oriented(*mOrig, 0);

  node u;
  forall_nodes(u, *this) {
    node v = mNodeOrig[u];

    if (!v) {
#if DEBUG
      printf("Warning: %d has no original copy\n", u->index());
#endif

      continue;
    }

    if (oriented[v])
      continue;

    oriented[v] = 1;
    
#if DEBUG
    printf("Reorienting %d (from %d)\n", v->index(), u->index());
#endif

    adjEntry a = u->firstAdj();
    adjEntry last = 0;

    edge start = mEdgeOrig[a->theEdge()];
    assert(start);
    edge f = start;
    int sign = 1;

    int numsteps = 0;

    do {
      adjEntry orig = get_adj(f, v);
      assert(orig);

#if DEBUG
      printf("At %d->%d, original %d->%d, sign %d\n", a->theNode()->index(), a->twinNode()->index(), orig->theNode()->index(), orig->twinNode()->index(), sign);
#endif

      if (last)
	moveAdjAfter(orig, last);

      last = orig;

      while ((sign > 0 && rpair[a]) || (sign < 0 && lpair[a])) {
	if (sign > 0 && rpair[a]) {
#if DEBUG
	  printf("Switching to %d->%d (sign %d)\n", rpair[a]->theNode()->index(), rpair[a]->twinNode()->index(), rsign[a]);
#endif
	  a = rpair[a];
	  sign = sign * rsign[a];
	}
	
	if (sign < 0 && lpair[a]) {
#if DEBUG
	  printf("Switching to %d->%d (sign %d)\n", lpair[a]->theNode()->index(), lpair[a]->twinNode()->index(), lsign[a]);
#endif
	  a = lpair[a];
	  sign = sign * lsign[a];
	}
      }

      if (sign > 0)
	a = a->cyclicSucc();
      else
	a = a->cyclicPred();

      f = mEdgeOrig[a->theEdge()];
      assert(f);

      if (numsteps++ > numberOfNodes()) {
#if DEBUG
	printf("Infinite cycle\n");
#else
	fprintf(stderr, "Infinite cycle\n");
#endif
	break;
      }


    } while (f != start);
  }
}

adjEntry Slice::find_adj(node u, node v)
{
  adjEntry a;
  forall_adj(a, u) 
    if (a->twinNode() == v)
      return a;

  return NULL;
}

//----------------------- Embedder --------------------------------------

Embedder::Embedder(const Graph & G): Graph(G), mSlice(0), mSliceEmb(0), mSignature(*this, 0)
{
}

Embedder::Embedder(const Graph & G, NodeArray< node > & mapNode, EdgeArray< edge > & mapEdge): mSlice(0), mSliceEmb(0), mSignature(*this, 0)
{
  construct(G, mapNode, mapEdge);
}

Embedder::Embedder(const Embedder & G): Graph(G), mSlice(G.mSlice), mSliceEmb(0), mSignature(*this, 0)
{
  
}

Embedder::~Embedder()
{
  if (mSlice)
    delete mSlice;
}
  
int Embedder::planar()
{
  return test_planarity(*this);
}
 
int Embedder::embed(int genus, int orientable)
{
  Slice * S = new Slice(*this);
  mSlice = S->embed_in_surface(genus, orientable);
  
  if (S != mSlice)
    delete S;

  return !!mSlice;
}

int Embedder::set_embedding(Slice * slice)
{
  if (!slice)
    slice = mSlice;

  if (!slice || slice->original() != this)
    return 0;

  slice->set_embedding(mSignature);
  mSliceEmb = slice;

  return 1;
}

adjEntry Embedder::face_traverse_step(adjEntry & a, int & sign, AdjEntryArray<int> & visited, int face)
{
#if DEBUG
  printf("Face traverse at %d->%d with signature %d\n", a->theNode()->index(), a->twinNode()->index(), sign);
#endif
  
  if (sign > 0)
    visited[a] = face;

  adjEntry b = a->twin();
  sign = sign * mSignature[a->theEdge()];
  
  if (sign < 0)
    visited[b] = face;

  if (sign > 0)
    a = b->cyclicSucc();
  else
    a = b->cyclicPred();

  return a;
}

adjEntry Embedder::face_construct_step(adjEntry & a, int & sign, Face & F)
{
#if DEBUG
  printf("Face construct at %d->%d with signature %d\n", a->theNode()->index(), a->twinNode()->index(), sign);
#endif

  F.mEdges.pushBack(a->theEdge());
  if (sign > 0)
    F.mAdj.pushBack(a);
  else {
    mLeft[a] = F.id();
    mLeftFace[a] = &F;
  }

  adjEntry b = a->twin();
  sign = sign * mSignature[a->theEdge()];
  
  if (sign < 0)
    F.mAdj.pushBack(b);
  else {
    mLeft[b] = F.id();
    mLeftFace[b] = &F;
  }

  if (sign > 0)
    a = b->cyclicSucc();
  else
    a = b->cyclicPred();

  return a;
}

int Embedder::face_traverse(adjEntry a, AdjEntryArray<int> & visited, int face)
{
#if DEBUG
  printf("Traversing face %d\n", face);
#endif

  assert(face);

  int sign = 1;
  int count = 0;

  while (!visited[a] || sign < 0) {
    face_traverse_step(a, sign, visited, face);
    count++;
  }

#if DEBUG
  printf("Face %d of length %d\n", face, count);
#endif

  return count;
}

int Embedder::face_construct(adjEntry a, Face & F)
{
  F.init(this, mFaceInc[a]);

#if DEBUG
  printf("Constructing face %d\n", F.mId);
#endif

  int sign = 1;
  int count = 0;

  adjEntry first = a;
  do {
    face_construct_step(a, sign, F);
    count++;
  } while (a != first || sign < 0);

#if DEBUG
  printf("Face %d of length %d\n", F.mId, count);
#endif

  return count;
}

int Embedder::compute_faces()
{
#if DEBUG
  printf("Embedder::compute_faces\n");
#endif      
  
  if (!mSliceEmb)
    set_embedding();

  mFaceInc.init(*this, 0);

  mFaceNum = 0;
  edge e;
  forall_edges(e, *this) {
    if (!mFaceInc[e->adjSource()])
      face_traverse(e->adjSource(), mFaceInc, ++mFaceNum);
    if (!mFaceInc[e->adjTarget()])
      face_traverse(e->adjTarget(), mFaceInc, ++mFaceNum);
  }

#if DEBUG
  printf("Total %d faces\n", mFaceNum);
#endif

  forall_edges(e, *this) {
    mFaceInc[e->adjSource()]--;
    mFaceInc[e->adjTarget()]--;
  }

  return mFaceNum;
}

int Embedder::construct_faces()
{
#if DEBUG
  printf("Embedder::construct_faces\n");
#endif      

  compute_faces();

  mLeft.init(*this, 0);
  mLeftFace.init(*this, 0);
  int seen[mFaceNum];
  for (int i=0; i<mFaceNum; i++)
    seen[i] = 0;

  int f;
  edge e;
  forall_edges(e, *this) {
    f = mFaceInc[e->adjSource()];
    if (!seen[f]) {
      seen[f] = 1;
      Face F;
      face_construct(e->adjSource(), F);
      mFaces.pushBack(F);
    }
    f = mFaceInc[e->adjTarget()];
    if (!seen[f]) {
      seen[f] = 1;
      Face F;
      face_construct(e->adjTarget(), F);
      mFaces.pushBack(F);
    }
  }

  Face * F = NULL;
  forall_listiterators(Face, it, mFaces) {
    Face * G = (Face *)&(*it);
    if (F) 
      F->mNext = G;
    F = G;
  }
  return mFaceNum;
}

int Embedder::numberOfFaces()
{
  return mFaceNum;
}

int Embedder::genus()
{
  int n = numberOfNodes();
  int m = numberOfEdges();
  int f = numberOfFaces();

  int g = 2 - (n - m + f); 

#if DEBUG
  printf("Computing genus: n=%d, m=%d, f=%d => genus = %d\n", n, m, f, g);
#endif

  return g;
}

Face & Embedder::neighbor_face(Face & F, edge e) 
{
  int f = mFaceInc[e->adjSource()];
  if (f == F.id())
    return *mLeftFace[e->adjSource()];
  else
    return F;
}

int Embedder::same_face(edge e, edge f)
{
  adjEntry a = e->adjSource();
  adjEntry b = f->adjSource();

  return mFaceInc[a] == mFaceInc[b] || mLeft[a] == mLeft[b] ||
    mFaceInc[a] == mLeft[b] || mLeft[a] == mFaceInc[b];
}

int Embedder::same_face(edge e, node u)
{
  edge f;
  forall_adj_edges(f, u) {
    if (same_face(e, f))
      return 1;
  }
  return 0;
}

int Embedder::same_face(node u, node v)
{
  edge e;
  edge f;
  forall_adj_edges(e, u) {
    forall_adj_edges(f, v) 
      if (same_face(e, f))
	return 1;
  }
  return 0;
}



int Embedder::unique_emb(edge e, edge f)
{
  adjEntry a = e->adjSource();
  adjEntry b = f->adjSource();

  if (mFaceInc[a] == mLeft[a] || mFaceInc[b] == mLeft[b])
    return 0;

  if (mFaceInc[a] == mFaceInc[b] && mLeft[a] == mLeft[b] ||
      mFaceInc[a] == mLeft[b] && mLeft[a] == mFaceInc[b])
    return 0;
      
  return 1;
}


int Embedder::compute_singularities()
{
  int res = 0;
  Face * F;
  forall_emb_faces(F, *this)
    res += F->compute_singularities();

  return res;
}

int Embedder::check_embedding(int gen, int orientable)
{
  if (orientable) 
    assert(orientable_emb() == orientable);
  
  compute_faces();

  assert(genus() == gen);

  return 1;
}

int Embedder::compute_genus()
{
  compute_faces();
  
  return genus();
}

int Embedder::orientable_emb()
{
  edge e;
  forall_edges(e, *this) {
    if (mSignature[e] != 1)
      return -1;
  }
  return 1;
}

//---------------------- Face ---------------------------------------------

void Face::init(Embedder * G, int id)
{
  mGraph = G;
  mId = id;
  mNext = NULL;
}

int Face::compute_singularities() 
{
  EdgeArray<int> seen(*mGraph, 0);
  forall_listiterators(edge, it, mEdges) {
    edge e = *it;
    if (seen[e] == 1)
      mSingEdges.pushBack(e);
    seen[e]++;
  }

  NodeArray<int> vis(*mGraph, 0);
  forall_listiterators(edge, it, mEdges) {
    edge e = *it;
    node u = e->source();
    if (vis[u] == 2)
      mSingNodes.pushBack(u);
    vis[u]++;

    u = e->target();
    if (vis[u] == 2)
      mSingNodes.pushBack(u);
    vis[u]++;
  }

  mEdgeSingular = mSingEdges.size();
  mSingular = mSingNodes.size();
  
  return mSingular;
}
