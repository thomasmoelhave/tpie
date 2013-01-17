*Platform*: sanford running Linux 64-bit

*Branch*: serialization 27c951f

*Test initiated* on 2013-Jan-17 14:32:23

*Tests used*: serialization_speed

Tests run in `/run/shm` (internal memory) with 1 GiB data.

Write/backwards read times for serialization stream improved from 4037, 960 ms
to 1999, 981 ms and again to 1771, 978 ms.

## Test output

<pre>
rav@sanford:~/tpie/build$ rm -f /run/shm/*; TMPDIR=/run/shm test/speed_regression/serialization_speed --mb 1024 --times 20 serialization_backward
Hostname:       sanford
Platform:       Linux 64-bit
Git branch:     refs/heads/serialization
Git commit:     27c951fd6756c68af931f71c85c7b7838eafb81a
Local time:     2013-Jan-17 14:32:23
Block size:     2048 KiB
MB:             1024
Samples:        20
sizeof(item):   16
items:          67108864
Testing serialization reverse stream
            Test           Write            Read
               1            1793             978
               2            1808             981
               3            1767             980
               4            1769             977
               5            1768             978
               6            1767             978
               7            1767             977
               8            1769             977
               9            1767             978
              10            1767             977
              11            1770             979
              12            1767             978
              13            1768             979
              14            1768             977
              15            1769             978
              16            1768             976
              17            1768             979
              18            1768             981
              19            1767             979
              20            1771             980
            mean         1771.30         978.350
          stddev         10.3370         1.38697
</pre>
