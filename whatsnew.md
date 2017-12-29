Version 1.20beta1
-----------------

Upgrade notes:
- This release is compatible with RepRapFirmware 1.20. It will not work with older versions of RepRapFirmware.

New and changed features:
- Added support for M291 message boxes with optional OK/Cancel buttons and Z adjustment
- Added a shifted mode to the keyboard
- Added an option to simulate printing a file
- Added support for CPLD 7" display (thanks bluesign2k)
- Filenames in GCode commands sent to the host are always quoted, so that filenames containing parentheses or semicolon can be used
- Configuration changes in the geometry etc. of the host are now picked up, normally within 10 seconds of being made

Bug fixes:
- The extrusion factor adjustment buttons always adjusted the factor for extruder 0
- Corrected colours for extrusion popup (thanks chandler767)

Internal changes:
- Use later version of gcc
- Up/down/left/right arrow icons replaced by characters to save flash memory space
- Keyboard generation reworked to save flash memory space

