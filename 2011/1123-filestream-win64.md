*Platform*: bbq running Windows 64-bit

*Branch*: filestream 0eb2bfd

*Test output*: bbq:/c/Users/Rav/Desktop/speedtest5.txt

*Test initiated* on Nov 23, 2011 10:11 am

*Tests used*: stream_speed_test (1.6 GB), test_ami_sort (64 GB), pq_speed_test (64 GB)

*Time taken*: stream_speed_test 46 minutes, test_ami_sort 71 minutes, pq_speed_test 3 hours

Test output
-----------

<pre>
E:\speedtest>del tmp 
Could Not Find E:\speedtest\tmp
23-11-2011 
10:11

E:\speedtest>C:\dev\tpie\Release\test\speed_regression\Release\stream_speed_test.exe 1 13107200 
Writing 13107200*1024 items, reading them, writing 13107200 arrays, reading them
758025599 654424375 739952347 636244010
Failed to store time estimation database: Atomic rename failed

E:\speedtest>rmdir /S /Q sorttest 

E:\speedtest>mkdir sorttest 

E:\speedtest>pushd sorttest 
23-11-2011 
10:57

E:\speedtest\sorttest>C:\dev\tpie\Release\test\Release\test_ami_sort.exe -v -t 8589934592 
BTE: BTE_STREAM_IMP_UFS 4
Comparison device: Operator.
Input size: 8589934592 items.
Item size: 4 bytes.
TPIE memory size: 33554432 bytes.
TPIE free memory: 31403472 Bytes2.
Generating input (8589934592 random integers)...Done.
Input stream length: 8589934592
Time taken: 529.963
TPIE free memory: 31403472 bytes.
Sorting input...Done.
Sorted stream length: 8619817064
Time taken: 3718.29
TPIE free memory: 29306072 bytes.

E:\speedtest\sorttest>popd

E:\speedtest>rmdir /S /Q pqtest 

E:\speedtest>mkdir pqtest 

E:\speedtest>pushd pqtest 
23-11-2011 
12:08

E:\speedtest\pqtest>C:\dev\tpie\Release\test\speed_regression\Release\pq_speed_test.exe 1 8589934592 50331648 
Memory limit: 50331648
1 times, 8589934592 elements
Blockfact Elems Push Pop Total
1 8589934592 6739274701 3759874678 10499535604
Failed to store time estimation database: Atomic rename failed

E:\speedtest\pqtest>popd
</pre>
