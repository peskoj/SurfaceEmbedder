#!/usr/bin/env python

import random
import sys

seed = float(sys.argv[1])
assert 0 <= seed <= 1

for line in sys.stdin:
    values = list(map(int, line.split()))
    n = values[0]
    m = values[1]

    x = list(range(n))
    random.shuffle(x, lambda : seed)

    for i in range(2*m):
        values[i+2] = x[values[i+2]]

    print(" ".join(map(str, values)))
