# Version 3.2.2
## Upgrade Notes:
- This release is compatible with RepRapFirmware 3.2-beta1 or later. It will partially work with RepRapFirmware 3.1.1 but not with any older version.
- Flashing this release will reset the configuration to defaults

# Limitations
- Due to the lack of RAM this build will not run on version 1 or early version 2 PanelDue boards that use the `ATSAM3S2B` chip.
    - Version 1.0, 1.1 and earlier v2 PanelDue: `ATSAM3S2B` processor (64MHz, 32kb RAM, 128kB flash) - these are the ones that the new firmware probably won't run on.
    - Version 2 PanelDue manufactured from August 2016: `ATSAM3S4B` processor (64MHz, 48kb RAM, 256kb flash). Will run the new firmware, but not as fast as version 3 will.
    - Version 3 PanelDue, including all 5i and 7i: `ATSAM4S4B` processor, 120MHz, 64kB RAM, 256kb flash. Will run the new firmware.
- There will be no reliable status updates coming from RepRapFirmware while waiting for heaters to reach temperature (and some other cases) before RRF 3.2-beta4.

## New and changed features:
* Babystep amount is configurable from preset values
* When babystep buttons are pressed the value on the screen will be updated immediately and will be overwritten
with the actual value on the next status update (usually these values should be equal anyway)
* Feedrate for move buttons is configuratble from preset values
* PanelDueFirmware does no longer distinguish between responses to `M409` with and without `K` parameter. This prevents
issues if values are moved from live to detail response or vice versa
* Provide interface for RepRapFirmware to reset as well as erase and reset the PanelDue
* Preparations to show more than one bed/one chamber

## Bug fixes:
* Babystepping amount was not initialized

## Bug fixes since 3.2.1:
* Setting babystep amount to the lowest value would invalidate stored settings on next start of PanelDue
* A chamber could steal the second slot and status/temperatures would alternate between the tool and the chamber
* A null pointer dereference could occur when RRF was restarted leading to a reset of PanelDueFirmware
* State "Initializing" was replaced too early by "Idle"
* Fetching the first bed/heater would not filter out invalidated entries
* Under rare cicumstances it could happen that selectin a bed/chamber or setting temperatures for these could
cause a restart of PanelDueFirmware


# Version 3.2.1 - Revoked
## Upgrade Notes:
- This release is compatible with RepRapFirmware 3.2-beta1 or later. It will partially work with RepRapFirmware 3.1.1 but not with any older version.
- Flashing this release will reset the configuration to defaults

# Limitations
- Due to the lack of RAM this build will not run on version 1 or early version 2 PanelDue boards that use the `ATSAM3S2B` chip.
    - Version 1.0, 1.1 and earlier v2 PanelDue: `ATSAM3S2B` processor (64MHz, 32kb RAM, 128kB flash) - these are the ones that the new firmware probably won't run on.
    - Version 2 PanelDue manufactured from August 2016: `ATSAM3S4B` processor (64MHz, 48kb RAM, 256kb flash). Will run the new firmware, but not as fast as version 3 will.
    - Version 3 PanelDue, including all 5i and 7i: `ATSAM4S4B` processor, 120MHz, 64kB RAM, 256kb flash. Will run the new firmware.
- There will be no reliable status updates coming from RepRapFirmware while waiting for heaters to reach temperature (and some other cases) before RRF 3.2-beta4.

## New and changed features:
* Babystep amount is configurable from preset values
* When babystep buttons are pressed the value on the screen will be updated immediately and will be overwritten
with the actual value on the next status update (usually these values should be equal anyway)
* Feedrate for move buttons is configuratble from preset values
* PanelDueFirmware does no longer distinguish between responses to `M409` with and without `K` parameter. This prevents
issues if values are moved from live to detail response or vice versa
* Provide interface for RepRapFirmware to reset as well as erase and reset the PanelDue
* Preparations to show more than one bed/one chamber

## Bug fixes:
* Babystepping amount was not initialized

# Version 3.2.0
## Upgrade Notes:
- This release is compatible with RepRapFirmware 3.2-beta1 or later. It will partially work with RepRapFirmware 3.1.1 but not with any older version.
- Flashing this release will reset the configuration to defaults

## Limitations
- Due to the lack of RAM this build will not run on version 1 or early version 2 PanelDue boards that use the `ATSAM3S2B` chip.
    - Version 1.0, 1.1 and earlier v2 PanelDue: `ATSAM3S2B` processor (64MHz, 32kb RAM, 128kB flash) - these are the ones that the new firmware probably won't run on.
    - Version 2 PanelDue manufactured from August 2016: `ATSAM3S4B` processor (64MHz, 48kb RAM, 256kb flash). Will run the new firmware, but not as fast as version 3 will.
    - Version 3 PanelDue, including all 5i and 7i: `ATSAM4S4B` processor, 120MHz, 64kB RAM, 256kb flash. Will run the new firmware.
- There will be no reliable status updates coming from RepRapFirmware while waiting for heaters to reach temperature (and some other cases) before RRF 3.2-beta3.

## New and changed features:
- This release uses the RepRapFirmware ObjectModel instead of limited status responses
- Support for spindles with current RPM as well as active RPM
- Bed heater will only be shown if it is configured
- Support for chamber heaters (will only show if bed heater + number of tools `<= 6` on 5" and 7" or `<= 4` on 4.3")
- Tools and assigned heaters and extruders can be numbered arbitrarily (e.g. tool 1 can use heater 8 and extruder 2)
- A simple screensaver has been added to help preventing screen burn-in on long-lasting prints
- Tool buttons will reflect the tool status (only "Active" and anything else ("Standby" is displayed as "Off" because it is confusing otherwise)
- Prevent flickering if values did not change

### Changes within the Release Candidate iterations
- Status colors of tools were confusing (because a tool can never go to "off" after being active once) so it only shows "active" or "anything else"
- Extrusion and retraction commands are now a single line to prevent being interleaved by other commands
- Screensaver is now a popup to better interact with existing or incoming popups
- No longer fetch detailed M409 for Fans or Sensors (as they are currently not required)
- Only request null-flag for state request

## Bug fixes
- Axes will be shown as they are configured, i.e. if configured axes are XYZA then PanelDue will display them like this instead of XYZU

### Bug Fixes within the Release Candidate itereations
- In some cases setting a tool's heater would instead the heatbed temperature
- Chamber and heatbed icons did not change colors according to their state
- It was not possible to disable the chamber heater by tapping the chamber button
- Move popup buttons were none-functional
- Commands entered via on-screen-keyboard were not echoed into the text field
- Screensaver timeout select pop-up would not disappear when clicking anything but "Set" or a tab button
- Extrusion factor was not always targeting the correct extruder
- Extrusion factor controls were not hidden if amount of tools was decreased
- Bed and chamber controls were not hidden if they were removed from config
- If a JSON key path was too long it would result in an endless fetch-loop
- Message box was not closed on PanelDue if closed e.g. in DWC
- Fix some fields not being hidden correctly (possible screensaver fix)

# Version 3.2-RC4
## Upgrade Notes and Limitations
See 3.2-RC2

## Changes since RC3+1:
- Only request null-flag for state request

## Bug fixes:
- Fix some fields not being hidden correctly (possible screensaver fix)

# Version 3.2-RC3+1

## Bug fixes:
- Message box was not closed on PanelDue if closed e.g. in DWC

# Version 3.2-RC3

## Upgrade Notes and Limitations
See 3.2-RC2

## Changes since RC2
- Extrusion and retraction commands are now a single line to prevent being interleaved by other commands
- Screensaver is now a popup to better interact with existing or incoming popups
- No longer fetch detailed M409 for Fans or Sensors (as they are currently not required)

## Bug Fixes since RC2:
- Screensaver timeout select pop-up would not disappear when clicking anything but "Set" or a tab button
- Extrusion factor was not always targeting the correct extruder
- Extrusion factor controls were not hidden if amount of tools was decreased
- Bed and chamber controls were not hidden if they were removed from config
- If a JSON key path was too long it would result in an endless fetch-loop

# Version 3.2-RC2

## Upgrade Notes:
- This release is compatible with RepRapFirmware 3.2-beta1 or later. It will partially work with RepRapFirmware 3.1.1 but not with any older version.

## Limitations
Due to the lack of RAM this build will not run on version 1 or early version 2 PanelDue boards that use the `ATSAM3S2B` chip.
- Version 1.0, 1.1 and earlier v2 PanelDue: ATSAM3S2B processor (64MHz, 32kb RAM, 128kB flash) - these are the ones that the new firmware probably won't run on.
- Version 2 PanelDue manufactured from August 2016: ATSAM3S4B processor (64MHz, 48kb RAM, 256kb flash). Will run the new firmware, but not as fast as version 3 will.
- Version 3 PanelDue, including all 5i and 7i: ATSAM4S4B processor, 120MHz, 64kB RAM, 256kb flash. Will run the new firmware.

# Changes since RC1:
- Status colors of tools were confusing (because a tool can never go to "off" after being active once) so it only shows "active" or "anything else"

## Bug fixes since RC1:
- In some cases setting a tool's heater would instead the heatbed temperature
- Chamber and heatbed icons did not change colors according to their state
- It was not possible to disable the chamber heater by tapping the chamber button
- Move popup buttons were none-functional
- Commands entered via on-screen-keyboard were not echoed into the text field

# Version 3.2-RC1

## Upgrade notes:
- This release is compatible with RepRapFirmware 3.2-beta1 or later. It will partially work with RepRapFirmware 3.1.1 but not with any older version.
- Flashing this release will reset the configuration to defaults

## Limitations
- Due to the lack of RAM this build will not run on version 1 or early version 2 PanelDue boards that use the `ATSAM3S2B` chip. It  is expected to work on later
version 2 PanelDue boards using the `ATSAM3S4B` chip. If you have a PanelDue using the `ATSAM3S2B` chip, we suggest you replace the board by
a version 3 PanelDue board, which can drive your existing LCD.

## New and changed features:
- This release uses the RepRapFirmware ObjectModel instead of limited status responses
- Support for spindles with current RPM as well as active RPM
- Bed heater will only be shown if it is configured
- Support for chamber heaters (will only show if bed heater + number of tools <= 6)
- Tools and assigned heaters and extruders can be numbered arbitrarily (e.g. tool 1 can use heater 8 and extruder 2)
- A simple screensaver has been added to help preventing screen burn-in on long-lasting prints
- Tool buttons will reflect the tool status
- Prevent flickering if values did not change

## Bug fixes
- Axes will be shown as they are configured, i.e. if configured axes are XYZA then PanelDue will display them like this instead of XYZU

Version 1.20
------------

Upgrade notes:
- This release is compatible with RepRapFirmware 1.20 and 1.21RC versions. It will not work with older versions of RepRapFirmware.

New and changed features:
- The first 4 macros are displayed on the Control page unless there are too many tools configured to leave room
- Macro file names can be prefixed by e.g. "1_" to control the display order, as in DWC
- Custom splash screens can be added by appending the compressed splash screen image to the end of the 'nologo' version of the binary
- The splash screen can now be cancelled by touching the display
- Larger brightness up/down steps are used
- The display dims after 1 minute of inactivity. Dimming can be disabled.
- The "Simulating" state reported by 1.21RC3 RepRapFirmware is recognised
- The babystepping increment is reduced to 0.02mm
- Czech language is supported, but not displaying correctly on 4.3" build
- Characters from U_0x0100 to U+0x1FF are now supported, but not displaying correctly on 4.3" build
- Corrected German translation of "Invert display"
- "Speed" is not longer translated to German because the translation was too long to fit
- Pressing the Control button now closes popups on the Control page
- The standby bed temperature can now be set
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

