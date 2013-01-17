*Platform*: sanford running Linux 64-bit

*Branch*: serialization bffcf31

*Test initiated* on 2013-Jan-17 13:29:55

*Tests used*: serialization_speed

Tests run in `/run/shm` (internal memory) with 1 GiB data.

Write/read times:
<pre>
        serialization    file_stream
Write   695 ms           425 ms
Read    617 ms           259 ms
</pre>

Write/backwards read times:
<pre>
        serialization    file_stream
Write   4037 ms          422 ms
Read    960 ms           1061 ms
</pre>

## Test output

<pre>
rav@sanford:~/tpie/build$
export TMPDIR=/run/shm
for i in serialization_forward serialization_backward stream_forward stream_backward; do
	test/speed_regression/serialization_speed \
	       --mb 1024 --times 20 $i \
	       | tee 20130117-$i.txt \
	       && rm /run/shm/*
done

Hostname:       sanford
Platform:       Linux 64-bit
Git branch:     refs/heads/serialization
Git commit:     bffcf31dba3368ab18abd62bdb58653c396f802e
Local time:     2013-Jan-17 13:29:55
Block size:     2048 KiB
MB:             1024
Samples:        20
sizeof(item):   16
items:          67108864
Testing serialization forward stream
            Test           Write            Read
               1             726             617
               2             692             615
               3             691             616
               4             693             615
               5             693             615
               6             691             623
               7             699             615
               8             692             615
               9             692             617
              10             692             623
              11             695             615
              12             695             616
              13             694             618
              14             693             623
              15             703             624
              16             701             616
              17             692             616
              18             693             617
              19             691             616
              20             697             616
            mean         695.750         617.400
          stddev         7.87985         3.11870

Local time:     2013-Jan-17 13:30:23
Testing serialization reverse stream
            Test           Write            Read
               1            4047             958
               2            4019             959
               3            4028             959
               4            4024             958
               5            4022             958
               6            4022             958
               7            4022             958
               8            4020             959
               9            4018             958
              10            4021             959
              11            4018             959
              12            4025             959
              13            4024             960
              14            4021             964
              15            4019             959
              16            4019             959
              17            4020             959
              18            4022             959
              19            4113             972
              20            4213             972
            mean         4036.85         960.300
          stddev         46.5089         4.20651

Local time:     2013-Jan-17 13:32:04
Testing forward file_stream
            Test           Write            Read
               1             452             269
               2             436             272
               3             422             258
               4             425             258
               5             422             258
               6             423             259
               7             422             261
               8             423             257
               9             424             258
              10             423             258
              11             429             258
              12             422             258
              13             422             258
              14             422             258
              15             422             257
              16             425             257
              17             422             258
              18             422             258
              19             422             258
              20             422             257
            mean         425.100         259.250
          stddev         7.17378         3.97194

Local time:     2013-Jan-17 13:32:19
Testing reverse file_stream
            Test           Write            Read
               1             446            1061
               2             419            1061
               3             420            1063
               4             423            1061
               5             428            1062
               6             421            1060
               7             420            1062
               8             419            1061
               9             421            1061
              10             419            1061
              11             421            1061
              12             422            1061
              13             422            1063
              14             420            1061
              15             421            1061
              16             420            1062
              17             423            1061
              18             419            1061
              19             421            1061
              20             421            1061
            mean         422.300         1061.30
          stddev         5.93917        0.732695
</pre>
