#!/bin/bash

echo '\documentclass{article}
\usepackage{tikz}
\begin{document}
\begin{tikzpicture}[every node/.style={draw}]
\def\oval{circle}
\def\rectangle{rectangle}
\def\unit{0.05}'

cat | sed '/arrow/d;/width/d;/Creator/d;/graph/d;/directed/d;/^w/d;/^h/d' \
    | sed 's/[-]*[0-9].[0-9]*e[-]*[0-9]*/0/g' \
    | tr '\n' ' ' \
    | sed 's/node [^\]]* id \([0-9]*\) label "\([^"]*\)" x \([^ ]*\) y \([0-9\.\-e]*\) type "\([^"]*\)" ] ] /\\node[\\\5] (\1) at (\3*\\unit, \4*\\unit) {\2};\
/g' | sed 's/edge \[ source \([^ ]*\) target \([^ ]*\) type "\([^"]*\)" ] ] /\\draw (\1)--(\2);\
/g' | sed '$d'

echo '\end{tikzpicture}
\end{document}'

