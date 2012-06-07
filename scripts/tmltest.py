#!/usr/bin/env python

import sys
import subprocess
import getopt
import io
import xml.parsers.expat
import re

oPrint = 0
oTest = 1

def print_tg6(g):
    print(template.format(g['repr'], g['label'], g['terminals']))


def test_graph(g):
    #print(g)
    m = re.match("\((\d+), (\d+)\)", g['terminals'])
    assert m
    g['x'] = m.group(1)
    g['y'] = m.group(2)
    #print(g)
    c = command % g;
    #print(c)

    p = subprocess.Popen([c], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    out = p.communicate(bytes(g['label'] + '\n', 'UTF-8'))
    #print(out)
    res = out[0].decode('UTF-8').strip()
    if oTest and res:
        print_tg6(g)
    if oPrint:
        print(res)


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

import getopt
optlist, args = getopt.getopt(sys.argv[1:], 'pPtT')

for o, a in optlist:
    if o == "-p":
        oPrint = 1
    if o == "-P":
        oPrint = 0
    if o == "-t":
        oTest = 1
    if o == "-T":
        oTest = 0

if len(args) < 1:
    print("""
Usage: tmltest.py [-pPtT] <command>
""")
    sys.exit(1)

command = args[0]
print(command, file=sys.stderr)
    
template = "<graph repr=\"{0}\" label=\"{1}\" terminals=\"{2}\" />"

parse_tg6()

