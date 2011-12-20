*Platform*: bbq running Windows 64-bit

*Branches*: filestream f604a0eed3 vs. filestreampagealign f604a0eed3

filestreampagealign was patched to use a heap-allocated buffer in
file_accessor_crtp::write_header rather than stack-allocated, and to disable
file_accessor::stdio::truncate, since on Windows it relies on the old
file_accessor::write that has been removed.

*Test output*: grits:/c/Users/Rav/Desktop/pagealign-pq-segment.txt

*Test initiated* on Tue Dec 20 19:28 CET 2011

*Test used*: pq_speed_test -s -b 0.125 1 2684354560 50331648

*Block size*: 256 KiB

*Data size*: A segment_t is 6 floats, so sizeof(segment_t) is 24 bytes.
2684354560 * sizeof(segment_t) = 60 GiB. We use memory limit = 50 MiB.
