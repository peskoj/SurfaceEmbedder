#!/bin/bash

echo 'graph {'

cat | sed '/arrow/d;/width/d;/Creator/d;/graph/d;/directed/d' \
    | sed 's/[-]*[0-9].[0-9]*e[-]*[0-9]*/0/g' \
    | tr '\n' ' ' \
    | sed 's/node [^\]]* id \([0-9]*\) label "\([^"]*\)" x \([^ ]*\) y \([0-9\.\-e]*\) w \([0-9\.]*\) h \([0-9\.]*\) type "\([^"]*\)" ] ] /\1 [label=\2, height=\6, width=\5, shape=\7, pos="\3, \4PIN\7"];\
/g' | sed 's/PINrectangle/\!/;s/PINoval//' \
    | sed 's/00*\([,"\!]\)/0\1/g' \
    | sed 's/edge \[ source \([^ ]*\) target \([^ ]*\) type "\([^"]*\)" ] ] /\1--\2;\
/g' | sed '$d'

echo '}'

