*Platform*: bbq running Windows 64-bit

*Branch*: filestream, with page alignment vs. without page alignment

*Test output*: bbq:/c/Users/Rav/Desktop/speedtest-pagealign2.txt

*Test initiated* on Tue Dec 13 23:58 CET 2011

*Test used*: pq_speed_test -b 0.03125 1 13421772800 50331648

Results
-------

With page alignment: 4426.141 to push, 3545.848 to pop, 7974.486 in total

No page alignment: 4596.567 to push, 3550.950 to pop, 8149.850 in total

2.2% faster with page alignment.
