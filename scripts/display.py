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

def runsage(values):
    cmd = list(commands)
    cmd[0] = cmd[0].format(graph6(values))
    
    command = "\n".join(cmd)

    subprocess.call(["sage", "-c", "{0}".format(command)], stdout = sys.stdout)
    
commands = [
    'g = Graph("{0}")',
    'show(g)',
]

for line in sys.stdin:
    values = list(map(int, line.split()))
    runsage(values)
