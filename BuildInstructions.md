# Instructions for building the Paneldue Firmware

## Building with RepRapFirmware

If you are already setup to build the latest version of the firmware, then follow these instructions to be able to build the firmware.

1. Download the code from this github project (preferably by using a git client to be able to keep it up to date) to a folder called C:/Eclipse/PanelDue.

2. Open Eclipse and ask it to import the Paneldue Project into the same workspace as the RepRapFirmware.

3. Right click RRFLibraries. Build Configurations -> Set Active -> SAM3S_4S.  

4. Right click PanelDueFirmware. Build Configurations -> Set Active -> Pick the correct line for the screen you are building for.  

5. Build the PanelDue project

## Building Standalone

1. Download and install the latest version of the [Arm Gcc Cross Compiler](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads). Ensure that you select the option at the end to install to path.

2. Download and install the latest [Eclipse IDE for C/C++](https://www.eclipse.org/downloads/packages/installer).

3. Download the latest [windows build tools](https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases/). Extract the contents to C:\windows-build-tools. Add this location to [path](https://docs.alfresco.com/4.2/tasks/fot-addpath.html).  

4. Create the folder C:\Eclipse.  

5. Download or git clone the master branch of this github project into C:\Eclipse.  

6. Download or git clone the dev branch of [RRFLibraries](https://github.com/Duet3D/RRFLibraries) into C:\Eclipse.  

7. Load Eclipse and set the workspace as C:\Eclipse.  

8. File -> Open Projects from the File System. Set the import source as C:\Eclipse and finish.  

9. Right click RRFLibraries. Build Configurations -> Set Active -> SAM3S_4S.  

10. Right click PanelDueFirmware. Build Configurations -> Set Active -> Pick the correct line for the screen you are building for.  

11. Build the PanelDue project