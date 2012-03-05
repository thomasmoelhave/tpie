*Platform*: bbq running Windows 64-bit

*Branches*: memory-speedtest 2 MB 692e93c vs. filestreamsimple c12060d (note: not page aligned)

*Test output*: bbq:/c/Users/Rav/Desktop/speedtest6.txt

*Test initiated* on Mon Dec 5 13:44 CET 2011

*Tests used*: stream_speed_test (10 GB), test_ami_sort (100 GB), pq_speed_test (100 GB)

*Results*: scan is 2% slower. sort is 6.67% slower. pq/push is 4.48% faster. pq/pop is 43.69% slower. Overall, pq is 9.27% slower.

Test output
-----------

<pre>
E:\speedtest>del tmp 
Could Not Find E:\speedtest\tmp
05-12-2011 
13:44

E:\speedtest>C:\dev\tpie\build-memory\test\speed_regression\Release\stream_speed_test_2mb.exe 5 1310720 
Writing 1310720*1024 items, reading them, writing 1310720 arrays, reading them
71317 67498 82534 68874
70980 65270 83678 67470
71229 65722 83366 66908
71385 64864 83678 66814
71182 71479 83085 67594
1444927
Failed to store time estimation database: Atomic rename failed

E:\speedtest>del tmp 
Could Not Find E:\speedtest\tmp
05-12-2011 
14:08

E:\speedtest>C:\dev\tpie\build-fss\test\speed_regression\Release\stream_speed_test.exe 5 1310720 
Writing/reading 1310720*1024 items (ami), writing 1310720 arrays, reading them (ami)
73242 67345 84988 68484
73210 69966 85534 69638
73990 65988 84692 68562
73242 71713 84224 68842
73288 63211 85784 68234
1474177
scale=10;10000-14449270000/1474177 0
198
Failed to store time estimation database: Atomic rename failed

E:\speedtest>rmdir /S /Q sorttest 

E:\speedtest>mkdir sorttest 

E:\speedtest>pushd sorttest 
05-12-2011 
14:33

E:\speedtest\sorttest>C:\dev\tpie\build-memory\test\Release\test_ami_sort_2mb.exe -v -t 13421772800 
BTE: BTE_STREAM_IMP_STDIO BTE_STREAM_IMP_UFS 32
Comparison device: Operator.
Input size: 13421772800 items.
Item size: 4 bytes.
TPIE memory size: 33554432 bytes.
TPIE free memory: 33501696 Bytes2.
Generating input (13421772800 random integers)...Done.
Input stream length: 13421772800
Time taken: 844.631
TPIE free memory: 31404544 bytes.
Sorting input...Done.
Sorted stream length: 13421772800
Time taken: 5609.14
TPIE free memory: 29307024 bytes.

E:\speedtest\sorttest>popd

E:\speedtest>rmdir /S /Q sorttest 

E:\speedtest>mkdir sorttest 

E:\speedtest>pushd sorttest 
05-12-2011 
16:21

E:\speedtest\sorttest>C:\dev\tpie\build-fss\test\Release\test_ami_sort.exe -v -t 13421772800 
BTE: BTE_STREAM_IMP_UFS 4
Comparison device: Operator.
Input size: 13421772800 items.
Item size: 4 bytes.
TPIE memory size: 33554432 bytes.
TPIE free memory: 31403552 Bytes2.
Generating input (13421772800 random integers)...Done.
Input stream length: 13421772800
Time taken: 812.308
TPIE free memory: 31403552 bytes.
Sorting input...Done.
Sorted stream length: 13596093100
Time taken: 5983.3
TPIE free memory: 29306232 bytes.

E:\speedtest\sorttest>popd

E:\speedtest>rmdir /S /Q pqtest 

E:\speedtest>mkdir pqtest 

E:\speedtest>pushd pqtest 
05-12-2011 
18:14

E:\speedtest\pqtest>C:\dev\tpie\build-memory\test\speed_regression\Release\pq_speed_test_2mb.exe 1 13421772800 50331648 
Memory: 50331648 available, 52428800 limit
1 times, 13421772800 elements
Blockfact Elems Push Pop Total
32 13421772800 10352971 4136215 14489560
Failed to store time estimation database: Atomic rename failed

E:\speedtest\pqtest>popd

E:\speedtest>rmdir /S /Q pqtest 

E:\speedtest>mkdir pqtest 

E:\speedtest>pushd pqtest 
05-12-2011 
22:15

E:\speedtest\pqtest>C:\dev\tpie\build-fss\test\speed_regression\Release\pq_speed_test.exe 1 13421772800 50331648 
Memory limit: 50331648
1 times, 13421772800 elements
Blockfact Elems Push Pop Total
1 13421772800 9889604 5943553 15833485
Failed to store time estimation database: Atomic rename failed

E:\speedtest\pqtest>popd
</pre>
