*Platform*: Windows 64-bit

*Branches*: master d68e26 vs. wrapwrap 841d4a merged with master d68e26

*Test output* below

*Test run* on February 2nd, 2012

*Tests used*: sort_speed_test 10000 MB data, 700 MB memory

*Conclusion*: Wrapped ami::stream is significantly 3% faster than raw ami::stream!

Results
-------
<pre>
C:\Users\jakobt\Desktop>test.bat 2> log9

C:\Users\jakobt\Desktop>echo OFF
"=====================> filestream <======================================="
The type of the file system is NTFS.
Enter current volume label for drive E:
WARNING, ALL DATA ON NON-REMOVABLE DISK
DRIVE E: WILL BE LOST!
Proceed with Format (Y/N)? QuickFormatting 1907737M
Creating file system structures.
Format complete.
       1,8 TB total disk space.
       1,8 TB are available.
Writin and sorting 10000mb
      Test     Write      Sort
         1     67108    369392
         2     67470    366537
         3     67548    365539
         4     67704    365133
      mean   67457.5    366650
    stddev   252.494   1920.67
"=====================> master <======================================="
The type of the file system is NTFS.
Enter current volume label for drive E:
WARNING, ALL DATA ON NON-REMOVABLE DISK
DRIVE E: WILL BE LOST!
Proceed with Format (Y/N)? QuickFormatting 1907737M
Creating file system structures.
Format complete.
       1,8 TB total disk space.
       1,8 TB are available.
Writin and sorting 10000mb
      Test     Write      Sort
         1     67626    375164
         2     67657    373682
         3     67548    374493
         4     67735    372325
      mean   67641.5    373916
    stddev   77.3843   1221.54
</pre>
