#!/usr/bin/env python

import sys
import subprocess
import getopt
import io
import xml.parsers.expat
import re

def print_tg6(g):
    template = "<graph repr=\"{0}\" label=\"{1}\" terminals=\"{2}\" />"
    print(template.format(g['repr'], g['label'], g['terminals']))

def text2tg6(text, x, y):
    c = "gconvert graph6"
    p = subprocess.Popen([c], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    out = p.communicate(bytes(text +'\n', 'UTF-8'))
    res = out[0].decode('UTF-8').strip()
    
    print_tg6({'repr': "g6", 'label': res, 'terminals': "({0}, {1})".format(x, y)})


def add_k5(n, m, edges, x, y):
    newedges = ""
    for u in [x, y, n, n+1, n+2]:
        for v in [x, y, n, n+1, n+2]:
            if u < v and (u,v) != (x,y) and (u,v) != (y,x):
                newedges += "  {0} {1}".format(u, v)

    text2tg6("{0} {1} {2} {3}".format(n+3, m+9, edges, newedges), x, y)
    text2tg6("{0} {1} {2} {3}".format(n+3, m+10, edges, "  {0} {1}".format(x, y) + newedges), x, y)

def add_k33(n, m, edges, x, y):
    newedges = ""
    for u in [x, y, n]:
        for v in [n+1, n+2, n+3]:
            newedges += "  {0} {1}".format(u, v)

    text2tg6("{0} {1} {2} {3}".format(n+4, m+9, edges, newedges), x, y)

    newedges = ""
    for u in [x, n, n+1]:
        for v in [y, n+2, n+3]:
            if (u,v) != (x,y):
                newedges += "  {0} {1}".format(u, v)

    text2tg6("{0} {1} {2} {3}".format(n+4, m+8, edges, newedges), x, y)
    text2tg6("{0} {1} {2} {3}".format(n+4, m+9, edges, "  {0} {1}".format(x, y) + newedges), x, y)

def process_graph(g):
    #print(g)
    rem = re.match("\((\d+), (\d+)\)", g['terminals'])
    assert rem
    g['x'] = int(rem.group(1))
    g['y'] = int(rem.group(2))
    c = "gconvert text"

    p = subprocess.Popen([c], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    out = p.communicate(bytes(g['label'] + '\n', 'UTF-8'))
    res = out[0].decode('UTF-8').strip()
    rem = re.match("^(\d+) (\d+) (.*)$", res)
    assert rem
    n = int(rem.group(1))
    m = int(rem.group(2))
    edges = rem.group(3)
    
    add_k5(n, m, edges, g['x'], g['y'])
    add_k33(n, m, edges, g['x'], g['y'])


def parse_tg6():
    def start_element(name, attrs):
        if name == "graph":
            process_graph(attrs)

    def end_element(name):
        pass

    def char_data(data):
        pass

    p = xml.parsers.expat.ParserCreate()

    p.StartElementHandler = start_element
    p.EndElementHandler = end_element
    p.CharacterDataHandler = char_data

    document = ''.join(sys.stdin.readlines())
    #print(document)
    p.Parse("<root>" + document + "</root>")


parse_tg6()
