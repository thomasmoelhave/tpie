*Platform*: grits running Linux 64-bit

*Branch*: filestream 81c3310

*Test output*: grits:/home/rav/blocksize-pq-segment_t

*Test initiated* on Mon Dec 12 10:59:02 CET 2011

*Test used*: pq_speed_test -s -b $bs 1 2684354560 50331648

*Block sizes tested*: 32 KiB, 64 KiB, 128 KiB, 256 KiB, 512 KiB, 1 MiB, 2 MiB

A segment_t is 6 floats, so sizeof(segment_t) is 24 bytes. 2684354560 *
sizeof(segment_t) = 60 GiB. We use memory limit = 50 MiB.

Results
-------

<pre>
Block size   Elements      Time to push   Time to pop    Total time
    32 KiB   2684354560    6429.733       1117.636       7550.111
    64 KiB   2684354560    4243.428       1580.546       5826.226
   128 KiB   2684354560    2305.234       1090.496       3397.735
   256 KiB   2684354560    2354.499        552.321       2909.788
   512 KiB   2684354560    2490.050        722.817       3214.670
     1 MiB   2684354560    2482.062       1134.584       3618.356
     2 MiB   2684354560    4100.139       1287.367       5390.122
</pre>
