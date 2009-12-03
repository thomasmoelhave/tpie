#!/bin/bash
for i in `seq 15 15`; do 
    #A=`./test22 old $((2**$i))`
    #B=`./test22 new $((2**$i))`
    A=0
    B=0
    for j in 1; do
	A=$(( $A + `./test22 old $((2**$i))` ))
	B=$(( $B + `./test22 new $((2**$i))` ))
    done
    echo $((2**$i)) $A $B `python -c "print float($B)/float($A)"`
done