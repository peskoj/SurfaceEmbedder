#!/usr/bin/env python

import sys
import subprocess
import getopt
import io

def print_tgraph(values, i, j):
    label = tgraph6(values, i, j)
    
    print(template.format("g6", label, isomap(label, i, j)))

def graph6(values):
    p = subprocess.Popen(["gconvert graph6"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    g = ' '.join(map(str,values)) + '\n'
    #print(g)
    out = p.communicate(bytes(g, 'UTF-8'))
    #print(out)
    return out[0].decode('UTF-8').strip()


def make_str(i, j):
    return "z" * min(i,j) + "a" + "z"*(max(i,j) - min(i,j) -1) + "a"

def isomap(label, i, j):
    p = subprocess.Popen(["gconvert dreadnaut"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    out = p.communicate(bytes(label + '\n', 'UTF-8'))
    #print(out)
    res = out[0].decode('UTF-8').strip()
    cmd = res + "f=[{0},{1}]cxb".format(i,j)
    
    p = subprocess.Popen(["dreadnaut | grep \"^[0-9 ]*$\""], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    out = p.communicate(bytes(cmd, 'UTF-8'))
    #print(out)
    res = out[0].decode('UTF-8').strip()
    #print(res)
    mapping = list(map(int, res.split()))
    mi = mapping.index(i)
    mj = mapping.index(j)
    return (min(mi, mj), max(mi, mj))

def tgraph6(values, i, j):
    p = subprocess.Popen(["gconvert graph6 | labelg -q -f'{0}'".format(make_str(i,j))], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    g = ' '.join(map(str,values)) + '\n'
    #print(g)
    out = p.communicate(bytes(g, 'UTF-8'))
    #print(out)
    return out[0].decode('UTF-8').strip()


def remove_edge(u, v, values):
    total = len(values)
    new = list(values)
    for i in range(1, total // 2):
        if (values[2*i] == u and values[2*i+1] == v) or (values[2*i] == v and values[2*i+1] == u):
            new[1] -= 1
            new[2*i] = values[total-2]
            new[2*i+1] = values[total-1]
            new = new[0:total-2]
            break
    return new
    

template = "<graph repr=\"{0}\" label=\"{1}\" terminals=\"{2}\" />"

for line in sys.stdin:
    values = list(map(int, line.split()))
    #print(values)
    n = values[0]
    m = values[1]
    for i in range(n):
        for j in range(i+1, n):
            v = remove_edge(i, j, values)
            print_tgraph(v, i, j)

