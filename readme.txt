MVP MDPP16 Firmware
============================================================

Section max sizes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Regexp for file names: "^d+.*\.bin$"
with d+ containing the section index.

Requirements: Input dir/zip must contain at least one valid section file.

Example:
  firmware-testdir1/
        08-description.bin
        12-mdpp16_main.bin
        9something-else.bin

  Same works with a flat zip archive (no root folder inside the zip allowed as
  of now).

Flash algo
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
No special handling for specific section numbers yet.

for each section in firmware:
    erase(section)
    write_memory(Addr(0, 0, 0), section, firmware.get_section(section))
    verify_memory(Addr(0, 0, 0), section, firmware.get_section(section))
