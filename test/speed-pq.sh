#!/bin/sh

# Copyright 2011, The TPIE development team
# 
# This file is part of TPIE.
# 
# TPIE is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
# 
# TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with TPIE.  If not, see <http://www.gnu.org/licenses/>

PQITEMS=67108864
PQMEMORY=50331648
TIMES=4

while true; do

  prog='/home/rav/work/tpie/Release/test/speed_regression/pq_speed_test -b $bs $TIMES $PQITEMS $PQMEMORY'

  for bs in 0.0625 0.125 0.25 0.5 0.75 1.0; do
    echo '==============================================================================='
    echo 'BEGIN TEST'
    echo "Block size: $bs"
    echo
    mkdir testspeed
    cd testspeed
    eval "time $prog"
    cd ..
    rm -rf testspeed
    echo
    echo 'END TEST'
  done

  PQITEMS=`echo "2*$PQITEMS"|bc`
done
