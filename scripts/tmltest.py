#!/usr/bin/env python

import sys
import subprocess
import getopt
import io
import xml.parsers.expat

def print_tg6(g):
    print(template.format(g['repr'], g['label'], g['terminals']))


def test_graph(g):
    #print(g)
    p = subprocess.Popen([command], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    out = p.communicate(bytes(g['label'] + '\n', 'UTF-8'))
    #print(out)
    res = out[0].decode('UTF-8').strip()
    if res:
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


command = sys.argv[1]
print(command, file=sys.stderr)
    
template = "<graph repr=\"{0}\" label=\"{1}\" terminals=\"{2}\" />"

parse_tg6()

