#!/usr/bin/python

import sys
import subprocess
import getopt

def printgraph(values):
    print ' '.join(map(str,values))

def add_edge(u, v, values):
    new = list(values)
    new.append(u)
    new.append(v)
    new[1] += 1
    return new

def min_orbit(u, v, values):
    p = subprocess.Popen(["edge_orbit.py {0} {1}".format(u, v)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    g = ' '.join(map(str,values)) + '\n'
    out = p.communicate(g)
    orbit = eval(out[0].strip())
    assert((u,v) in orbit)
    return (u,v) == orbit[0]
    

def test_fixed(u, v, values):
    p = subprocess.Popen(["runmintester-opt", "-c", "-g{0}".format(genus), "-o{0}".format(orientability), '-e{0} {1}'.format(u, v)], stdout = sys.stdout, stdin=subprocess.PIPE)
    p.communicate(' '.join(map(str,values)))
    p.stdin.close()
    

def has_edge(u, v, values):
    total = len(values)
    for i in range(1, total/2):
        if (values[2*i] == u and values[2*i+1] == v) or (values[2*i] == v and values[2*i+1] == u):
            return True
    return False

genus = 1
orientability = 0

try:
    opts, rest = getopt.getopt(sys.argv[1:], "g:o:")
except getopt.GetoptError:
    print "genusplus.py -g<genus> -o<orientability>"
    exit(2)

for (opt, arg) in opts:
    if opt == "-g":
        genus = int(arg)
        assert(genus >= 0)
    elif opt == "-o":
        orientability = int(arg)
        assert(orientability >= -1 and orientability <= 1)

for line in sys.stdin:
    values = map(int, line.split())
    n = values[0]
    m = values[1]
    printgraph(values)
    sys.stdout.flush()
    for i in range(n):
        for j in range(i+1, n):
            if not has_edge(i, j, values) and min_orbit(i, j, values):
                new = add_edge(i, j, values)
                test_fixed(i, j, new)
                #test_fixed(i,j, values)
    print("\n")
    sys.stdout.flush()

