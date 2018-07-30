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

Instructions for building PanelDueFirmware under Linux (version 1.18 and later)
=================================================================================

1. Download and install the gcc cross-compiler.  To build firmware version 1.18 and later,
you need version 2017-q2-update or later.  Many distributions provide
pre-built packages for arm-none-eabi and if recent enough,  they should work.
If not, you can download a tarball from https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads.
Either way, ensure that arm-none-eabi-g++, et. al. are in yor PATH.  If you extracted
gcc-arm-none-eabi-7-2017-q4-major.tar.bz2 in /opt, make sure that 
/opt/gcc-arm-none-eabi-7-2017-q4-major/arm-none-eabi/bin is in your path.

2. If you don't already have a recent Eclipse installed, download and install Eclipse
Photon for C/C++ Developers, from http://www.eclipse.org/downloads/eclipse-packages/.

3. In Eclipse create new workspace in a location of your choice.  Example: ~/PanelDue

4. Download this github project as a zip file and unzip it into ~/PanelDue.
Then rename folder PanelDueFirmware-master in that folder to PanelDueFirmware.
You can also just clone the repo it if you have git installed.  No need to rename the folder
in this case. 

5. Load Eclipse and tell it to import the PanelDueFirmware project.

6. After the import, right click on the Tools directory, select Properties then
C/C++ Build.  Check "Exclude resource from build".  This is a Windows-only tool
for converting bitmap files for use as splash screens.  It won't build on Linux.

7. In Project - Build Configurations - Set Active, select the configuration that matches
the panel you're building for. 

8. Build the PanelDueFirmware project

NOTE:  If you nose around the project settings, you may find that GccPath is set to a Windows
path.  This will be ignored as long as you have the cross compiler in your 
environment's PATH.

G Joseph, updated 2018-07-30.
