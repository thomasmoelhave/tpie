*Platform*: grits running Linux 64-bit

*Branches*: memory-speedtest 692e93c vs. filestreamsimple c12060d

*Test output*: grits:/home/rav/filestreamsimple2

*Test initiated* on Mon Dec 5 13:46:42 CET 2011

*Tests used*: stream_speed_test (10 GB), test_ami_sort (100 GB), pq_speed_test (100 GB)

*Results*: scan is 0.17% slower. sort is 2.11% slower. pq/push is 10.91% faster. pq/pop is 43.31% slower. Overall, pq is 2.89% slower.

stream_speed_test
-----------------

<pre>
Scan/memory/2MB
Mon Dec  5 13:46:42 CET 2011
Writing 1310720*1024 items, reading them, writing 1310720 arrays, reading them
61613 64553 61949 65200
253315
27.55user 43.08system 4:13.97elapsed 27%CPU (0avgtext+0avgdata 15568maxresident)k
41913264inputs+41943440outputs (0major+1534minor)pagefaults 0swaps
</pre>
<pre>
Scan/filestreamsimple
Mon Dec  5 13:50:56 CET 2011
Writing/reading 1310720*1024 items (ami), writing 1310720 arrays, reading them (ami)
62003 64569 62322 64853
253747
12.60user 51.80system 4:14.13elapsed 25%CPU (0avgtext+0avgdata 15936maxresident)k
41906848inputs+41943080outputs (3major+1554minor)pagefaults 0swaps
</pre>

test_ami_sort
-------------
<pre>
Sort/memory/2MB
Mon Dec  5 13:55:11 CET 2011
BTE: BTE_STREAM_IMP_UFS 512
Comparison device: Operator.
Input size: 13421772800 items.
Item size: 4 bytes.
TPIE memory size: 33554432 bytes.
TPIE free memory: 33501736 Bytes2.
Generating input (13421772800 random integers)...Done.
Input stream length: 13421772800
Time taken: 224.41u 74.19s 317.95
TPIE free memory: 31404584 bytes.
Sorting input...Done.
Sorted stream length: 13421772800
Time taken: 3461.14u 725.68s 6122.5
TPIE free memory: 29307120 bytes.
3685.55user 801.38system 1:47:22elapsed 69%CPU (0avgtext+0avgdata 101408maxresident)k
525752136inputs+629214568outputs (6major+6449minor)pagefaults 0swaps
</pre>
<pre>
Sort/filestreamsimple
Mon Dec  5 15:42:33 CET 2011
BTE: BTE_STREAM_IMP_UFS 32
Comparison device: Operator.
Input size: 13421772800 items.
Item size: 4 bytes.
TPIE memory size: 33554432 bytes.
TPIE free memory: 31403464 Bytes2.
Generating input (13421772800 random integers)...Done.
Input stream length: 13421772800
Time taken: 503.77u 70.56s 587.74
TPIE free memory: 31403464 bytes.
Sorting input...Done.
Sorted stream length: 13596094150
Time taken: 3062.53u 800.11s 6251.95
TPIE free memory: 29306176 bytes.
3566.32user 872.13system 1:54:01elapsed 64%CPU (0avgtext+0avgdata 105440maxresident)k
672307488inputs+632264032outputs (232major+7673minor)pagefaults 0swaps
</pre>

pq_speed_test
-------------
<pre>
PQ/memory/2MB
Mon Dec  5 17:36:35 CET 2011
Memory: 50331648 available, 52428800 limit
1 times, 13421772800 elements
Blockfact Elems Push Pop Total
512 13421772800 7955377 2718287 10677463
3016.46user 2761.86system 2:57:57elapsed 54%CPU (0avgtext+0avgdata 163456maxresident)k
835853816inputs+995641016outputs (87major+11278minor)pagefaults 0swaps
</pre>
<pre>
PQ/filestreamsimple
Mon Dec  5 20:34:33 CET 2011
Memory limit: 50331648
1 times, 13421772800 elements
Blockfact Elems Push Pop Total
1 13421772800 7088082 3895689 10986353
priority_queue: Memory limit: 45mb(47185920bytes)
m_for_queue: 47185920
memory before alloc: 50277968b
Memory used by file_stream: 2097320b
2097360, 24, 40, 24, 4194688, 16384
priority_queue
	setting_k: 9
	setting_mmark: 111913
	setting_m: 1007165
memory after alloc: 17151240b
3430.05user 2843.34system 3:03:06elapsed 57%CPU (0avgtext+0avgdata 542192maxresident)k
863102712inputs+878436408outputs (166major+39071minor)pagefaults 0swaps
</pre>
