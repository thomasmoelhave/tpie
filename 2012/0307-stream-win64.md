*Platform*: Soda running Windows 64-bit

*Branches*: master vs. filestream

*Test output*: Soda:/c/Users/Jakob/Desktop/tpie_test2/0307-stream.log

*Test initiated* on 2012-Mar-07 16:16:16

*Tests used*: stream_speed_test 50 GB

*Results*: Write is 7.1% slower, read is 3.5% slower.

Report
------
<pre>
Stream speed test

Hostname:       soda
Platform:       Windows 64-bit
Git branch:     refs/heads/master
Git commit:     bf2d5264b8cf934daf54108cb7d896999bfea713
Local time:     2012-Mar-07 16:16:16
Block size:     2048 KiB

Data (MB):      51200
Repeats:        10

Stream memory usage: 2097520
Test            Write           Read            Hash            
1               582200          416751          22392774868992  
2               573685          416620          22392774868992  
3               581642          417075          22392774868992  
4               576523          416772          22392774868992  
5               579168          416822          22392774868992  
6               588726          423853          22392774868992  
7               583567          416721          22392774868992  
8               577217          416792          22392774868992  
9               582782          417188          22392774868992  
10              587051          416974          22392774868992  
mean            581256          417557          22392774868992  
stddev          4696.18         2219.01         0.000000        
End time:       2012-Mar-07 19:02:49
</pre>
<pre>
Stream speed test

Hostname:       soda
Platform:       Windows 64-bit
Git branch:     refs/heads/filestream
Git commit:     22f8e2bfdb117bd369787a511d02560d87362689
Local time:     2012-Mar-07 19:02:49
Block size:     2048 KiB

Data (MB):      51200
Repeats:        10

Stream memory usage: 2097528
Test            Write           Read            Hash            
1               621891          433009          22392774868992  
2               631519          430817          22392774868992  
3               627559          433707          22392774868992  
4               620736          430816          22392774868992  
5               614696          432875          22392774868992  
6               622903          436812          22392774868992  
7               613986          433199          22392774868992  
8               623755          431646          22392774868992  
9               611920          433407          22392774868992  
10              639218          431643          22392774868992  
mean            622818          432793          22392774868992  
stddev          8406.09         1764.91         0.000000        
End time:       2012-Mar-07 21:58:50
</pre>
