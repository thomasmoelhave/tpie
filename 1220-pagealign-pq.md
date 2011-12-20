*Platform*: grits running Linux 64-bit

*Branches*: filestream f604a0eed3 vs. filestreampagealign f604a0eed3

*Test output*: grits:/home/rav/pagealign-pq-segment_t

*Test initiated* on Tue Dec 20 19:06:47 CET 2011 (ETA 20:41 CET)

*Test used*: pq_speed_test -s -b 0.125 1 2684354560 50331648

*Block size*: 256 KiB

*Data size*: A segment_t is 6 floats, so sizeof(segment_t) is 24 bytes.
2684354560 * sizeof(segment_t) = 60 GiB. We use memory limit = 50 MiB.
