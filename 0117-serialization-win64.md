*Platform*: bbq running Window 64-bit

*Branch*: serialization bffcf31

*Test initiated* on 2013-Jan-17 13:01:51

*Tests used*: serialization_speed

Tests run with 5 GiB data (4 GiB internal memory in total; 2.8 GiB available for caching).

Write/read times:
<pre>
        serialization    file_stream
Write   45.27 s          45.34 s
Read    37.29 s          31.07 s
</pre>

Write/backwards read times:
<pre>
        serialization    file_stream
Write   46.36 s          44.83 s
Read    38.65 s          43.11 s
</pre>

## Test output

<pre>
Rav@BBQ /c/dev
$ for i in serialization_forward serialization_backward stream_forward stream_backward; do
	tpie/build64/test/speed_regression/Release/serialization_speed.exe \
		--mb 5120 --times 5 $i \
		| tee 20130117-$i.txt
done

Hostname:       bbq
Platform:       Windows 64-bit
Git branch:     refs/heads/serialization
Git commit:     bffcf31dba3368ab18abd62bdb58653c396f802e
Local time:     2013-Jan-17 13:01:51
Block size:     2048 KiB
MB:             5120
Samples:        5
sizeof(item):   16
items:          335544320
Testing serialization forward stream
            Test           Write            Read
               1           46363           35334
               2           45255           55504
               3           45224           31418
               4           44834           32728
               5           44678           31449
            mean         45270.8         37286.6
          stddev         659.080         10307.6

Local time:     2013-Jan-17 13:08:45
Testing serialization reverse stream
            Test           Write            Read
               1           46332           37954
               2           45957           38937
               3           46768           38750
               4           46378           39015
               5           46363           38594
            mean         46359.6         38650.0
          stddev         287.223         422.240

Local time:     2013-Jan-17 13:15:53
Testing forward file_stream
            Test           Write            Read
               1           44928           30154
               2           45458           29905
               3           44959           32120
               4           45770           31496
               5           45598           31699
            mean         45342.6         31074.8
          stddev         380.874         984.350

Local time:     2013-Jan-17 13:22:17
Testing reverse file_stream
            Test           Write            Read
               1           44772           42073
               2           45193           49436
               3           44631           40747
               4           44662           41277
               5           44896           42010
            mean         44830.8         43108.6
          stddev         227.657         3579.36
</pre>
