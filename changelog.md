v1.1.10
=======

* Add new 'verify-firmware' command to mvlc-mvp-updater

v1.1.9.1
========

* Add device type checks to the command line mvlc-mvp-updater tool.

v1.1.9
======

* Maintenance release: update to the latest mesytec-mvlc driver version.

v1.1.8
======

* Add workaround for certain modules when flashing via the VME interface.

v1.1.7
======

* add --debug and --trace command line switches to enable verbose logging on
  the console.

v1.1.6
======

* tweak log output of the "erase flash" operation
* fix tooltip of mvlc refresh usb devices button

v1.1.5
======

* Workaround for device type checks for MCPD-8 with serial numbers < 0012

v1.1.4
======

* Perform min firmware version check when attempting to interact with a VME
  device, not when selecting it in the UI. Allows to flash, e.g. an MDPP-32
  through an MVLC that has firmware < FW0036.

v1.1.3
======

* fix device type checks (workarounds for inconsistent naming)

v1.1.2
======

* fix possible endless loops in error paths

v1.1.1
======

* bugfixes

v1.1.0
======

* New feature: connect to an MVLC and use the MVP interface through VME.

v1.0
====

* Auto select the first serial port with a connected device instead of the
  first port in the system.

* Disable the area selection if it's not required.
