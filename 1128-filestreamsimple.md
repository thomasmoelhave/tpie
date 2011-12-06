*Platform*: sanford running Linux 64-bit

*Branches*: filestream cf2e873e78096c47930d7afede505381a98d1b7c vs. filestreamsimple d7dade9f3077a5d933461edd25219cc12f7ee890

*Test output*: sanford:/home/rav/speedtest-filestreamsimple

*Test initiated* on Monday November 28 12:50 PM

*Tests used*: `stream_speed_test` (3.5 GB) in `/dev/shm`, `test_ami_sort` (5.12 GB) and `pq_speed_test` (35 GB) on /mnt/data2 (ext4)

*Time taken* for filestreamsimple: 5 seconds two times for `stream_speed_test`, 4 minutes for `test_ami_sort`, 45 minutes for `pq_speed_test`

stream_speed_test
-----------------
* filestreamsimple is 25% faster in write_item (2.1 s vs. 1.3 s).
* filestream is 42% faster in read_item (0.75 s vs. 1.29 s).

Output:

<pre>
Scan/filestream
Writing 460800*1024 items, reading them, writing 460800 arrays, reading them
2256885 758919 1407034 961636
2107138 752444 1479456 956967
</pre>
<pre>
Scan/filestreamsimple
Writing 460800*1024 items, reading them, writing 460800 arrays, reading them
1715 1280 1391 955
1582 1286 1389 950
</pre>

test_ami_sort
-------------
* filestream is 1.60% faster

Output:

<pre>
Sort/filestream
BTE: BTE_STREAM_IMP_UFS 32
Comparison device: Operator.
Input size: 687194767 items.
Item size: 4 bytes.
TPIE memory size: 33554432 bytes.
TPIE free memory: 31403048 Bytes2.
Generating input (687194767 random integers)...Done.
Input stream length: 687194767
Time taken: 14.33u 0.76s 29.16
TPIE free memory: 31403048 bytes.
Sorting input...Done.
Sorted stream length: 687194767
Time taken: 97.99u 9.02s 232.45
TPIE free memory: 29305680 bytes.
</pre>

<pre>
Sort/filestreamsimple
BTE: BTE_STREAM_IMP_UFS 32
Comparison device: Operator.
Input size: 687194767 items.
Item size: 4 bytes.
TPIE memory size: 33554432 bytes.
TPIE free memory: 31403128 Bytes2.
Generating input (687194767 random integers)...Done.
Input stream length: 687194767
Time taken: 14.7u 0.54s 30.01
TPIE free memory: 31403128 bytes.
Sorting input...Done.
Sorted stream length: 687194767
Time taken: 111.25u 9.45s 236.24
TPIE free memory: 29305840 bytes.
</pre>

pq_speed_test
-------------
filestreamsimple is 33.0% faster at popping, as fast at pushing, 15.26% faster overall

<pre>
PQ/filestream
Memory limit: 50331648
1 times, 4697620480 elements
Blockfact Elems Push Pop Total
1 4697620480 2004280120 966071157 3006977912
</pre>
<pre>
PQ/filestreamsimple
Memory limit: 50331648
1 times, 4697620480 elements
Blockfact Elems Push Pop Total
1 4697620480 2007710 648192 2711489
</pre>
