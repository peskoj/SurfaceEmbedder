PATH=$PATH:~/apps/nauty24/:~/apps/nauty/nauty24/

a=$1
b=$2
np=$3

if [ -z "$b" ]; then
    echo "Range expected on input"
    exit
fi

if [ -z "$np" ]; then
    echo "Number of processes expected on input"
    exit
fi

for ((i=$a; i<=$b; i += 2)); do
    for ((p=0; p< $np; p++)); do
        cat > ogdftorus-divide-$i-$p.pbs <<EOF
#! /bin/bash
#PBS -N ogdftorus-divide-$i-$p
#PBS -q batch
#PBS -M pskoda@sfu.ca
#PBS -m bae
PATH=\$PATH:~/apps/nauty24/:~/apps/nauty/nauty24/
dir=/home/pskoda/projects/torus
cd \$dir
time geng -Ctd3D3 $i $p/$np | listg -e | sed -nf convert.sed | ./ogdftorus > div-graphs-$i-$p.out
EOF
    done;

done;
