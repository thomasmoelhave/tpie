*Platform*: grits running Linux 64-bit

*Branch*: filestream 81c3310

*Test output*: grits:/home/rav/blocksize-pq-segment_t

*Test initiated* on Mon Dec 12 10:59:02 CET 2011

*Test used*: pq_speed_test -s -b $bs 1 2684354560 50331648

*Block sizes tested*: 32 KiB, 64 KiB, 128 KiB, 256 KiB, 512 KiB, 1 MiB, 2 MiB

A segment_t is 6 floats, so sizeof(segment_t) is 24 bytes. 2684354560 *
sizeof(segment_t) = 60 GiB. We use memory limit = 50 MiB.
