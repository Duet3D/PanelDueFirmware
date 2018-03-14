1. To get the latest release binaries go to:

https://github.com/dc42/PanelDueFirmware/releases

2. Instructions for flashing the firmware binary via USB are at:

https://miscsolutions.wordpress.com/paneldue/

(see section "Testing the board and updating the firmware").

3. Customising the splash screen

If you wish to display a custom splash screen when PanelDue is powered up:
- First export the image you want to display in 24-bit bitmap (.bmp) format. The width and height in pixels must match exactly the resolution of the TFT panel (480x272 for the 4.3" panel, or 800x480 for the 5" and 7" panels)
- The image must compress sufficiently well to fit in the available flash memory. Images containing large blocks of the same colour compress well.
- Version 1 PanelDue controllers have 128kb flash memory. Version 2 controllers use either a ATSAM3S2B (128kb) chip or a ATSAM3S4B (256kb) chip. Version 3 controllers and the 7i integrated version have 256kb flash memory. If you have a 128kb chip then you will only be able to use a splash screen if you are using the 4.3" panel and the image compresses well.

To add the splash screen:
- Run this Windows command to compress the image: bmp2c-escher3d.exe myimage.bmp myimage.bin /b /c
- Run this Windows command to append it to the binary: copy /b PanelDue-v3-5.0-nologo.bin+myimage.bin PanelDueFirmware.bin

substituting appropriate filenames. Then check that the resulting firmware file (PanelDueFirmware.bin in this example) is no olarher than the flash memory size.
