#!/usr/bin/env python

import sys
import subprocess
import getopt
import io
import xml.parsers.expat

def print_tg6(g):
    print(template.format(g['repr'], g['label'], g['terminals']))

def make_str(i, j):
    return "z" * min(i,j) + "a" + "z"*(max(i,j) - min(i,j) -1) + "a"

def test_graph(g):
    #print(g)
    (i,j) = eval(g['terminals'])
    p = subprocess.Popen(["labelg -q -f{0}".format(make_str(i,j))], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    out = p.communicate(bytes(g['label'] + '\n', 'UTF-8'))
    #print(out)
    res = out[0].decode('UTF-8').strip()
    #print(res)
    if res == g['label']:
        print_tg6(g)
    

def parse_tg6():
    def start_element(name, attrs):
        if name == "graph":
            test_graph(attrs)

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


template = "<graph repr=\"{0}\" label=\"{1}\" terminals=\"{2}\" />"

parse_tg6()

