## main

## 1.2.0, 09/04/2025

* fix build with uthash \< 2.3.0 [bgilbert]
* explicitly fail if macOS universal build is attempted [bgilbert]
* better handling of implicit mode in dcm-dump [jcupitt]
* better handling of trailing spaces in string values [y-baba-isb]
* much faster read of files with an EOT but no FGS [pcram-techcyte]
* add `dcm_filehandle_get_frame_number()` [jcupitt]
* add DICOM catenation support [jcupitt]
* fix GCC compiler warning [bgilbert]
- fix /0 from crafted file [xz0x]

## 1.1.0, 28/3/24

* deprecate `dcm_init()` [bgilbert]
* improve memory usage [bgilbert]
* fix docs build with LLVM != 14 [bgilbert]
* improve thread safety docs [mollyclaretechcyte]
* fix a double free error and clarify docs on pointer ownership [dtatsis]
* fix a divide by zero error [voidz0r]

## 1.0.5, 9/10/23

* fix a strict aliasing issue [bgilbert]

## 1.0.4, 9/10/23

* change default buildtype to debugoptimized [jcupitt]
* fix readthedocs integration [jcupitt]

## 1.0.3, 7/10/23

* improve handling of OV and OL [jcupitt]
* better frame data print [jcupitt]
* improve ExtendedOffsetTable handling [jcupitt]

## 1.0.2, 5/10/23

* fix a crash and some error pileups [bgilbert]
* fix some return values [bgilbert]

## 1.0.1, 3/10/23

* fix MSVC compiler warnings [jcupitt]
* only use `__attribute__((constructor))` if supported [bgilbert]

## 1.0.0, 2/10/23

* first release!

