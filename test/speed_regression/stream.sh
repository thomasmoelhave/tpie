#!/bin/bash
d=$(pwd)
rm -rf stream.dat
cp stream.cpp stream.cpp~ 
branch=$(git branch -a | sed -nre 's/[*] //p')
for rev in 0 1 2 3 4 HEAD; do
    cd $d
    rrev=$rev
    tag=stream_speed_regression_test_point_$rev
    if [[ $rev == "HEAD" ]]; then
	rrev=100000
	tag=$branch
    fi
    sed -re "s/\#define REV .*/\#define REV $rrev/" -i stream.cpp
    if cd ../../ && git checkout $tag -f tpie/ && cd tpie && make clean && make && cd $d && make clean && make stream CFLAGS="-DREV=$rev"; then
	cd $d
	printf "$rrev" | tee -a stream.dat
	rm -rf tmp
	./stream | tee -a stream.dat
    else
	cd $d
	echo "$rev Build failed" | tee -a stream.dat
    fi
done
mv stream.cpp~ stream.cpp
gnuplot stream.gp
if which okular >/dev/null; then
    okular stream.ps
elif which evince >/dev/null; then
    evince stream.ps
elif which gv >/dev/null; then
    gv stream.ps
fi