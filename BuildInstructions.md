Instructions for building PanelDueFirmware under Windows (version 1.18 and later)
=================================================================================

1. Download and install the gcc cross-compiler. To build firmware version 1.18 and later, you need version 2017-q2-update. You can download an installer for this version from https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads.

2. Download and install Eclipse Oxygen for C/C++ Developers, from http://www.eclipse.org/downloads/eclipse-packages/.

3. In Eclipse create new workspace C:/Eclipse/PanelDue. Then exit Eclipse.

4. Download this github project as a zip file and unzip it into C:/Eclipse/PanelDue. Then rename folder PanelDueFirmware-dev in that folder to PanelDue.

5. Delete the bmp2c subfolder. The files in it are not part of the firmware.

6. Load Eclipse and tell it to import the PanelDue project.

7. Edit the build variable GccPath to be the path where you installed the cross-compiler.

8. Ensure there is a copy of make.exe on your PATH. Of you don't have make, try changing the build settings to "Use internal builder".

9. Build the PanelDue project

D Crocker, updated 2018-03-07.
