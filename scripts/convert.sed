#!/usr/bin/sed -n -f
/Graph/d
/^$/{
x
s/^\n*//g
s/\n/ /g
s/  / /g
s/\([0-9]* [0-9]* \)/\1 /g
/./ p
d
}
H
${
x
s/^\n*//g
s/\n/ /g
s/  / /g
s/\([0-9]* [0-9]* \)/\1 /g
/./ p
d
}
