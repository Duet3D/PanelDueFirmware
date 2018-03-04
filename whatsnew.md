Version 1.20beta4
-----------------

Upgrade notes:
- This release is compatible with RepRapFirmware 1.20 and 1.21RC versions. It will not work with older versions of RepRapFirmware.

New and changed features:
- The first 4 macros are displayed on the Control page unless there are too many tools configured to leave room
- Macro file names can be prefixed by e.g. "1_" to control the display order, as in DWC
- Custom splash screens can be added by appending the compressed splash screen image to the end of the 'nologo' version of the binary
- The splash screen can now be cancelled by touching the display
- Larger brightness up/down steps are used
- The display dims after 1 minute of inactivity
- The "Simulating" state reported by 1.21RC3 firmware is recognised
- The babystepping increment is reduced to 0.02mm
- Czech language is supported. but not working on 4.3" build
- Characters from U_0x0100 to U+0x1FF are now supported, but not displaying correctly on 4.3" build
- Corrected German translation of "Invert display"
- "Speed" is not longer translated to German because the translation was too long to fit
- Pressing the Control button now closes popups on the Control page
- The standby bed temperature can now be set

Bug fixes:
- The extrusion factor buttons adjusted the wrong extruder

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

