Instructions for building PanelDueFirmware under Windows
========================================================

1. Download and install the gcc cross-compiler. A simple way of doing this is to download Arduino version 1.5.8 and install it into folder C:/Arduino-1.5.8. The compiler and associated tools will then be in folder C:\Arduino-1.5.8\hardware\tools\gcc-arm-none-eabi-4.8.3-2014q1\bin. If you already have a later version of Arduino installed including the add-on for SAM processors, you will find the compiler and tools in a different folder, for example C:\Users\<YOUR USER NAME>\AppData\Local\Arduino15\packages\arduino\tools\arm-none-eabi-gcc\4.8.3-2014q1\bin.

2. Download and install Eclipse Neon 2.

3. In Eclipse create new workspace C:/Eclipse/PanelDue. Then exit Eclipse.

4. Download this github project as a zip file and unzip it into C:/Eclipse/PanelDue. Then rename folder PanelDueFirmware-dev in that folder to PanelDue.

5. Load Eclipse and tell it to import the PanelDue project.

6. If your compiler and tools are in a folder other than C:\Arduino-1.5.8\hardware\tools\gcc-arm-none-eabi-4.8.3-2014q1\bin, configure the path to the tools in both projects. You will find this in the project settings under C/C++ Build -> Settings -> Cross Settings.

7. Ensure there is a copy of make.exe on your PATH. If you installed Arduino 1.5.8 into C:/Arduino-1.5.8 then there will be one in C:\Arduino-1.5.8\hardware\arduino\sam\system\CMSIS\Examples\cmsis_example\gcc_arm.

8. Build PanelDue.

D Crocker, updated 2017-03-23.
