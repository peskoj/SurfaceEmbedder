#!/bin/bash
tmltest.py -pT 'gconvert text | sed "s/$/  %(x)s %(y)s/" | while read n m edges; do m=$((m+1)); echo "$n $m $edges"; done'