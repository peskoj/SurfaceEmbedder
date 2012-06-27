#ifndef INC_EMBEDDER
#define INC_EMBEDDER

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/planarity/BoyerMyrvold.h>
#include <ogdf/graphalg/MinCostFlowReinelt.h>
#include <ogdf/basic/tuples.h>
#include <ogdf/internal/planarity/ConnectedSubgraph.h>
#include <vector>
#include <assert.h>
#include <algorithm>


#define forall_disks(D, L)  for (Disk ** D = (L).mDisks.begin(); D!=(L).mDisks.end(); ++D)
#define forall_emb_faces(F, E)  for ((F) = &(E).faces().front(); (F); (F) = (F)->next())

class Slice;

class Disk
{
 public:
  Slice * mSlice;
  Disk * mPair;
  int mId;
  node mCenter;
  int mCenterVisible;
  int mClockwise;
  int mSign;
  List<edge> mCenterEdges;
  List<adjEntry> mOrient; //disk edges with an arbitrary orientation
  NodeArray<node> mNodePairs; //pairs of nodes of disks
  NodeArray<int> mSide; //which side of a one-sided disk 
  NodeArray<node> mPrev; //previous node in mOrient
  NodeArray<adjEntry> mNext; //oriented arc from the node
  int mGroup;
  int mSymmetry;

  Disk(Slice & S, int symmetric);
  Disk(Slice & S, const Disk & D, NodeArray<node> & vCopy, EdgeArray<edge> & eCopy);
  void set_symmetry(int s);
  int id();
  int has_pair();
  Disk * pair();
  int check_symmetry(node target);
  void recompute();
  bool consistencyCheck();

  node next(node u);
  adjEntry arc(node u);
  adjEntry prev_arc(node u);
  adjEntry center_arc(node u);
  node prev(node u);
  int pairing_sign();
  int adjacent_nodes(node u, node v);
  void boundary(List<edge> & boundary, node u, node v);
  void shorter_boundary(List<edge> & boundary, node start, node end);
  int alternating(List<node> & a, node * b);
  node center();
};

class Obstruction {
 protected:
  vector<Cycle> mCycles;
  bool mValid;
 public:
  uint numCycles();
  Cycle & operator[](int);
  Cycle & cycle(int a) { return (*this)[a]; };
  Obstruction();
  virtual int value();
  bool valid() { return mValid; };
};

class DisjointK23: public Obstruction {
 private:
  void init(Slice * slice, KuratowskiSubdivision & S, int nind);
 public:
  DisjointK23(Slice * slice, KuratowskiSubdivision & S, int nind);
};

class DisjointK4: public Obstruction {
 private:
  void init(Slice * slice, KuratowskiSubdivision & S, int nind);
 public:
  DisjointK4(Slice * slice, KuratowskiSubdivision & S, int nind);
};

class DiskEars: public Obstruction {
 public:
  DiskEars(Slice * slice, Disk * disk, KuratowskiSubdivision & S, bool isK33);
  void init(Slice * slice, Disk * disk, KuratowskiSubdivision & S, bool isK33);
};

class DiskTripod: public Obstruction {
 public:
  DiskTripod(Slice * slice, Disk * disk, KuratowskiSubdivision & S, bool isK33);
  void init(Slice * slice, Disk * disk, KuratowskiSubdivision & S, bool isK33);
};


class Unsolvable: public Obstruction {
 public:
  Unsolvable() { 
#if DEBUG
    printf("Unsolvable obstruction created!\n");
#endif
  }
};

class UnknownObstruction: public Obstruction {
 public:
  UnknownObstruction() { 
#if DEBUG
    printf("Unknown obstruction created!\n");
#endif
  }
};

class Slice: public Graph
{
 private:
  Graph * mOrig;              //original graph
  NodeArray<node> mNodeOrig;  //nodes that will be identified
  EdgeArray<edge> mEdgeOrig;  //edges that will be identified

  //  EdgeArray<int> mEmbedded;   //is the edge embedded
  NodeArray< List<node> > mNodeCopies; //all copies of a node in mOrig
  EdgeArray< List<edge> > mEdgeCopies; // options how to embed an edge
  NodeArray< List<node> > mPoss; //possible attachments of a vertex

  int mDiskNum;               //number of disks
  int mSingle, mDouble;       //number of single-sided and double sided disks
  int mGenus;                 //genus available for cutting
  int mOrientable;            //orientability of the surface (+1 orientable, -1 non-orientable, 0 both)
  int mCentersVisible;
  //  Cycle mCycle;
  AdjEntryArray<Disk*> mDiskInc; //Disk on the right side of an adjacency
  NodeArray< List<Disk*> > mDiskNodes; //Disks incident to a vertex
  Array<Disk*> mDisks;        //array of disks
  int mUnselected;
  //  int mUnselectedHidden;

  edge split(edge e);
  edge split_adj(adjEntry a);
  node split_node(node u, List<adjEntry> & inc);
  node split_node(node u, List<adjEntry> & inc, int newid);
  node move_edges(node u, List<adjEntry> & inc, node v);
  adjEntry find_adj(node u, node v);
  node newNodeCopy(node u);
  node newNodeCopy(node u, int newid);
  void delNodeCopy(node u);
  edge newEdgeCopy(edge e, node u, node v);
  void delEdgeCopy(edge e);
  void moveAdjEntry(adjEntry a, node u);
  void moveAdjacencies(node v, List<adjEntry> & inc);
  void moveAdjacencies(node u, node v);
  void moveAdjacencies(node u, node v, List<adjEntry> & save);
  void copy_inc(node u, List<adjEntry> & inc);
  node identify_nodes_reversible(node u, node v, List<adjEntry> & inc);
  node identify_nodes(node u, node v);

  void compute_disk_inc();

  int add_disk(Disk * D);
  node add_center(Disk * D);
  void add_centers();
  void reveal_center(Disk * D);
  void reveal_centers();
  void remove_center(Disk * D);
  void delete_center(Disk * D);
  void remove_centers();
  Disk * is_center(node v);
  void push_disk_boundary(Disk * D, Cycle & C);
  void cycle_attachment(Disk * D, Cycle & C, List<node> & attachment);
  int count_inc(AdjEntryArray<Disk*> & inc, List<edge> & sg, node & center);
  Disk * common_disk(node u, node v);
  int disk_neighbors(List<node> a, List<node> b);

  int cut_along_twosided(Cycle & cycle);
  int cut_along_onesided(Cycle & cycle);
  int orient_cycle(edge s, EdgeArray<int> & color, List<adjEntry> & orient);
  int orient_disk(List<edge> & cycle, AdjEntryArray<Disk*> & color, Disk * D, edge start = NULL);
  void clear_orientation(Disk * D);
  void duplicate(node u, List<node> & nodes, Disk * D1, Disk * D2);
  void duplicate_cycle(Cycle & cycle, Disk * D1, Disk * D2);
  void create_one_disk(Cycle & cycle);
  void create_two_disks(Cycle & cycle);
  void cut_off_disk(Disk * D, EdgeArray<int> & disk, Disk * newD); //!!!Buggy, do not use
  void cut_off_disk_and_join(Disk * D, Disk * newD);
  void join_disks(Disk * D1, Disk * D2, node source, node target);
  void extend_poss(NodeArray<node> & copy, NodeArray<node> & move);

  //  int disjoint_ears(Disk * D, Cycle * cycles);
  int test_ears(Cycle * cycles);
  int is_planar_embedding(Cycle * cycles);
  void kuratowski_analysis(KuratowskiSubdivision & S, int isK33, vector<Obstruction *> & obstructions);
  //int construct_cycles(KuratowskiSubdivision & S, int isK33, Cycle * rcycles, int &cnum, CycleData & cycledata);
  //int choose_cycles(KuratowskiSubdivision & S, int isK33, Cycle * cycles, CycleData & cycledata);
  Obstruction * choose_obstruction(vector<Obstruction *> & obstructions);
  
  int disk_orientation(Disk * D, int withcenters = 1);
  int test_disks_orientation(int orient, Disk * A, Disk * B);
  Obstruction * is_non_orientable();
  void flip_disk(Disk * D);
  void regroup(Disk * D);

  Obstruction * noncontractible_cycles();

  void embed_edge(edge e);
  void unembed_edge(edge e);

  node disk_attachment(Disk * D, edge & e);

  Slice * restore_and_test(edge e);
  Slice * dichotomy(Disk * D, edge e);
  Slice * symmetric_dichotomy(Disk * D, edge e, edge f);
  Slice * test_dichotomy();
  Slice * test_extend_smart();
  Slice * cut_cycles(Obstruction * B);
  Slice * test_choice(node u, node v);
  Slice * test_choice_inside(node & u, node v);
  Slice * test_choices(node w);
  Slice * test_all_choices();
  Slice * test_all_disk_choices();
  int disk_choices();
  
  Slice * extend_embedding();
  Slice * embed();
  void traverse_face(int face, adjEntry a, AdjEntryArray<int> & visited, NodeArray< List<int> > & faces);

  void show_unselected();
  void hide_unselected();

  void read_slice(Graph & G);
  void print_slice();
  void print_slice_fast();
  void read_disk(Disk * D, Array<node> & nodes);
  void print_disk(Disk * D, NodeArray<int> & ind);
  void print_disk_fast(Disk * D);

  void compute_copies();
  int compute_unselected();

  void change_signature(Disk * D, EdgeArray<int> & signature, NodeArray<int> & flipped);
  void pair_disk(Disk * D, int oA, int oB, AdjEntryArray<adjEntry> & pairs);
  void sign_disk(Disk * D, AdjEntryArray<int> & signs);
  void pair_adj(adjEntry a, adjEntry b, int oA, int oB, AdjEntryArray<adjEntry> & pairs);
  void check_disk_embedding(Disk * D, int oD);

 public:
  void init(Graph & G);
  Slice();
  Slice(const Slice & slice);
  Slice(const Slice & slice, NodeArray<node> & vCopy, EdgeArray<edge> & eCopy);
  Slice(Graph & G);
  ~Slice();
  Slice * embed_in_surface(int genus, int orientable);
  Graph * original() { return mOrig; }

  void pair_disks(Disk * D, AdjEntryArray<adjEntry> & lpair, AdjEntryArray<adjEntry> & rpair, AdjEntryArray<int> & lsign, AdjEntryArray<int> & rsign);
  void set_signatures(Disk * D, EdgeArray<int> & signature, NodeArray<int> & vsign);
  void correct_disk_embedding(Disk * D);
  void set_embedding(EdgeArray<int> & signature);
  void copy_cycle(NodeArray<node> & vCopy, EdgeArray<edge> & eCopy, Cycle & C, Cycle & Copy);

  Disk * disk(edge e);
  int between_disks(edge e);
  int disk_edge(edge e);
  int disk_group(edge e);
  int incident(edge e, Disk * D);
  int incident(node v, Disk * D);

  int disk_num() { return mDiskNum; };
  bool extendedConsistencyCheck();
  bool valid_cycle(Cycle & C);

  void draw_slice(char * filename = (char *)"tmp.layout.gml");
};

class Embedder;

class Face 
{
  friend class Embedder;
 protected:
  Embedder * mGraph;
  List<adjEntry> mAdj;
  List<edge> mEdges;
  int mId;
  int mSingular;
  int mEdgeSingular;
  List<edge> mSingEdges;
  List<node> mSingNodes;
  Face * mNext;
  
  void init(Embedder * G, int id);
  int compute_singularities();
 public:
  List<adjEntry> & adj() { return mAdj; }
  List<edge> & edges() { return mEdges; }
  int id() { return mId; }
  int singular() { return mSingular; }
  int edgeSingular() { return mEdgeSingular; }
  List<edge> & singEdges() { return mSingEdges; }
  List<node> & singNodes() { return mSingNodes; }
  Face * next() { return mNext; }
};

class Embedder: public Graph
{
  friend class Face;
 protected:
  Slice * mSlice;
  Slice * mSliceEmb;
  EdgeArray<int> mSignature;
  AdjEntryArray<int> mFaceInc;
  AdjEntryArray<int> mLeft;
  AdjEntryArray<Face*> mLeftFace;
  List<Face> mFaces;
  int mFaceNum;

  adjEntry face_traverse_step(adjEntry & a, int & sign, AdjEntryArray<int> & visited, int face);
  adjEntry face_construct_step(adjEntry & a, int & sign, Face & F);
  int face_traverse(adjEntry a, AdjEntryArray<int> & visited, int face);
  int face_construct(adjEntry a, Face & F);

 public:
  Embedder(const Graph & G, NodeArray< node > & mapNode, EdgeArray< edge > & mapEdge);
  Embedder(const Graph & G);
  Embedder(const Embedder & G);
  Embedder();
  ~Embedder();
  
  int planar();
  int embed(int genus, int orientable);
  int set_embedding(Slice * slice = 0);
  int check_embedding(int gen, int orientable);
  int orientable_emb();
  void draw_emb(char * filename = (char *)"tmp.layout.gml");
  inline EdgeArray<int> & signature() { return mSignature; }

  int genus();
  int min_genus(int maximum, int orientable);
  int compute_genus();
  int numberOfFaces();
  int compute_faces();
  int construct_faces();
  int compute_singularities();

  List<Face> & faces() { return mFaces; }
  Face & neighbor_face(Face & F, edge e);
  int same_face(edge e, edge f);
  int same_face(edge e, node u);
  int same_face(node u, node v);
  int unique_emb(edge e, edge f);
};

void print_emb(Embedder & E, int genus);

#endif
