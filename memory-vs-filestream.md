<pre>
Test	Input size	Memory time	Filestream time	Difference	Source
Scan	10 GiB		253.315		253.747		-0.17%		1205-filestreamsimple2.md
ScanW	5 * 10 GiB	1444.927	1474.177	-1.98%		1205-filestreamsimple2-win64.md
Sort	100 GiB		3946		3465		12.18%		1206-filestreamsimplesort.md
SortW	100 GiB		3919		2871		26.74%		1206-filestreamsimplesort-win64.md
PQ	100 GiB		10677.463	10986.353	-2.89%		1205-filestreamsimple2.md
PQW	100 GiB		14489.560					1205-filestreamsimple2-win64.md
PQW	100 GiB				7847		45.8%		1208-pq-block-size-win64.md
</pre>
