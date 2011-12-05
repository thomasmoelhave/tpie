*Platform*: grits running Linux 64-bit

*Branches*: memory-speedtest 692e93c vs. filestreamsimple c12060d

*Test output*: grits:/home/rav/filestreamsimple2

*Test initiated* on Mon Dec 5 13:46:42 CET 2011

*Tests used*: stream_speed_test (10 GB), test_ami_sort (100 GB), pq_speed_test (100 GB)

stream_speed_test
-----------------

Results: No difference.

<pre>
Scan/memory/2MB
Mon Dec  5 13:46:42 CET 2011
Writing 1310720*1024 items, reading them, writing 1310720 arrays, reading them
61613 64553 61949 65200
27.55user 43.08system 4:13.97elapsed 27%CPU (0avgtext+0avgdata 15568maxresident)k
41913264inputs+41943440outputs (0major+1534minor)pagefaults 0swaps
</pre>
<pre>
Scan/filestreamsimple
Mon Dec  5 13:50:56 CET 2011
Writing/reading 1310720*1024 items (ami), writing 1310720 arrays, reading them (ami)
62003 64569 62322 64853
12.60user 51.80system 4:14.13elapsed 25%CPU (0avgtext+0avgdata 15936maxresident)k
41906848inputs+41943080outputs (3major+1554minor)pagefaults 0swaps
</pre>
