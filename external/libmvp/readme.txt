MVP MDPP16 Firmware
===================

Section max sizes
^^^^^^^^^^^^^^^^^
 Section Sectors   Bytes
       0       0      63
       1       1   65536
       2       1   65536
       3       8  524288
       8       0    4096
       9       1   65536
      10       1   65536
      11       6  393216
      12      51 3342336

File names inside directory/zip
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Regexp for file names: "^d+.*\.bin$"
with d+ containing the section index.

Required sections: 
    *  8 - descr. text
    * 12 - firmware data
