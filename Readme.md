# PanelDueFirmware

This firmware runs on PanelDue touchscreens available from Duet3D Ltd. and their resellers. It is used to check the status on and control a machine running primarily RepRapFirmware though since it simply sends GCodes over serial line it can be basically used with every controller that understands the sent commands.

## Download
To get the latest release binaries go to:

https://github.com/Duet3D/PanelDueFirmware/releases

## Flashing instructions
Instructions for flashing the firmware binary via USB are at:

https://duet3d.dozuki.com/Wiki/PanelDue_Firmware_update

# Customization
## Splash Screen
Customising the splash screen (versions 1.20RC4 and later)

If you wish to display a custom splash screen when PanelDue is powered up, you need to append a compressed version of the splash screen image to the -nologo version of the PanelDue firmware appropriate to your model of PanelDue and screen size.

- First export the image you want to display in 24-bit bitmap (.bmp) format. The width and height in pixels must match exactly the resolution of the TFT panel (480x272 for the 4.3" panel, or 800x480 for the 5" and 7" panels)
- The image must compress sufficiently well to fit in the available flash memory. Images containing large blocks of the same colour compress well.
- Version 1 PanelDue controllers have 128kb flash memory. Version 2 controllers use either a `ATSAM3S2B` (128kb) chip or a `ATSAM3S4B` (256kb) chip. Version 3 controllers and the 7i integrated version have 256kb flash memory. If you have a 128kb chip then you will only be able to use a splash screen if you are using the 4.3" panel and the image compresses well.

There is a tool included in `Tools/gobmp2c/(linux|macos|win)` containing pre-compiled binaries for the three major operating systems.
It's a command-line tool and can be used like follows:

```
$ ./bmp2c --help
Usage of ./bmp2c:
  -binary
        Binary output
  -outfile string
        Output file. The default is to output to stdout. (default "-")
```

Use the `-binary` parameter to create splash screen and append it like this to the compiled firmware binary and provide the name of
the input file as the last parameter e.g.

```
bmp2c -binary -outfile mysplashscreen.bin mysplashscreen.bmp
```

*On Windows:*
Run this Windows command to append it to the binary: `copy /b PanelDue-v3-5.0-nologo.bin+myimage.bin PanelDueFirmware.bin`

*On Linux:*
Run this Linux command to append it to the binary: `cat PanelDue-v3-5.0-nologo.bin myimage.bin > PanelDueFirmware.bin`

substituting appropriate filenames. Then:
- Check that the resulting firmware file (PanelDueFirmware.bin in this example) is no larger than the flash memory size
- Install the new PanelDueFirmware.bin file on your PanelDue

## Custom Icons
The aforementioned tool can also be used to create new Icons. These also have to be provided in 24-bit bitmap format and will be
converted into a paletted representation. The palette contains 12 different colors:

| 24-bit input | 16-bit output | Note |
|---|---|---|
| 0x000000 | 0x0000 | black |
| 0xffffff | 0xffff | white - note that this will usually be rendered as transparent instead |
| 0x201c20 | 0x20e4 | dark grey  |
| 0xf8f8f8 | 0xffdf | very light grey, used to replace white |
| 0x181c18 | 0x18e3 | very dark grey  |
| 0xf0f0f0 | 0xf79e | light grey  |
| 0xc83030 | 0xc986 | red |
| 0xd06060 | 0xd30c | lighter red  |
| 0xc02018 | 0xc103 | red  |
| 0xf8e890 | 0xff52 | light yellow  |
| 0xf8fcd8 | 0xfffb | very light yellow  |
| 0x40ac48 | 0x4569 | green  |
| 0x909090 | 0x9492 | grey  |

The tool can convert multiple input files into one output file. To convert icons into a paletted representation use

```
bmp2c -outfile Icons.hpp icon1.bmp [icon2.bmp ...]
```

It will append to the output file.