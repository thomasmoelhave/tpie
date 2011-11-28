*Platform*: sanford running Linux 64-bit

*Branches*: filestream cf2e873e78096c47930d7afede505381a98d1b7c vs. filestreamsimple d7dade9f3077a5d933461edd25219cc12f7ee890

*Test output*: sanford:/home/rav/speedtest-filestreamsimple

*Test initiated* on Monday November 28 12:50 PM

*Tests used*: `stream_speed_test` in `/dev/shm`, `test_ami_sort` and `pq_speed_test` on /mnt/data2 (ext4)

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
