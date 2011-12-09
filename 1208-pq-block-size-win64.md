*Platform*: bbq running Windows 64-bit

*Branches*: filestreamsimple f259263

*Block sizes*: 64 KiB, 128 KiB, 256 KiB, 512 KiB, 1 MiB, 2 MiB

*Test output*: bbq:/c/Users/Rav/Desktop/speedtest-pq-blocksize.txt

*Test initiated* on Thu Dec 8 15:15 CET 2011

*Test used*: pq_speed_test -b $BLOCKFACTOR 1 13421772800 50331648

Results
-------

64 KiB: 4327 seconds to push, 3518 seconds to pop, 7847 seconds in total.

128 KiB: 5008 seconds to push, 3755 seconds to pop, 8764 seconds in total.

256 KiB: 6441 seconds to push, 3790 seconds to pop, 10232 seconds in total.

512 KiB: 6575 seconds to push, 3983 seconds to pop, 10559 seconds in total.

1 MiB: 8331 seconds to push, 4599 seconds to pop, 12931 seconds in total.

2 MiB: 9512 seconds to push, 5969 seconds to pop, 15481 seconds in total.

Test output
-----------

E:\speedtest\sorttest>rmdir /S /Q pqtest 

E:\speedtest\sorttest>mkdir pqtest 

E:\speedtest\sorttest>pushd pqtest 
08-12-2011 
15:15

E:\speedtest\sorttest\pqtest>C:\dev\tpie\build-fss\test\speed_regression\Release\pq_speed_test.exe -b 0.03125 1 13421772800 50331648 
Memory limit: 50331648
1 times, 13421772800 elements
Blockfact Elems Push Pop Total
0.03125 13421772800 4326948 3517878 7847233
Failed to store time estimation database: Atomic rename failed

E:\speedtest\sorttest\pqtest>popd

E:\speedtest\sorttest>rmdir /S /Q pqtest 

E:\speedtest\sorttest>mkdir pqtest 

E:\speedtest\sorttest>pushd pqtest 
08-12-2011 
17:26

E:\speedtest\sorttest\pqtest>C:\dev\tpie\build-fss\test\speed_regression\Release\pq_speed_test.exe -b 0.0625 1 13421772800 50331648 
Memory limit: 50331648
1 times, 13421772800 elements
Blockfact Elems Push Pop Total
0.0625 13421772800 5008021 3755497 8764454
Failed to store time estimation database: Atomic rename failed

E:\speedtest\sorttest\pqtest>popd

E:\speedtest\sorttest>rmdir /S /Q pqtest 

E:\speedtest\sorttest>mkdir pqtest 

E:\speedtest\sorttest>pushd pqtest 
08-12-2011 
19:52

E:\speedtest\sorttest\pqtest>C:\dev\tpie\build-fss\test\speed_regression\Release\pq_speed_test.exe -b 0.125 1 13421772800 50331648 
Memory limit: 50331648
1 times, 13421772800 elements
Blockfact Elems Push Pop Total
0.125 13421772800 6441396 3790098 10232180
Failed to store time estimation database: Atomic rename failed

E:\speedtest\sorttest\pqtest>popd

E:\speedtest\sorttest>rmdir /S /Q pqtest 

E:\speedtest\sorttest>mkdir pqtest 

E:\speedtest\sorttest>pushd pqtest 
08-12-2011 
22:42

E:\speedtest\sorttest\pqtest>C:\dev\tpie\build-fss\test\speed_regression\Release\pq_speed_test.exe -b 0.25 1 13421772800 50331648 
Memory limit: 50331648
1 times, 13421772800 elements
Blockfact Elems Push Pop Total
0.25 13421772800 6575415 3982648 10558626
Failed to store time estimation database: Atomic rename failed

E:\speedtest\sorttest\pqtest>popd

E:\speedtest\sorttest>rmdir /S /Q pqtest 

E:\speedtest\sorttest>mkdir pqtest 

E:\speedtest\sorttest>pushd pqtest 
09-12-2011 
01:38

E:\speedtest\sorttest\pqtest>C:\dev\tpie\build-fss\test\speed_regression\Release\pq_speed_test.exe -b 0.5 1 13421772800 50331648 
Memory limit: 50331648
1 times, 13421772800 elements
Blockfact Elems Push Pop Total
0.5 13421772800 8331242 4599488 12931308
Failed to store time estimation database: Atomic rename failed

E:\speedtest\sorttest\pqtest>popd

E:\speedtest\sorttest>rmdir /S /Q pqtest 

E:\speedtest\sorttest>mkdir pqtest 

E:\speedtest\sorttest>pushd pqtest 
09-12-2011 
05:14

E:\speedtest\sorttest\pqtest>C:\dev\tpie\build-fss\test\speed_regression\Release\pq_speed_test.exe -b 1 1 13421772800 50331648 
Memory limit: 50331648
1 times, 13421772800 elements
Blockfact Elems Push Pop Total
1 13421772800 9511569 5969418 15481377
Failed to store time estimation database: Atomic rename failed

E:\speedtest\sorttest\pqtest>popd
09-12-2011 
09:32
Tests done
