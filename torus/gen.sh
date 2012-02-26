PATH=$PATH:/home/pskoda/apps/nauty/nauty24/

a=$1
b=$2

if [ -z "$b" ]; then
    echo "Range expected on input"
    exit
fi

for ((i=$a; i<=$b; i += 1)); do
    geng -Ctd3D3 $i > graphs-$i.raw
    cat graphs-$i.raw | listg -e | grep -iv 'graph' | tr '\n' ';' | sed 's/;;/\
/g' | sed 's/;/ /g;s/  / /g;s/^ //' > graphs-$i.in
    # cat graphs-$i.raw | listg -e | grep -iv 'graph' | tr '\n' ';' | sed 's/;;/\\n/g' | sed 's/;/ /g' > graphs-$i.in
done;
