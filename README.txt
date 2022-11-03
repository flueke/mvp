Mesytec MVP Programming Tool
================================================================================

Firmware update utility for newer generation mesytec VME modules. Currently supports the
following modules: MDPP-16, MDPP-32, VMMR, MVLC.

* Website: https://mesytec.com/downloads/firmware%20updates/mvp
* Github: https://github.com/flueke/mvp

* Build requirements:
  - cmake
  - Qt5
  - Quazip: https://github.com/stachenov/quazip
  - Boost >= 1.58

* Building:
    mkdir build
    cd build
    cmake ../mvp
    make
