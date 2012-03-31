#!/usr/bin/python

import sys

def printgraph(values):
    print ' '.join(map(str,values))


def addk5(v, values):
    new = list(values)
    n = new[0]
    vert = [v, n, n+1, n+2, n+3]
    for i in vert:
        for j in vert:
            if i < j:
                new.append(i)
                new.append(j)
    new[0] += 4
    new[1] += 10
    printgraph(new)
    
def addk33(v, values):
    new = list(values)
    n = new[0]
    A = [v, n, n+1]
    B = [n+2, n+3, n+4]
    for i in A:
        for j in B:
            new.append(i)
            new.append(j)
    new[0] += 5
    new[1] += 9
    printgraph(new)
    

for line in sys.stdin:
    values = map(int, line.split())
    n = values[0]
    m = values[1]
    for i in range(n):
        addk5(i, values)
        addk33(i, values)
    
