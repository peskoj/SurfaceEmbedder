PATH=$PATH:~/apps/nauty24/

a=$1
b=$2

if [ -z "$b" ]; then
    echo "Range expected on input"
    exit
fi

for ((i=$a; i<=$b; i += 2)); do
    ./kubicni < graphs-$i.raw > graphs-$i.out 2> graphs-$i.log 
done;
