#include "interval_tree.h"

ITree::ITree(int n = 0): mSize(2*n), mA(size)
{
  
}

ITree::~ITree()
{
}


int ITree::add_down(int left, int right, int value, int p1, int p2, int pos)
{
  assert(pos < mSize);

  if (mA[pos])
    return 0;

  if (left <= p1 && right >= p2) {
    mA[pos] = value;
    return p2-p1+1;
  }

  if (left > p2 || right < p1) 
    return 0;
	
  return add_down(left, right, value, p1, (p1+p2)/2, 2*pos)
    + add_down(left, right, value, (p1+p2)/2+1, p2), 2*pos+1);
}

int ITree::add(int left, int right, int value)
{
  add_down(left, right, value, 0, mSize, 1);
}

int ITree::min_down(int x, int p1, int p2, int pos)
{
  int c = (p1+p2)/2;
  int v;
  if (x > c)
    v = min_down(c+1, p2, pos*2+1);
  else 
    v = min_down(p1, c, pos*2);

  if (!v)
    return a[pos];
}

int ITree::min(int x)
{
  min_down(x, 0, mSize, 1);
}
