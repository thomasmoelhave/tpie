#!/bin/bash
d=$(pwd)
rm -rf stream.dat
cp stream.cpp stream.cpp~ 
for rev in 1644 1700 1800 1875 HEAD; do
    cd $d
    rrev=$rev
    if [[ $rev == "HEAD" ]]; then
	rrev=100000
    fi
    sed -re "s/\#define REV .*/\#define REV $rrev/" -i stream.cpp
    if cd ../../tpie/ && svn up -r "$rev" && make clean && make && cd $d && make clean && make stream CFLAGS="-DREV=$rev"; then
		cd ../../tpie/
		R=$(svn info | sed -nre 's/Revision: //p')
		cd $d
		printf "$R" | tee -a stream.dat
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