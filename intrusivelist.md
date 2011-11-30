We investigate the benefits of modifying file_stream to using a single buffer
that is either used or free instead of maintaining two intrusive linked lists
of used and free buffers respectively.

We find that the intrusive lists have virtually no performance impact.

*Platform*: Linux gonzales 3.1.2-1-ARCH #1 SMP PREEMPT Tue Nov 22 09:17:56 CET
2011 x86_64 Intel(R) Core(TM) i7-2600K CPU @ 3.40GHz GenuineIntel GNU/Linux

*Branches*: filestreamspeedup-nointrusivelist 25aa536... vs.
filestreamspeedup-nointrusivelist^ 3a0d22e...

*Test used*: stream_speed_test

*Result*: Writing 64M items (512 MB or 256 blocks) to /dev/shm is just 0.8%
slower with the linked lists.

<pre>
[rav@gonzales:/dev/shm]$ ~/work/tpie/build-list/test/speed_regression/stream_speed_test 10 $((64*1024))
Writing 65536*1024 items, reading them, writing 65536 arrays, reading them
369109 112493 170908 93555
350233 111803 170776 93618
350220 111740 170883 93722
350224 111906 170895 93595
350059 115909 170783 93645
350385 112065 170794 93933
350576 112144 170786 93611
350276 111709 170739 93616
350166 111792 170985 93905
350214 111985 170971 93845
[rav@gonzales:/dev/shm]$ rm -f tmp
</pre>
<pre>
[rav@gonzales:/dev/shm]$ ~/work/tpie/build-nolist/test/speed_regression/stream_speed_test 10 $((64*1024))
Writing 65536*1024 items, reading them, writing 65536 arrays, reading them
364639 111795 170489 94338
347197 111702 170734 94093
347067 111894 170360 93875
351552 111295 170423 94072
347102 111859 170134 93993
346365 111586 170450 94117
346842 111634 170715 94070
346573 111517 170695 94265
347659 112006 170599 94208
347826 112206 170843 94577
</pre>
