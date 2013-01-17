*Platform*: sanford running Linux 64-bit

*Branch*: serialization ba22347

*Test initiated* on 2013-Jan-17 13:57:34

*Tests used*: serialization_speed

Tests run in `/run/shm` (internal memory) with 1 GiB data.

Write/backwards read times for serialization stream improved from 4037, 960 ms to 1999, 981 ms.

## Test output

<pre>
rav@sanford:~/tpie/build$ rm -f /run/shm/*; TMPDIR=/run/shm test/speed_regression/serialization_speed --mb 1024 --times 20 serialization_backward
Hostname:       sanford
Platform:       Linux 64-bit
Git branch:     refs/heads/serialization
Git commit:     ba22347d59129162484e67a2cb94951feee62945
Local time:     2013-Jan-17 13:57:34
Block size:     2048 KiB
MB:             1024
Samples:        20
sizeof(item):   16
items:          67108864
Testing serialization reverse stream
            Test           Write            Read
               1            2013             981
               2            1996             981
               3            1996             982
               4            1998             981
               5            2007             981
               6            1998             982
               7            1998             981
               8            1998             981
               9            2000             981
              10            1999             981
              11            1999             981
              12            1998             982
              13            1999             981
              14            1998             983
              15            1998             981
              16            2000             982
              17            1999             981
              18            1998             982
              19            1998             981
              20            1997             982
            mean         1999.35         981.400
          stddev         3.91051        0.598243
</pre>
