*Platform*: Linux 64-bit

*Branches*: filestream

*Test initiated* on 2012-Mar-07 17:19:40

*Tests used*: stream_speed_test and file_stream_speed_test, 50 GB

*Results*: Using a file_stream directly is just as fast as using the
ami::stream wrapper.

Output
------
<pre>
Stream speed test

Hostname:       sanford
Platform:       Linux 64-bit
Git branch:     refs/heads/filestream
Git commit:     22f8e2bfdb117bd369787a511d02560d87362689
Local time:     2012-Mar-07 17:19:40
Block size:     2048 KiB

Data (MB):      51200
Repeats:        10

Stream memory usage: 2097456
Test            Write           Read            Hash            
1               633042          488812          22392774868992  
2               639341          489640          22392774868992  
3               633751          489175          22392774868992  
4               634007          489212          22392774868992  
5               636004          490425          22392774868992  
6               637250          488471          22392774868992  
7               635346          488646          22392774868992  
8               636192          488583          22392774868992  
9               629826          488834          22392774868992  
10              635783          489505          22392774868992  
mean            635054          489130          22392774868992  
stddev          2586.06         600.276         0.000000        
End time:       2012-Mar-07 20:27:08
</pre>
<pre>
file_stream speed test

Hostname:       sanford
Platform:       Linux 64-bit
Git branch:     refs/heads/filestream
Git commit:     22f8e2bfdb117bd369787a511d02560d87362689
Local time:     2012-Mar-07 20:27:08
Block size:     2048 KiB

Data (MB):      51200
Repeats:        10

file_stream memory usage: 2097320
Test            Write           Read            Hash            
1               640050          488713          22392774868992  
2               635411          488206          22392774868992  
3               634109          488978          22392774868992  
4               639413          489141          22392774868992  
5               640330          489512          22392774868992  
6               638629          488707          22392774868992  
7               629783          488612          22392774868992  
8               635389          489807          22392774868992  
9               632212          488581          22392774868992  
10              633875          488713          22392774868992  
mean            635920          488897          22392774868992  
stddev          3581.74         475.488         0.000000        
End time:       2012-Mar-07 23:34:42
</pre>
