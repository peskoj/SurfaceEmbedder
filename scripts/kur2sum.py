#!/usr/bin/python

import sys

def printgraph(values):
    print ' '.join(map(str,values))


def addk5m(u, v, values):
    new = list(values)
    n = new[0]
    vert = [u, v, n, n+1, n+2]
    for i in vert:
        for j in vert:
            if (i <> u or j <> v) and i < j:
                new.append(i)
                new.append(j)
    new[0] += 3
    new[1] += 9
    printgraph(new)    

def addk5(u, v, values):
    new = list(values)
    n = new[0]
    vert = [u, v, n, n+1, n+2]
    for i in vert:
        for j in vert:
            if i < j:
                new.append(i)
                new.append(j)
    new[0] += 3
    new[1] += 10
    printgraph(new)
    
def addk33m(u, v, values):
    new = list(values)
    n = new[0]
    A = [u, n, n+1]
    B = [v, n+2, n+3]
    for i in A:
        for j in B:
            if i <> u or j <> v:
                new.append(i)
                new.append(j)
    new[0] += 4
    new[1] += 8
    printgraph(new)

def addk33e(u, v, values):
    new = list(values)
    n = new[0]
    A = [u, n, n+1]
    B = [v, n+2, n+3]
    for i in A:
        for j in B:
            new.append(i)
            new.append(j)
    new[0] += 4
    new[1] += 9
    printgraph(new)

def addk33(u, v, values):
    new = list(values)
    n = new[0]
    A = [u, v, n]
    B = [n+1, n+2, n+3]
    for i in A:
        for j in B:
            new.append(i)
            new.append(j)
    new[0] += 4
    new[1] += 9
    printgraph(new)
    

def has_edge(u, v, values):
    total = len(values)
    for i in range(1, total/2):
        if (values[2*i] == u and values[2*i+1] == v) or (values[2*i] == v and values[2*i+1] == u):
            return True
    return False

for line in sys.stdin:
    values = map(int, line.split())
    n = values[0]
    m = values[1]
    for i in range(n):
        for j in range(i+1, n):
            if not has_edge(i, j, values):
                addk5(i, j, values)
                addk5m(i, j, values)
                addk33(i, j, values)
                addk33e(i, j, values)
                addk33m(i, j, values)
