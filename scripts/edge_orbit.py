#!/usr/bin/env python

import sys
import subprocess
import getopt
import io

def printgraph(values):
    print(' '.join(map(str,values)))

def graph6(values):
    p = subprocess.Popen(["gconvert graph6"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    g = ' '.join(map(str,values)) + '\n'
    #print(g)
    out = p.communicate(bytes(g, 'UTF-8'))
    #print(out)
    return out[0].decode('UTF-8').strip()

def has_edge(u, v, values):
    total = len(values)
    for i in range(1, total // 2):
        if (values[2*i] == u and values[2*i+1] == v) or (values[2*i] == v and values[2*i+1] == u):
            return True
    return False

def runsage(values):
    cmd = list(commands)
    cmd[0] = cmd[0].format(graph6(values))
    cmd[5] = cmd[5].format(edge[0], edge[1])
    if not complement:
        cmd.remove(1)
    
    command = "\n".join(cmd)
    #print(command)

    subprocess.call(["sage", "-c", "{0}".format(command)], stdout = sys.stdout)
    
commands = [
    'g = Graph("{0}")',
    'g = g.complement()',
    'h = g.line_graph(False)',
    'a = h.automorphism_group(return_group=False, orbits=True)',
    'for orbit in a:',
    '  if ({0},{1}) in orbit:',
    '    print(orbit)',
]

edge = sys.argv[1:3]
#print("Edge: ", edge)
assert(len(edge) == 2)

for line in sys.stdin:
    values = list(map(int, line.split()))
    #print(values)
    n = values[0]
    m = values[1]
    if not has_edge(edge[0], edge[1], values):
        complement = True

    runsage(values)
