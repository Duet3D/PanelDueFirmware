/*
  UTFT.cpp - Arduino/chipKit library support for Color TFT LCD Boards
  Copyright (C)2010-2012 Henning Karlsen. All right reserved
  
  This library is the continuation of my ITDB02_Graph, ITDB02_Graph16
  and RGB_GLCD libraries for Arduino and chipKit. As the number of 
  supported display modules and controllers started to increase I felt 
  it was time to make a single, universal library as it will be much 
  easier to maintain in the future.

  Basic functionality of this library was originally based on the 
  demo-code provided by ITead studio (for the ITDB02 modules) and 
  NKC Electronics (for the RGB GLCD module/shield).

  This library supports a number of 8bit, 16bit and serial graphic 
  displays, and will work with both Arduino and chipKit boards. For a 
  full list of tested display modules and controllers, see the 
  document UTFT_Supported_display_modules_&_controllers.pdf.

  When using 8bit and 16bit display modules there are some 
  requirements you must adhere to. These requirements can be found 
  in the document UTFT_Requirements.pdf.
  There are no special requirements when using serial displays.

  You can always find the latest version of the library at 
  http://electronics.henningkarlsen.com/

  If you make any modifications or improvements to the code, I would 
  appreciate that you share the code with me so that I might include 
  it in the next release. I can be contacted through 
  http://electronics.henningkarlsen.com/contact.php.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "asf.h"
#include "UTFT.hpp"
#include "memorysaver.h"
#include "HW_AVR.h" 
#include <cstring>			// for strchr

template <class T> inline void swap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}

inline bool UTFT::isParallel() const
{
	return displayTransferMode >= TModeLowestParallel;
}

UTFT::UTFT(DisplayType model, TransferMode pmode, unsigned int RS, unsigned int WR, unsigned int CS, unsigned int RST, unsigned int SER_LATCH)
	: fcolour(0xFFFF), bcolour(0), transparentBackground(false),
	  displayModel(model), displayTransferMode(pmode),
	  portRS(RS), portWR(WR), portCS(CS), portRST(RST), portSDA(RS), portSCL(SER_LATCH),
	  translateFrom(NULL), translateTo(NULL),
	  numContinuationBytesLeft(0)
{ 
	switch (model)
	{
		case HX8347A:
		case SSD1289:
		case ILI9325C:
		case ILI9325D:
		case S1D19122:
		case S6D1121:
			disp_x_size=239;
			disp_y_size=319;
			break;
		case ILI9327:
		case HX8352A:
			disp_x_size=239;
			disp_y_size=399;
			break;
		case HX8340B:
		case HX8340B_S:
			disp_x_size=175;
			disp_y_size=219;
			break;
		case ST7735:
			disp_x_size=127;
			disp_y_size=159;
			break;
		case PCF8833:
			disp_x_size=127;
			disp_y_size=127;
			break;
		case SSD1963_480:
			disp_x_size=271;
			disp_y_size=479;
			break;
		case SSD1963_800:
			disp_x_size=479;
			disp_y_size=799;
			break;
	}

	_set_direction_registers();
	portRS.setMode(OneBitPort::Output);
	portWR.setMode(OneBitPort::Output);
	portCS.setMode(OneBitPort::Output);
	portRST.setMode(OneBitPort::Output);
}

inline void UTFT::LCD_Write_COM(uint8_t VL)  
{   
	setRSLow();
	LCD_Write_Bus((uint16_t)VL);
}

inline void UTFT::LCD_Write_DATA16(uint16_t VHL)
{
	setRSHigh();
	LCD_Write_Bus(VHL);
}

inline void UTFT::LCD_Write_Repeated_DATA16(uint16_t VHL, uint16_t num)
{
	setRSHigh();
	LCD_Write_Bus(VHL);
	LCD_Write_Again(num - 1);
}

void UTFT::LCD_Write_Repeated_DATA16(uint16_t VHL, uint16_t num1, uint16_t num2)
{
	while (num2 != 0)
	{
		LCD_Write_Repeated_DATA16(VHL, num1);
		--num2;
	}
}

// This one is deliberately not inlined to avoid bloating the initialization code.
// Use LCD_Write_DATA16 instead where high performance is wanted.
void UTFT::LCD_Write_DATA8(uint8_t VL)
{
	setRSHigh();
	LCD_Write_Bus((uint16_t)VL);
}

void UTFT::LCD_Write_COM_DATA16(uint8_t com1, uint16_t dat1)
{
     LCD_Write_COM(com1);
     LCD_Write_DATA16(dat1);
}

void UTFT::LCD_Write_COM_DATA8(uint8_t com1, uint8_t dat1)
{
     LCD_Write_COM(com1);
     LCD_Write_DATA8(dat1);
}

void UTFT::InitLCD(DisplayOrientation po, bool is24bit)
{
	orient = po;
	textXpos = 0;
	textYpos = 0;
	lastCharColData = 0UL;
	numContinuationBytesLeft = 0;

	removeReset();
	delay_ms(5); 
	assertReset();
	delay_ms(15);
	removeReset();
	delay_ms(15);

	assertCS();

	switch(displayModel)
	{
#ifndef DISABLE_HX8347A
	case HX8347A:
		LCD_Write_COM_DATA16(0x46,0x00A4);
		LCD_Write_COM_DATA16(0x47,0x0053);
		LCD_Write_COM_DATA16(0x48,0x0000);
		LCD_Write_COM_DATA16(0x49,0x0044);
		LCD_Write_COM_DATA16(0x4a,0x0004);
		LCD_Write_COM_DATA16(0x4b,0x0067);
		LCD_Write_COM_DATA16(0x4c,0x0033);
		LCD_Write_COM_DATA16(0x4d,0x0077);
		LCD_Write_COM_DATA16(0x4e,0x0012);
		LCD_Write_COM_DATA16(0x4f,0x004C);
		LCD_Write_COM_DATA16(0x50,0x0046);
		LCD_Write_COM_DATA16(0x51,0x0044);

		//240x320 window setting
		LCD_Write_COM_DATA16(0x02,0x0000); // Column address start2
		LCD_Write_COM_DATA16(0x03,0x0000); // Column address start1
		LCD_Write_COM_DATA16(0x04,0x0000); // Column address end2
		LCD_Write_COM_DATA16(0x05,0x00ef); // Column address end1
		LCD_Write_COM_DATA16(0x06,0x0000); // Row address start2
		LCD_Write_COM_DATA16(0x07,0x0000); // Row address start1
		LCD_Write_COM_DATA16(0x08,0x0001); // Row address end2
		LCD_Write_COM_DATA16(0x09,0x003f); // Row address end1

		// Display Setting
		LCD_Write_COM_DATA16(0x01,0x0006); // IDMON=0, INVON=1, NORON=1, PTLON=0
		LCD_Write_COM_DATA16(0x16,0x00C8); // MY=0, MX=0, MV=0, ML=1, BGR=0, TEON=0   0048
		LCD_Write_COM_DATA16(0x23,0x0095); // N_DC=1001 0101
		LCD_Write_COM_DATA16(0x24,0x0095); // PI_DC=1001 0101
		LCD_Write_COM_DATA16(0x25,0x00FF); // I_DC=1111 1111

		LCD_Write_COM_DATA16(0x27,0x0002); // N_BP=0000 0010
		LCD_Write_COM_DATA16(0x28,0x0002); // N_FP=0000 0010
		LCD_Write_COM_DATA16(0x29,0x0002); // PI_BP=0000 0010
		LCD_Write_COM_DATA16(0x2a,0x0002); // PI_FP=0000 0010
		LCD_Write_COM_DATA16(0x2C,0x0002); // I_BP=0000 0010
		LCD_Write_COM_DATA16(0x2d,0x0002); // I_FP=0000 0010

		LCD_Write_COM_DATA16(0x3a,0x0001); // N_RTN=0000, N_NW=001    0001
		LCD_Write_COM_DATA16(0x3b,0x0000); // P_RTN=0000, P_NW=001
		LCD_Write_COM_DATA16(0x3c,0x00f0); // I_RTN=1111, I_NW=000
		LCD_Write_COM_DATA16(0x3d,0x0000); // DIV=00
		delay_ms(1);
		LCD_Write_COM_DATA16(0x35,0x0038); // EQS=38h
		LCD_Write_COM_DATA16(0x36,0x0078); // EQP=78h
		LCD_Write_COM_DATA16(0x3E,0x0038); // SON=38h
		LCD_Write_COM_DATA16(0x40,0x000F); // GDON=0Fh
		LCD_Write_COM_DATA16(0x41,0x00F0); // GDOFF

		// Power Supply Setting
		LCD_Write_COM_DATA16(0x19,0x0049); // CADJ=0100, CUADJ=100, OSD_EN=1 ,60Hz
		LCD_Write_COM_DATA16(0x93,0x000F); // RADJ=1111, 100%
		delay_ms(1);
		LCD_Write_COM_DATA16(0x20,0x0040); // BT=0100
		LCD_Write_COM_DATA16(0x1D,0x0007); // VC1=111   0007
		LCD_Write_COM_DATA16(0x1E,0x0000); // VC3=000
		LCD_Write_COM_DATA16(0x1F,0x0004); // VRH=0011

		//VCOM SETTING
		LCD_Write_COM_DATA16(0x44,0x004D); // VCM=101 0000  4D
		LCD_Write_COM_DATA16(0x45,0x000E); // VDV=1 0001   0011
		delay_ms(1);
		LCD_Write_COM_DATA16(0x1C,0x0004); // AP=100
		delay_ms(2);

		LCD_Write_COM_DATA16(0x1B,0x0018); // GASENB=0, PON=0, DK=1, XDK=0, VLCD_TRI=0, STB=0
		delay_ms(1);
		LCD_Write_COM_DATA16(0x1B,0x0010); // GASENB=0, PON=1, DK=0, XDK=0, VLCD_TRI=0, STB=0
		delay_ms(1);
		LCD_Write_COM_DATA16(0x43,0x0080); //set VCOMG=1
		delay_ms(2);

		// Display ON Setting
		LCD_Write_COM_DATA16(0x90,0x007F); // SAP=0111 1111
		LCD_Write_COM_DATA16(0x26,0x0004); //GON=0, DTE=0, D=01
		delay_ms(1);
		LCD_Write_COM_DATA16(0x26,0x0024); //GON=1, DTE=0, D=01
		LCD_Write_COM_DATA16(0x26,0x002C); //GON=1, DTE=0, D=11
		delay_ms(1);
		LCD_Write_COM_DATA16(0x26,0x003C); //GON=1, DTE=1, D=11

		// INTERNAL REGISTER SETTING
		LCD_Write_COM_DATA16(0x57,0x0002); // TEST_Mode=1: into TEST mode
		LCD_Write_COM_DATA16(0x95,0x0001); // SET DISPLAY CLOCK AND PUMPING CLOCK TO SYNCHRONIZE
		LCD_Write_COM_DATA16(0x57,0x0000); // TEST_Mode=0: exit TEST mode
		//LCD_Write_COM_DATA(0x21,0x0000);
		LCD_Write_COM(0x22);   
		break;
#endif
#ifndef DISABLE_ILI9327
	case ILI9327:
		LCD_Write_COM_DATA16(0xE9, 0x0020);
		LCD_Write_COM(0x11); //Exit Sleep
		lib_delay_ms(100);
		LCD_Write_COM_DATA16(0xD1, 0x0000);
		LCD_Write_DATA16(0x0071);
		LCD_Write_DATA16(0x0019);
		LCD_Write_COM_DATA16(0xD0, 0x0007);
		LCD_Write_DATA16(0x0001);
		LCD_Write_DATA16(0x0008);
		LCD_Write_COM_DATA16(0x36, 0x0048);
		LCD_Write_COM_DATA16(0x3A, 0x0005);
		LCD_Write_COM_DATA15(0xC1, 0x0010);
		LCD_Write_DATA16(0x0010);
		LCD_Write_DATA16(0x0002);
		LCD_Write_DATA16(0x0002);
		LCD_Write_COM(0xC0); //Set Default Gamma
		LCD_Write_DATA16(0x0000);
		LCD_Write_DATA16(0x0035);
		LCD_Write_DATA16(0x0000);
		LCD_Write_DATA16(0x0000);
		LCD_Write_DATA16(0x0001);
		LCD_Write_DATA16(0x0002);
		LCD_Write_COM(0xC5); //Set frame rate
		LCD_Write_DATA16(0x0004);
		LCD_Write_COM_DATA16(0xD2, 0x0001); //power setting
		LCD_Write_DATA16(0x0044);
		LCD_Write_COM_DATA16(0xC8, 0x0004); //Set Gamma
		LCD_Write_DATA16(0x0067);
		LCD_Write_DATA16(0x0035);
		LCD_Write_DATA16(0x0004);
		LCD_Write_DATA16(0x0008);
		LCD_Write_DATA16(0x0006);
		LCD_Write_DATA16(0x0024);
		LCD_Write_DATA16(0x0001);
		LCD_Write_DATA16(0x0037);
		LCD_Write_DATA16(0x0040);
		LCD_Write_DATA16(0x0003);
		LCD_Write_DATA16(0x0010);
		LCD_Write_DATA16(0x0008);
		LCD_Write_DATA16(0x0080);
		LCD_Write_DATA16(0x0000);
		LCD_Write_COM_DATA16(0x2A, 0x0000);
		LCD_Write_DATA16(0x0000);
		LCD_Write_DATA16(0x0000);
		LCD_Write_DATA16(0x00EF);
		LCD_Write_COM_DATA16(0x2B, 0x0000);
		LCD_Write_DATA16(0x0000);
		LCD_Write_DATA16(0x0001);
		LCD_Write_DATA16(0x008F);
		LCD_Write_COM(0x29); //display on      
		LCD_Write_COM(0x2C); //display on
		break;
#endif
#ifndef DISABLE_SSD1289
	case SSD1289:
		LCD_Write_COM_DATA16(0x00,0x0001);
		LCD_Write_COM_DATA16(0x03,0xA8A4);
		LCD_Write_COM_DATA16(0x0C,0x0000);
		LCD_Write_COM_DATA16(0x0D,0x080C);
		LCD_Write_COM_DATA16(0x0E,0x2B00);
		LCD_Write_COM_DATA16(0x1E,0x00B7);
		LCD_Write_COM_DATA16(0x01,0x2B3F);
		LCD_Write_COM_DATA16(0x02,0x0600);
		LCD_Write_COM_DATA16(0x10,0x0000);
		LCD_Write_COM_DATA16(0x11,0x6070);
		LCD_Write_COM_DATA16(0x05,0x0000);
		LCD_Write_COM_DATA16(0x06,0x0000);
		LCD_Write_COM_DATA16(0x16,0xEF1C);
		LCD_Write_COM_DATA16(0x17,0x0003);
		LCD_Write_COM_DATA16(0x07,0x0233);
		LCD_Write_COM_DATA16(0x0B,0x0000);
		LCD_Write_COM_DATA16(0x0F,0x0000);
		LCD_Write_COM_DATA16(0x41,0x0000);
		LCD_Write_COM_DATA16(0x42,0x0000);
		LCD_Write_COM_DATA16(0x48,0x0000);
		LCD_Write_COM_DATA16(0x49,0x013F);
		LCD_Write_COM_DATA16(0x4A,0x0000);
		LCD_Write_COM_DATA16(0x4B,0x0000);
		LCD_Write_COM_DATA16(0x44,0xEF00);
		LCD_Write_COM_DATA16(0x45,0x0000);
		LCD_Write_COM_DATA16(0x46,0x013F);
		LCD_Write_COM_DATA16(0x30,0x0707);
		LCD_Write_COM_DATA16(0x31,0x0204);
		LCD_Write_COM_DATA16(0x32,0x0204);
		LCD_Write_COM_DATA16(0x33,0x0502);
		LCD_Write_COM_DATA16(0x34,0x0507);
		LCD_Write_COM_DATA16(0x35,0x0204);
		LCD_Write_COM_DATA16(0x36,0x0204);
		LCD_Write_COM_DATA16(0x37,0x0502);
		LCD_Write_COM_DATA16(0x3A,0x0302);
		LCD_Write_COM_DATA16(0x3B,0x0302);
		LCD_Write_COM_DATA16(0x23,0x0000);
		LCD_Write_COM_DATA16(0x24,0x0000);
		LCD_Write_COM_DATA16(0x25,0x8000);
		LCD_Write_COM_DATA16(0x4f,0x0000);
		LCD_Write_COM_DATA16(0x4e,0x0000);
		LCD_Write_COM(0x22);   
		break;
#endif
#ifndef DISABLE_ILI9325C
	case ILI9325C:
		LCD_Write_COM_DATA16(0xE5, 0x78F0); // set SRAM internal timing
		LCD_Write_COM_DATA16(0x01, 0x0100); // set Driver Output Control  
		LCD_Write_COM_DATA16(0x02, 0x0700); // set 1 line inversion  
		LCD_Write_COM_DATA16(0x03, 0x1030); // set GRAM write direction and BGR=1.  
		LCD_Write_COM_DATA16(0x04, 0x0000); // Resize register  
		LCD_Write_COM_DATA16(0x08, 0x0207); // set the back porch and front porch  
		LCD_Write_COM_DATA16(0x09, 0x0000); // set non-display area refresh cycle ISC[3:0]  
		LCD_Write_COM_DATA16(0x0A, 0x0000); // FMARK function  
		LCD_Write_COM_DATA16(0x0C, 0x0000); // RGB interface setting  
		LCD_Write_COM_DATA16(0x0D, 0x0000); // Frame marker Position  
		LCD_Write_COM_DATA16(0x0F, 0x0000); // RGB interface polarity  
		//*************Power On sequence ****************//  
		LCD_Write_COM_DATA16(0x10, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB  
		LCD_Write_COM_DATA16(0x11, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]  
		LCD_Write_COM_DATA16(0x12, 0x0000); // VREG1OUT voltage  
		LCD_Write_COM_DATA16(0x13, 0x0000); // VDV[4:0] for VCOM amplitude  
		LCD_Write_COM_DATA16(0x07, 0x0001);  
		delay_ms(200); // Dis-charge capacitor power voltage  
		LCD_Write_COM_DATA16(0x10, 0x1090); // SAP, BT[3:0], AP, DSTB, SLP, STB  
		LCD_Write_COM_DATA16(0x11, 0x0227); // Set DC1[2:0], DC0[2:0], VC[2:0]  
		delay_ms(50); // Delay 50ms  
		LCD_Write_COM_DATA16(0x12, 0x001F); // 0012  
		delay_ms(50); // Delay 50ms  
		LCD_Write_COM_DATA16(0x13, 0x1500); // VDV[4:0] for VCOM amplitude  
		LCD_Write_COM_DATA16(0x29, 0x0027); // 04  VCM[5:0] for VCOMH  
		LCD_Write_COM_DATA16(0x2B, 0x000D); // Set Frame Rate  
		delay_ms(50); // Delay 50ms  
		LCD_Write_COM_DATA16(0x20, 0x0000); // GRAM horizontal Address  
		LCD_Write_COM_DATA16(0x21, 0x0000); // GRAM Vertical Address  
		// ----------- Adjust the Gamma Curve ----------//  
		LCD_Write_COM_DATA16(0x30, 0x0000);  
		LCD_Write_COM_DATA16(0x31, 0x0707);  
		LCD_Write_COM_DATA16(0x32, 0x0307);  
		LCD_Write_COM_DATA16(0x35, 0x0200);  
		LCD_Write_COM_DATA16(0x36, 0x0008);  
		LCD_Write_COM_DATA16(0x37, 0x0004);  
		LCD_Write_COM_DATA16(0x38, 0x0000);  
		LCD_Write_COM_DATA16(0x39, 0x0707);  
		LCD_Write_COM_DATA16(0x3C, 0x0002);  
		LCD_Write_COM_DATA16(0x3D, 0x1D04);  
		//------------------ Set GRAM area ---------------//  
		LCD_Write_COM_DATA16(0x50, 0x0000); // Horizontal GRAM Start Address  
		LCD_Write_COM_DATA16(0x51, 0x00EF); // Horizontal GRAM End Address  
		LCD_Write_COM_DATA16(0x52, 0x0000); // Vertical GRAM Start Address  
		LCD_Write_COM_DATA16(0x53, 0x013F); // Vertical GRAM Start Address  
		LCD_Write_COM_DATA16(0x60, 0xA700); // Gate Scan Line  
		LCD_Write_COM_DATA16(0x61, 0x0001); // NDL,VLE, REV   
		LCD_Write_COM_DATA16(0x6A, 0x0000); // set scrolling line  
		//-------------- Partial Display Control ---------//  
		LCD_Write_COM_DATA16(0x80, 0x0000);  
		LCD_Write_COM_DATA16(0x81, 0x0000);  
		LCD_Write_COM_DATA16(0x82, 0x0000);  
		LCD_Write_COM_DATA16(0x83, 0x0000);  
		LCD_Write_COM_DATA16(0x84, 0x0000);  
		LCD_Write_COM_DATA16(0x85, 0x0000);  
		//-------------- Panel Control -------------------//  
		LCD_Write_COM_DATA16(0x90, 0x0010);  
		LCD_Write_COM_DATA16(0x92, 0x0600);  
		LCD_Write_COM_DATA16(0x07, 0x0133); // 262K color and display ON        
		break;
#endif
#ifndef DISABLE_ILI9325D
	case ILI9325D:
		LCD_Write_COM_DATA16(0xE5, 0x78F0); // set SRAM internal timing
		LCD_Write_COM_DATA16(0x01, 0x0100); // set Driver Output Control  
		LCD_Write_COM_DATA16(0x02, 0x0200); // set 1 line inversion  
		LCD_Write_COM_DATA16(0x03, 0x1030); // set GRAM write direction and BGR=1.  
		LCD_Write_COM_DATA16(0x04, 0x0000); // Resize register  
		LCD_Write_COM_DATA16(0x08, 0x0207); // set the back porch and front porch  
		LCD_Write_COM_DATA16(0x09, 0x0000); // set non-display area refresh cycle ISC[3:0]  
		LCD_Write_COM_DATA16(0x0A, 0x0000); // FMARK function  
		LCD_Write_COM_DATA16(0x0C, 0x0000); // RGB interface setting  
		LCD_Write_COM_DATA16(0x0D, 0x0000); // Frame marker Position  
		LCD_Write_COM_DATA16(0x0F, 0x0000); // RGB interface polarity  
		//*************Power On sequence ****************//  
		LCD_Write_COM_DATA16(0x10, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB  
		LCD_Write_COM_DATA16(0x11, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]  
		LCD_Write_COM_DATA16(0x12, 0x0000); // VREG1OUT voltage  
		LCD_Write_COM_DATA16(0x13, 0x0000); // VDV[4:0] for VCOM amplitude  
		LCD_Write_COM_DATA16(0x07, 0x0001);  
		delay_ms(200); // Dis-charge capacitor power voltage  
		LCD_Write_COM_DATA16(0x10, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB  
		LCD_Write_COM_DATA16(0x11, 0x0227); // Set DC1[2:0], DC0[2:0], VC[2:0]  
		delay_ms(50); // Delay 50ms  
		LCD_Write_COM_DATA16(0x12, 0x000D); // 0012  
		delay_mslay(50); // Delay 50ms  
		LCD_Write_COM_DATA16(0x13, 0x1200); // VDV[4:0] for VCOM amplitude  
		LCD_Write_COM_DATA16(0x29, 0x000A); // 04  VCM[5:0] for VCOMH  
		LCD_Write_COM_DATA16(0x2B, 0x000D); // Set Frame Rate  
		delay_ms(50); // Delay 50ms  
		LCD_Write_COM_DATA16(0x20, 0x0000); // GRAM horizontal Address  
		LCD_Write_COM_DATA16(0x21, 0x0000); // GRAM Vertical Address  
		// ----------- Adjust the Gamma Curve ----------//  
		LCD_Write_COM_DATA16(0x30, 0x0000);  
		LCD_Write_COM_DATA16(0x31, 0x0404);  
		LCD_Write_COM_DATA16(0x32, 0x0003);  
		LCD_Write_COM_DATA16(0x35, 0x0405);  
		LCD_Write_COM_DATA16(0x36, 0x0808);  
		LCD_Write_COM_DATA16(0x37, 0x0407);  
		LCD_Write_COM_DATA16(0x38, 0x0303);  
		LCD_Write_COM_DATA16(0x39, 0x0707);  
		LCD_Write_COM_DATA16(0x3C, 0x0504);  
		LCD_Write_COM_DATA16(0x3D, 0x0808);  
		//------------------ Set GRAM area ---------------//  
		LCD_Write_COM_DATA16(0x50, 0x0000); // Horizontal GRAM Start Address  
		LCD_Write_COM_DATA16(0x51, 0x00EF); // Horizontal GRAM End Address  
		LCD_Write_COM_DATA16(0x52, 0x0000); // Vertical GRAM Start Address  
		LCD_Write_COM_DATA16(0x53, 0x013F); // Vertical GRAM Start Address  
		LCD_Write_COM_DATA16(0x60, 0xA700); // Gate Scan Line  
		LCD_Write_COM_DATA16(0x61, 0x0001); // NDL,VLE, REV   
		LCD_Write_COM_DATA16(0x6A, 0x0000); // set scrolling line  
		//-------------- Partial Display Control ---------//  
		LCD_Write_COM_DATA16(0x80, 0x0000);  
		LCD_Write_COM_DATA16(0x81, 0x0000);  
		LCD_Write_COM_DATA16(0x82, 0x0000);  
		LCD_Write_COM_DATA16(0x83, 0x0000);  
		LCD_Write_COM_DATA16(0x84, 0x0000);  
		LCD_Write_COM_DATA16(0x85, 0x0000);  
		//-------------- Panel Control -------------------//  
		LCD_Write_COM_DATA16(0x90, 0x0010);  
		LCD_Write_COM_DATA16(0x92, 0x0000);  
		LCD_Write_COM_DATA16(0x07, 0x0133); // 262K color and display ON        
		break;
#endif
#ifndef DISABLE_HX8340B
	case HX8340B:
		LCD_Write_COM_DATA16(0x26,0x0084); //PT=10,GON=0, DTE=0, D=0100
		delay_ms(40);
		LCD_Write_COM_DATA16(0x26,0x00B8); //PT=10,GON=1, DTE=1, D=1000
		delay_mslay(40);
		LCD_Write_COM_DATA16(0x26,0x00BC); //PT=10,GON=1, DTE=1, D=1100
		delay_ms(20);                           //新增加的延时  080421    
		// LCD_Write_COM_DATA(0x0001,0x0000);     // PTL='1' Enter Partail mode

		//Driving ability Setting
		LCD_Write_COM_DATA16(0x60,0x0000);
		LCD_Write_COM_DATA16(0x61,0x0006);
		LCD_Write_COM_DATA16(0x62,0x0000);
		LCD_Write_COM_DATA16(0x63,0x00C8);
		lib_delay_ms(20);

		//Gamma Setting
		LCD_Write_COM_DATA16(0x73,0x0070);
		LCD_Write_COM_DATA16(0x40,0x0000);
		LCD_Write_COM_DATA16(0x41,0x0040);
		LCD_Write_COM_DATA16(0x42,0x0045);
		LCD_Write_COM_DATA16(0x43,0x0001);
		LCD_Write_COM_DATA16(0x44,0x0060);
		LCD_Write_COM_DATA16(0x45,0x0005);
		LCD_Write_COM_DATA16(0x46,0x000C);
		LCD_Write_COM_DATA16(0x47,0x00D1);
		LCD_Write_COM_DATA16(0x48,0x0005);

		LCD_Write_COM_DATA16(0x50,0x0075);
		LCD_Write_COM_DATA16(0x51,0x0001);
		LCD_Write_COM_DATA16(0x52,0x0067);
		LCD_Write_COM_DATA16(0x53,0x0014);
		LCD_Write_COM_DATA16(0x54,0x00F2);
		LCD_Write_COM_DATA16(0x55,0x0007);
		LCD_Write_COM_DATA16(0x56,0x0003);
		LCD_Write_COM_DATA16(0x57,0x0049);
		delay_ms(20);

		//Power Setting
		LCD_Write_COM_DATA16(0x1F,0x0003); //VRH=4.65V     VREG1（GAMMA） 00~1E  080421    
		LCD_Write_COM_DATA16(0x20,0x0000); //BT (VGH~15V,VGL~-12V,DDVDH~5V)
		LCD_Write_COM_DATA16(0x24,0x0024); //VCOMH(VCOM High voltage3.2V)     0024/12    080421    11~40
		LCD_Write_COM_DATA16(0x25,0x0034); //VCOML(VCOM Low voltage -1.2V)    0034/4A    080421    29~3F 
		//****VCOM offset**///
		LCD_Write_COM_DATA16(0x23,0x002F); //VMF(no offset)                            
		delay_ms(20);

		//##################################################################
		// Power Supply Setting
		LCD_Write_COM_DATA16(0x18,0x0044); //I/P_RADJ,N/P_RADJ Noraml mode 60Hz
		LCD_Write_COM_DATA16(0x21,0x0001); //OSC_EN='1' start osc
		LCD_Write_COM_DATA16(0x01,0x0000); //SLP='0' out sleep
		LCD_Write_COM_DATA16(0x1C,0x0003); //AP=011
		LCD_Write_COM_DATA16(0x19,0x0006); // VOMG=1,PON=1, DK=0,
		delay_ms(20);

		//##################################################################
		// Display ON Setting
		LCD_Write_COM_DATA16(0x26,0x0084); //PT=10,GON=0, DTE=0, D=0100
		delay_ms(40);
		LCD_Write_COM_DATA16(0x26,0x00B8); //PT=10,GON=1, DTE=1, D=1000
		delay_ms(40);
		LCD_Write_COM_DATA16(0x26,0x00BC); //PT=10,GON=1, DTE=1, D=1100
		delay_ms(20);

		//SET GRAM AREA
		LCD_Write_COM_DATA16(0x02,0x0000); 
		LCD_Write_COM_DATA16(0x03,0x0000); 
		LCD_Write_COM_DATA16(0x04,0x0000);
		LCD_Write_COM_DATA16(0x05,0x00AF);
		LCD_Write_COM_DATA16(0x06,0x0000);
		LCD_Write_COM_DATA16(0x07,0x0000);
		LCD_Write_COM_DATA16(0x08,0x0000);
		LCD_Write_COM_DATA16(0x09,0x00DB);
		delay_ms(20);
		LCD_Write_COM_DATA16(0x16,0x0008);  //MV MX MY ML SET  0028横屏显示（此时LCD_Write_COM_DATA(0x0005,0x00DB);  LCD_Write_COM_DATA(0x0009,0x00AF);）
		LCD_Write_COM_DATA16(0x17,0x0005);//COLMOD Control Register (R17h)
		LCD_Write_COM(0x21);
		LCD_Write_COM(0x22);
		break;
#endif
#ifndef DISABLE_HX8340B_S
	case HX8340B_S:
		LCD_Write_COM(0xC1); 
		LCD_Write_DATA8(0xFF);
		LCD_Write_DATA8(0x83);
		LCD_Write_DATA8(0x40); 
		LCD_Write_COM(0x11); 
		lib_delay_ms(100);
		LCD_Write_COM(0xCA); 
		LCD_Write_DATA8(0x70);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0xD9); 
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x11); 

		LCD_Write_COM(0xC9); 
		LCD_Write_DATA8(0x90);
		LCD_Write_DATA8(0x49);
		LCD_Write_DATA8(0x10); 
		LCD_Write_DATA8(0x28);
		LCD_Write_DATA8(0x28); 
		LCD_Write_DATA8(0x10); 
		LCD_Write_DATA8(0x00); 
		LCD_Write_DATA8(0x06);
		delay_ms(20);
		LCD_Write_COM(0xC2); 
		LCD_Write_DATA8(0x60);
		LCD_Write_DATA8(0x71);
		LCD_Write_DATA8(0x01); 
		LCD_Write_DATA8(0x0E);
		LCD_Write_DATA8(0x05); 
		LCD_Write_DATA8(0x02); 
		LCD_Write_DATA8(0x09); 
		LCD_Write_DATA8(0x31);
		LCD_Write_DATA8(0x0A);
  
		LCD_Write_COM(0xc3); 
		LCD_Write_DATA8(0x67);
		LCD_Write_DATA8(0x30);
		LCD_Write_DATA8(0x61); 
		LCD_Write_DATA8(0x17);
		LCD_Write_DATA8(0x48); 
		LCD_Write_DATA8(0x07); 
		LCD_Write_DATA8(0x05); 
		LCD_Write_DATA8(0x33); 
		delay_ms(10);
		LCD_Write_COM(0xB5); 
		LCD_Write_DATA8(0x35);
		LCD_Write_DATA8(0x20);
		LCD_Write_DATA8(0x45); 

		LCD_Write_COM(0xB4); 
		LCD_Write_DATA8(0x33);
		LCD_Write_DATA8(0x25);
		LCD_Write_DATA8(0x4c); 
		delay_ms(10);
		LCD_Write_COM(0x3a); 
		LCD_Write_DATA8(0x05);
		LCD_Write_COM(0x29); 
		delay_ms(10);
		LCD_Write_COM(0x2a); 
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00); 
		LCD_Write_DATA8(0xaf); 
		LCD_Write_COM(0x2b); 
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00); 
		LCD_Write_DATA8(0xdb); 
		LCD_Write_COM(0x2c); 
		break;
#endif
#ifndef DISABLE_ST7735
	case ST7735:
		LCD_Write_COM(0x11);//Sleep exit 
		delay_ms(12);
 
		//ST7735R Frame Rate
		LCD_Write_COM(0xB1); 
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x2C);
		LCD_Write_DATA8(0x2D); 
		LCD_Write_COM(0xB2); 
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x2C);
		LCD_Write_DATA8(0x2D); 
		LCD_Write_COM(0xB3); 
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x2C);
		LCD_Write_DATA8(0x2D); 
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x2C);
		LCD_Write_DATA8(0x2D); 

		LCD_Write_COM(0xB4); //Column inversion 
		LCD_Write_DATA8(0x07); 
 
		//ST7735R Power Sequence
		LCD_Write_COM(0xC0); 
		LCD_Write_DATA8(0xA2);
		LCD_Write_DATA8(0x02);
		LCD_Write_DATA8(0x84); 
		LCD_Write_COM(0xC1);
		LCD_Write_DATA8(0xC5); 
		LCD_Write_COM(0xC2); 
		LCD_Write_DATA8(0x0A);
		LCD_Write_DATA8(0x00); 
		LCD_Write_COM(0xC3); 
		LCD_Write_DATA8(0x8A);
		LCD_Write_DATA8(0x2A); 
		LCD_Write_COM(0xC4); 
		LCD_Write_DATA8(0x8A);
		LCD_Write_DATA8(0xEE); 
 
		LCD_Write_COM(0xC5); //VCOM 
		LCD_Write_DATA8(0x0E); 
 
		LCD_Write_COM(0x36); //MX, MY, RGB mode 
		LCD_Write_DATA8(0xC8); 

		//ST7735R Gamma Sequence
		LCD_Write_COM(0xe0); 
		LCD_Write_DATA8(0x0f);
		LCD_Write_DATA8(0x1a); 
		LCD_Write_DATA8(0x0f);
		LCD_Write_DATA8(0x18); 
		LCD_Write_DATA8(0x2f);
		LCD_Write_DATA8(0x28); 
		LCD_Write_DATA8(0x20);
		LCD_Write_DATA8(0x22); 
		LCD_Write_DATA8(0x1f);
		LCD_Write_DATA8(0x1b); 
		LCD_Write_DATA8(0x23);
		LCD_Write_DATA8(0x37);
		LCD_Write_DATA8(0x00); 

		LCD_Write_DATA8(0x07); 
		LCD_Write_DATA8(0x02);
		LCD_Write_DATA8(0x10); 
		LCD_Write_COM(0xe1); 
		LCD_Write_DATA8(0x0f);
		LCD_Write_DATA8(0x1b); 
		LCD_Write_DATA8(0x0f);
		LCD_Write_DATA8(0x17); 
		LCD_Write_DATA8(0x33);
		LCD_Write_DATA8(0x2c); 
		LCD_Write_DATA8(0x29);
		LCD_Write_DATA8(0x2e); 
		LCD_Write_DATA8(0x30);
		LCD_Write_DATA8(0x30); 
		LCD_Write_DATA8(0x39);
		LCD_Write_DATA8(0x3f); 
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x07); 
		LCD_Write_DATA8(0x03);
		LCD_Write_DATA8(0x10);  

		LCD_Write_COM(0x2a);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x7f);
		LCD_Write_COM(0x2b);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x9f);

		LCD_Write_COM(0xF0); //Enable test command  
		LCD_Write_DATA8(0x01); 
		LCD_Write_COM(0xF6); //Disable ram power save mode 
		LCD_Write_DATA8(0x00); 
 
		LCD_Write_COM(0x3A); //65k mode 
		LCD_Write_DATA8(0x05); 
		LCD_Write_COM(0x29);//Display on
		break;
#endif
#ifndef DISABLE_PCF8833
	case PCF8833:
		LCD_Write_COM(0x01);
		LCD_Write_COM(0x25);
		LCD_Write_DATA8(0x40);
		LCD_Write_COM(0x11);
		delay_ms(10);
		LCD_Write_COM(0x20);
		LCD_Write_COM(0x38);
		LCD_Write_COM(0x29);
		LCD_Write_COM(0x13);
		LCD_Write_COM(0x36);
		LCD_Write_DATA8(0x60);
		LCD_Write_COM(0x3A);
		LCD_Write_DATA8(0x05);
		LCD_Write_COM(0x2A);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x7F);
		LCD_Write_COM(0xB4);
		LCD_Write_DATA8(0x03);
		LCD_Write_DATA8(0x08);
		LCD_Write_DATA8(0x0b);
		LCD_Write_DATA8(0x0e);
		LCD_Write_COM(0xBA);
		LCD_Write_DATA8(0x07);
		LCD_Write_DATA8(0x0D);
		LCD_Write_COM(0x2B);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x7F);
		LCD_Write_COM(0x2C);
		break;
#endif
#ifndef DISABLE_S1D19122
	case S1D19122:
		//************* Start Initial Sequence **********//

		LCD_Write_COM(0x11);
		LCD_Write_COM(0x13);
		LCD_Write_COM(0x29);
    
		//--------------  Display Control ---------//
		LCD_Write_COM(0xB0);

		LCD_Write_DATA8(0x05);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0xF0);
		LCD_Write_DATA8(0x0A);
		LCD_Write_DATA8(0x41);
		LCD_Write_DATA8(0x02); 
		LCD_Write_DATA8(0x0A);
		LCD_Write_DATA8(0x30);
		LCD_Write_DATA8(0x31);
		LCD_Write_DATA8(0x36);
		LCD_Write_DATA8(0x37);
		LCD_Write_DATA8(0x40);
		LCD_Write_DATA8(0x02);
		LCD_Write_DATA8(0x3F);
		LCD_Write_DATA8(0x40);
		LCD_Write_DATA8(0x02);
		LCD_Write_DATA8(0x81);
		LCD_Write_DATA8(0x04);
		LCD_Write_DATA8(0x05);
		LCD_Write_DATA8(0x64);

		// ----------- Gamma  Curve  Set3 Postive----------//
		LCD_Write_COM(0xFC);

		LCD_Write_DATA8(0x88);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x10);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x10);
		LCD_Write_DATA8(0x42);
		LCD_Write_DATA8(0x42);
		LCD_Write_DATA8(0x22);
		LCD_Write_DATA8(0x11);
		LCD_Write_DATA8(0x11);
		LCD_Write_DATA8(0x22);
		LCD_Write_DATA8(0x99);
		LCD_Write_DATA8(0xAA);
		LCD_Write_DATA8(0xAA);
		LCD_Write_DATA8(0xAA);
		LCD_Write_DATA8(0xBB);
		LCD_Write_DATA8(0xBB);
		LCD_Write_DATA8(0xAA);
		LCD_Write_DATA8(0x33);
		LCD_Write_DATA8(0x33);
		LCD_Write_DATA8(0x11);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0xC0);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);

		// ----------- Gamma  Curve  Set3 Negative----------//
		LCD_Write_COM(0xFD);

		LCD_Write_DATA8(0x88);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x10);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x10);
		LCD_Write_DATA8(0x42);
		LCD_Write_DATA8(0x42);
		LCD_Write_DATA8(0x22);
		LCD_Write_DATA8(0x11);
		LCD_Write_DATA8(0x11);
		LCD_Write_DATA8(0x22);
		LCD_Write_DATA8(0x99);
		LCD_Write_DATA8(0xAA);
		LCD_Write_DATA8(0xAA);
		LCD_Write_DATA8(0xAA);
		LCD_Write_DATA8(0xBB);
		LCD_Write_DATA8(0xBB);
		LCD_Write_DATA8(0xAA);
		LCD_Write_DATA8(0x33);
		LCD_Write_DATA8(0x33);
		LCD_Write_DATA8(0x11);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x03);

		// ----------- EVRSER Regulator Voltage Setting---------//
		LCD_Write_COM(0xBE);

		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x15);
		LCD_Write_DATA8(0x16);
		LCD_Write_DATA8(0x08);
		LCD_Write_DATA8(0x09);
		LCD_Write_DATA8(0x15);
		LCD_Write_DATA8(0x10);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);

		// -----------Module Definiton Setting---------//
		LCD_Write_COM(0xC0);

		LCD_Write_DATA8(0x0E);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);

		// -----------PWRDEF Power Ability Ddfinition----------//
		LCD_Write_COM(0xC1);

		LCD_Write_DATA8(0x2F);
		LCD_Write_DATA8(0x23);
		LCD_Write_DATA8(0xB4);
		LCD_Write_DATA8(0xFF);
		LCD_Write_DATA8(0x24);
		LCD_Write_DATA8(0x03);
		LCD_Write_DATA8(0x20);
		LCD_Write_DATA8(0x02);
		LCD_Write_DATA8(0x02);
		LCD_Write_DATA8(0x02);
		LCD_Write_DATA8(0x20);
		LCD_Write_DATA8(0x20);
		LCD_Write_DATA8(0x00);

		// -----------Other Setting----------//
		LCD_Write_COM(0xC2);
		LCD_Write_DATA8(0x03);
		LCD_Write_COM(0x26);
		LCD_Write_DATA8(0x08);
		LCD_Write_COM(0x35);
   
		LCD_Write_COM(0x36);
		LCD_Write_DATA8(0x64);
		LCD_Write_COM(0x3A);
		LCD_Write_DATA8(0x05);
		LCD_Write_COM(0x2A);
		LCD_Write_DATA8(0x013f);
		LCD_Write_COM(0x2B);
		LCD_Write_DATA8(0xEF);
		LCD_Write_COM(0x2c);

		// -----------RGB Setting----------//
		LCD_Write_COM(0x2D);
		uint8_t R=0;
		uint8_t G=0;
		uint8_t B=0;   
    
		for(uint8_t i=0;i<32;i++)
		{ 
			LCD_Write_DATA8(R);
			R=R+2;
		}
		for(uint8_t i=0;i<64;i++)
		{ 
			LCD_Write_DATA8(G);
			G=G+1;
		} 
		for(uint8_t i=0;i<32;i++)
		{ 
			LCD_Write_DATA8(B);
			B=B+2;
		}    
		break;
#endif
#ifndef DISABLE_HX8352A
	case HX8352A:
		LCD_Write_COM_DATA8(0x83, 0x02);  //TESTM=1 
             
		LCD_Write_COM_DATA8(0x85, 0x03);  //VDC_SEL=011
		LCD_Write_COM_DATA8(0x8B, 0x01);
		LCD_Write_COM_DATA8(0x8C, 0x93); //STBA[7]=1,STBA[5:4]=01,STBA[1:0]=11
        
		LCD_Write_COM_DATA8(0x91, 0x01); //DCDC_SYNC=1
        
		LCD_Write_COM_DATA8(0x83, 0x00); //TESTM=0
		//Gamma Setting

		LCD_Write_COM_DATA8(0x3E, 0xB0);
		LCD_Write_COM_DATA8(0x3F, 0x03);
		LCD_Write_COM_DATA8(0x40, 0x10);
		LCD_Write_COM_DATA8(0x41, 0x56);
		LCD_Write_COM_DATA8(0x42, 0x13);
		LCD_Write_COM_DATA8(0x43, 0x46);
		LCD_Write_COM_DATA8(0x44, 0x23);
		LCD_Write_COM_DATA8(0x45, 0x76);
		LCD_Write_COM_DATA8(0x46, 0x00);
		LCD_Write_COM_DATA8(0x47, 0x5E);
		LCD_Write_COM_DATA8(0x48, 0x4F);
		LCD_Write_COM_DATA8(0x49, 0x40);	
		//**********Power On sequence************
        
		LCD_Write_COM_DATA8(0x17, 0x91);
       
		LCD_Write_COM_DATA8(0x2B, 0xF9);
		delay_ms(10);
        
		LCD_Write_COM_DATA8(0x1B, 0x14);
        
		LCD_Write_COM_DATA8(0x1A, 0x11);
              
		LCD_Write_COM_DATA8(0x1C, 0x06);
        
		LCD_Write_COM_DATA8(0x1F, 0x42);
		delay_ms(20);
        
		LCD_Write_COM_DATA8(0x19, 0x0A);
     
		LCD_Write_COM_DATA8(0x19, 0x1A);
		delay_ms(40);
        
        
		LCD_Write_COM_DATA8(0x19, 0x12);
		delay_ms(40);
        
		LCD_Write_COM_DATA8(0x1E, 0x27);
		delay_ms(100);	   
        
        
		//**********DISPLAY ON SETTING***********
        
		LCD_Write_COM_DATA8(0x24, 0x60);
        
		LCD_Write_COM_DATA8(0x3D, 0x40);
        
		LCD_Write_COM_DATA8(0x34, 0x38);
        
		LCD_Write_COM_DATA8(0x35, 0x38);
        
		LCD_Write_COM_DATA8(0x24, 0x38);
		delay_ms(40);
        
		LCD_Write_COM_DATA8(0x24, 0x3C);
        
		LCD_Write_COM_DATA8(0x16, 0x1C);
        
		LCD_Write_COM_DATA8(0x01, 0x06);
        
		LCD_Write_COM_DATA8(0x55, 0x00); 

		LCD_Write_COM_DATA8(0x02, 0x00);
		LCD_Write_COM_DATA8(0x03, 0x00);
		LCD_Write_COM_DATA8(0x04, 0x00);
		LCD_Write_COM_DATA8(0x05, 0xef);
        
		LCD_Write_COM_DATA8(0x06, 0x00);
		LCD_Write_COM_DATA8(0x07, 0x00);
		LCD_Write_COM_DATA8(0x08, 0x01);
		LCD_Write_COM_DATA8(0x09, 0x8f);

		LCD_Write_COM(0x22);
		break;
#endif
#ifndef DISABLE_SSD1963_480
	case SSD1963_480:
		LCD_Write_COM(0xE2);		//PLL multiplier, set PLL clock to 120M
		LCD_Write_DATA8(0x23);	    //N=0x36 for 6.5M, 0x23 for 10M crystal
		LCD_Write_DATA8(0x02);
		LCD_Write_DATA8(0x54);
		LCD_Write_COM(0xE0);		// PLL enable
		LCD_Write_DATA8(0x01);
		delay_ms(10);
		LCD_Write_COM(0xE0);
		LCD_Write_DATA8(0x03);
		delay_ms(10);
		LCD_Write_COM(0x01);		// software reset
		delay_ms(100);
		LCD_Write_COM(0xE6);		//PLL setting for PCLK, depends on resolution
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0x1F);
		LCD_Write_DATA8(0xFF);

		LCD_Write_COM(0xB0);		//LCD SPECIFICATION
		LCD_Write_DATA8((is24bit) ? 0x20 : 0x00);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x01);		//Set HDP	479
		LCD_Write_DATA8(0xDF);
		LCD_Write_DATA8(0x01);		//Set VDP	271
		LCD_Write_DATA8(0x0F);
		LCD_Write_DATA8(0x00);

		LCD_Write_COM(0xB4);		//HSYNC
		LCD_Write_DATA8(0x02);		//Set HT	531
		LCD_Write_DATA8(0x13);
		LCD_Write_DATA8(0x00);		//Set HPS	8
		LCD_Write_DATA8(0x08);
		LCD_Write_DATA8(0x2B);		//Set HPW	43
		LCD_Write_DATA8(0x00);		//Set LPS	2
		LCD_Write_DATA8(0x02);
		LCD_Write_DATA8(0x00);

		LCD_Write_COM(0xB6);		//VSYNC
		LCD_Write_DATA8(0x01);		//Set VT	288
		LCD_Write_DATA8(0x20);
		LCD_Write_DATA8(0x00);		//Set VPS	4
		LCD_Write_DATA8(0x04);
		LCD_Write_DATA8(0x0c);		//Set VPW	12
		LCD_Write_DATA8(0x00);		//Set FPS	2
		LCD_Write_DATA8(0x02);

		LCD_Write_COM(0xBA);
		LCD_Write_DATA8(0x0F);		//GPIO[3:0] out 1

		LCD_Write_COM(0xB8);
		LCD_Write_DATA8(0x07);	    //GPIO3=input, GPIO[2:0]=output
		LCD_Write_DATA8(0x01);		//GPIO0 normal

		LCD_Write_COM(0x36);		//rotation
		LCD_Write_DATA8(0x22);

		LCD_Write_COM(0xF0);		//pixel data interface
		LCD_Write_DATA8(0x03);


		delay_ms(1);

		setXY(0, 0, 479, 271);
		LCD_Write_COM(0x29);		//display on

		LCD_Write_COM(0xBE);		//set PWM for B/L
		LCD_Write_DATA8(0x06);
		LCD_Write_DATA8(0xf0);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0xf0);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);

		LCD_Write_COM(0xd0); 
		LCD_Write_DATA8(0x0d);	

		LCD_Write_COM(0x2C); 
		break;
#endif
#ifndef DISABLE_SSD1963_800
	case SSD1963_800:
		LCD_Write_COM(0xE2);		//PLL multiplier, set PLL clock to 120M
		LCD_Write_DATA8(0x1E);	    //N=0x36 for 6.5M, 0x23 for 10M crystal
		LCD_Write_DATA8(0x02);
		LCD_Write_DATA8(0x54);
		LCD_Write_COM(0xE0);		// PLL enable
		LCD_Write_DATA8(0x01);
		delay_ms(10);
		LCD_Write_COM(0xE0);
		LCD_Write_DATA8(0x03);
		delay_ms(10);
		LCD_Write_COM(0x01);		// software reset
		delay_ms(100);
		LCD_Write_COM(0xE6);		//PLL setting for PCLK, depends on resolution
		LCD_Write_DATA8(0x03);
		LCD_Write_DATA8(0xFF);
		LCD_Write_DATA8(0xFF);

		LCD_Write_COM(0xB0);		//LCD SPECIFICATION
		LCD_Write_DATA8((is24bit) ? 0x24 : 0x04);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x03);		//Set HDP	799
		LCD_Write_DATA8(0x1F);
		LCD_Write_DATA8(0x01);		//Set VDP	479
		LCD_Write_DATA8(0xDF);
		LCD_Write_DATA8(0x00);

		LCD_Write_COM(0xB4);		//HSYNC
		LCD_Write_DATA8(0x03);		//Set HT	928
		LCD_Write_DATA8(0xA0);
		LCD_Write_DATA8(0x00);		//Set HPS	46
		LCD_Write_DATA8(0x2E);
		LCD_Write_DATA8(0x30);		//Set HPW	48
		LCD_Write_DATA8(0x00);		//Set LPS	15
		LCD_Write_DATA8(0x0F);
		LCD_Write_DATA8(0x00);

		LCD_Write_COM(0xB6);		//VSYNC
		LCD_Write_DATA8(0x02);		//Set VT	525
		LCD_Write_DATA8(0x0D);
		LCD_Write_DATA8(0x00);		//Set VPS	16
		LCD_Write_DATA8(0x10);
		LCD_Write_DATA8(0x10);		//Set VPW	16
		LCD_Write_DATA8(0x00);		//Set FPS	8
		LCD_Write_DATA8(0x08);

		LCD_Write_COM(0xBA);
		LCD_Write_DATA8(0x0F);		//GPIO[3:0] out 1

		LCD_Write_COM(0xB8);
		LCD_Write_DATA8(0x07);	    //GPIO3=input, GPIO[2:0]=output
		LCD_Write_DATA8(0x01);		//GPIO0 normal

		LCD_Write_COM(0x36);		//rotation
		LCD_Write_DATA8(0x22);

		LCD_Write_COM(0xF0);		//pixel data interface
		LCD_Write_DATA8(0x03);

		delay_ms(1);

		setXY(0, 0, 799, 479);
		LCD_Write_COM(0x29);		//display on

		LCD_Write_COM(0xBE);		//set PWM for B/L
		LCD_Write_DATA8(0x06);
		LCD_Write_DATA8(0xf0);
		LCD_Write_DATA8(0x01);
		LCD_Write_DATA8(0xf0);
		LCD_Write_DATA8(0x00);
		LCD_Write_DATA8(0x00);

		LCD_Write_COM(0xd0); 
		LCD_Write_DATA8(0x0d);	

		LCD_Write_COM(0x2C); 
		break;
#endif
#ifndef DISABLE_S6D1121
	case S6D1121:
		LCD_Write_COM_DATA16(0x11,0x2004);		
		LCD_Write_COM_DATA16(0x13,0xCC00);		
		LCD_Write_COM_DATA16(0x15,0x2600);	
		LCD_Write_COM_DATA16(0x14,0x252A);	
		LCD_Write_COM_DATA16(0x12,0x0033);		
		LCD_Write_COM_DATA16(0x13,0xCC04);		
		LCD_Write_COM_DATA16(0x13,0xCC06);		
		LCD_Write_COM_DATA16(0x13,0xCC4F);		
		LCD_Write_COM_DATA16(0x13,0x674F);
		LCD_Write_COM_DATA16(0x11,0x2003);
		LCD_Write_COM_DATA16(0x30,0x2609);		
		LCD_Write_COM_DATA16(0x31,0x242C);		
		LCD_Write_COM_DATA16(0x32,0x1F23);		
		LCD_Write_COM_DATA16(0x33,0x2425);		
		LCD_Write_COM_DATA16(0x34,0x2226);		
		LCD_Write_COM_DATA16(0x35,0x2523);		
		LCD_Write_COM_DATA16(0x36,0x1C1A);		
		LCD_Write_COM_DATA16(0x37,0x131D);		
		LCD_Write_COM_DATA16(0x38,0x0B11);		
		LCD_Write_COM_DATA16(0x39,0x1210);		
		LCD_Write_COM_DATA16(0x3A,0x1315);		
		LCD_Write_COM_DATA16(0x3B,0x3619);		
		LCD_Write_COM_DATA16(0x3C,0x0D00);		
		LCD_Write_COM_DATA16(0x3D,0x000D);		
		LCD_Write_COM_DATA16(0x16,0x0007);		
		LCD_Write_COM_DATA16(0x02,0x0013);		
		LCD_Write_COM_DATA16(0x03,0x0003);		
		LCD_Write_COM_DATA16(0x01,0x0127);		
		LCD_Write_COM_DATA16(0x08,0x0303);		
		LCD_Write_COM_DATA16(0x0A,0x000B);		
		LCD_Write_COM_DATA16(0x0B,0x0003);   
		LCD_Write_COM_DATA16(0x0C,0x0000);   
		LCD_Write_COM_DATA16(0x41,0x0000);    
		LCD_Write_COM_DATA16(0x50,0x0000);   
		LCD_Write_COM_DATA16(0x60,0x0005);    
		LCD_Write_COM_DATA16(0x70,0x000B);    
		LCD_Write_COM_DATA16(0x71,0x0000);    
		LCD_Write_COM_DATA16(0x78,0x0000);    
		LCD_Write_COM_DATA16(0x7A,0x0000);   
		LCD_Write_COM_DATA16(0x79,0x0007);		
		LCD_Write_COM_DATA16(0x07,0x0051);   
		LCD_Write_COM_DATA16(0x07,0x0053);		
		LCD_Write_COM_DATA16(0x79,0x0000);
		LCD_Write_COM(0x22);
		break;
#endif
	default:
		break;
	}

	removeCS(); 

	setColor(0xFFFF);
	setBackColor(0);
	cfont.font=0;
}

void UTFT::setXY(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	if (orient & SwapXY)
	{
		swap(x1, y1);
		swap(x2, y2);

		if (orient & ReverseX)
		{
			y1 = disp_y_size - y1;
			y2 = disp_y_size - y2;
			swap(y1, y2);
		}
		if (orient & ReverseY)
		{
			x1 = disp_x_size - x1;
			x2 = disp_x_size - x2;
			swap(x1, x2);
		}
	}
	else
	{
		if (orient & ReverseY)
		{
			y1 = disp_y_size - y1;
			y2 = disp_y_size - y2;
			swap(y1, y2);
		}
		if (orient & ReverseX)
		{
			x1 = disp_x_size - x1;
			x2 = disp_x_size - x2;
			swap(x1, x2);
		}
	}

	switch(displayModel)
	{
#if !(defined(DISABLE_HX8347A) && defined(DISABLE_HX8352A))
	case HX8347A:
	case HX8352A:
		LCD_Write_COM_DATA8(0x02,x1>>8);
		LCD_Write_COM_DATA8(0x03,x1);
		LCD_Write_COM_DATA8(0x04,x2>>8);
		LCD_Write_COM_DATA8(0x05,x2);
		LCD_Write_COM_DATA8(0x06,y1>>8);
		LCD_Write_COM_DATA8(0x07,y1);
		LCD_Write_COM_DATA8(0x08,y2>>8);
		LCD_Write_COM_DATA8(0x09,y2);
		LCD_Write_COM(0x22);      
		break;
#endif
#if !defined(DISABLE_ILI9327)
	case ILI9327:
		LCD_Write_COM(0x2a);
  		LCD_Write_DATA8(0x00,x1>>8);
  		LCD_Write_DATA8(0x00,x1);
  		LCD_Write_DATA8(0x00,x2>>8);
  		LCD_Write_DATA8(0x00,x2);
  		LCD_Write_COM(0x2b);
  		LCD_Write_DATA8(0x00,y1>>8);
  		LCD_Write_DATA8(0x00,y1);
  		LCD_Write_DATA8(0x00,y2>>8);
  		LCD_Write_DATA8(0x00,y2);
  		LCD_Write_COM(0x2c); 							 
		break;
#endif
#if !defined(DISABLE_SSD1289)
	case SSD1289:
		LCD_Write_COM_DATA16(0x44,(x2<<8)+x1);
		LCD_Write_COM_DATA16(0x45,y1);
		LCD_Write_COM_DATA16(0x46,y2);
		LCD_Write_COM_DATA16(0x4e,x1);
		LCD_Write_COM_DATA16(0x4f,y1);
		LCD_Write_COM(0x22); 
		break;
#endif
#if !(defined(DISABLE_ILI9325C) && defined(DISABLE_ILI9325D))
	case ILI9325C:
	case ILI9325D:
		LCD_Write_COM_DATA16(0x20,x1);
		LCD_Write_COM_DATA16(0x21,y1);
		LCD_Write_COM_DATA16(0x50,x1);
		LCD_Write_COM_DATA16(0x52,y1);
		LCD_Write_COM_DATA16(0x51,x2);
		LCD_Write_COM_DATA16(0x53,y2);
		LCD_Write_COM(0x22); 
		break;
#endif
#if !defined(DISABLE_HX8340B_8)
	case HX8340B:
		LCD_Write_COM_DATA16(0x02,0x0000); 
		LCD_Write_COM_DATA16(0x03,x1); 
		LCD_Write_COM_DATA16(0x04,0x0000);
		LCD_Write_COM_DATA16(0x05,x2);
		LCD_Write_COM_DATA16(0x06,0x0000);
		LCD_Write_COM_DATA16(0x07,y1);
		LCD_Write_COM_DATA16(0x08,0x0000);
		LCD_Write_COM_DATA16(0x09,y2); 
		LCD_Write_COM(0x22);
		break;
#endif
#if !(defined(DISABLE_HX8340B_S) && defined(DISABLE_ST7735) && defined(DISABLE_S1D19122))
	case HX8340B_S:
	case ST7735:
	case S1D19122:
		LCD_Write_COM(0x2a); 
  		LCD_Write_DATA8(x1>>8);
  		LCD_Write_DATA8(x1);
  		LCD_Write_DATA8(x2>>8);
  		LCD_Write_DATA8(x2);
		LCD_Write_COM(0x2b); 
  		LCD_Write_DATA8(y1>>8);
  		LCD_Write_DATA8(y1);
  		LCD_Write_DATA8(y2>>8);
  		LCD_Write_DATA8(y2);
		LCD_Write_COM(0x2c); 
		break;
#endif
#if !defined(DISABLE_PCF8833)
	case PCF8833:
		LCD_Write_COM(0x2a); 
  		LCD_Write_DATA8(x1);
  		LCD_Write_DATA8(x2);
		LCD_Write_COM(0x2b); 
  		LCD_Write_DATA8(y1);
  		LCD_Write_DATA8(y2);
		LCD_Write_COM(0x2c); 
		break;
#endif
#if !(defined(DISABLE_SSD1963_480) && defined(DISABLE_SSD1963_800))
	case SSD1963_480:
	case SSD1963_800:
		// In the following we use LCD_WRITE_BUS to write additional data without having to write RS again.
		LCD_Write_COM_DATA16(0x2a, y1>>8);
  		LCD_Write_Bus(y1);
  		LCD_Write_Bus(y2>>8);
  		LCD_Write_Bus(y2);
		LCD_Write_COM_DATA16(0x2b, x1>>8);
  		LCD_Write_Bus(x1);
  		LCD_Write_Bus(x2>>8);
  		LCD_Write_Bus(x2);
		LCD_Write_COM(0x2c); 
		break;
#endif
#ifndef DISABLE_S6D1121
	case S6D1121:
		LCD_Write_COM_DATA16(0x46,(x2 << 8) | x1);
		LCD_Write_COM_DATA16(0x47,y2);
		LCD_Write_COM_DATA16(0x48,y1);
		LCD_Write_COM_DATA16(0x20,x1);
		LCD_Write_COM_DATA16(0x21,y1);
		LCD_Write_COM(0x22);
		break;
#endif
	default:
		break;
	}
}

void UTFT::clrXY()
{
	if (orient & SwapXY)
	{
		setXY(0, 0, disp_y_size, disp_x_size);
	}
	else
	{
		setXY(0, 0, disp_x_size, disp_y_size);
	}
}

void UTFT::drawRect(int x1, int y1, int x2, int y2)
{
	if (x1 > x2)
	{
		swap(x1, x2);
	}
	if (y1 > y2)
	{
		swap(y1, y2);
	}

	drawHLine(x1, y1, x2-x1);
	drawHLine(x1, y2, x2-x1);
	drawVLine(x1, y1, y2-y1);
	drawVLine(x2, y1, y2-y1);
}

void UTFT::drawRoundRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(x1, x2);
	}
	if (y1>y2)
	{
		swap(y1, y2);
	}
	if ((x2-x1) > 4 && (y2-y1) > 4)
	{
		drawPixel(x1+1, y1+1);
		drawPixel(x2-1, y1+1);
		drawPixel(x1+1, y2-1);
		drawPixel(x2-1, y2-1);
		drawHLine(x1+2, y1, x2-x1-4);
		drawHLine(x1+2, y2, x2-x1-4);
		drawVLine(x1, y1+2, y2-y1-4);
		drawVLine(x2, y1+2, y2-y1-4);
	}
}

void UTFT::fillRect(int x1, int y1, int x2, int y2, Colour grad, uint8_t gradChange)
{
	if (x1>x2)
	{
		swap(x1, x2);
	}
	if (y1>y2)
	{
		swap(y1, y2);
	}

	Colour fcolourSave = fcolour;
	uint8_t gradCount = 0;
	if (orient & SwapXY)
	{
		for (int i = x1; i <= x2; i++)
		{
			drawVLine(i, y1, y2-y1);
			if (++gradCount == gradChange)
			{
				gradCount = 0;
				fcolour += grad;
			}
		}
	}
	else
	{
		for (int i = y1; i <= y2; i++)
		{
			drawHLine(x1, i, x2-x1);
			if (++gradCount == gradChange)
			{
				gradCount = 0;
				fcolour += grad;
			}
		}
	}
	fcolour = fcolourSave;
}

void UTFT::fillRoundRect(int x1, int y1, int x2, int y2, Colour grad, uint8_t gradChange)
{
	if (x1>x2)
	{
		swap(x1, x2);
	}
	if (y1>y2)
	{
		swap(y1, y2);
	}

	if ((x2-x1) > 4 && (y2-y1) > 4)
	{
		Colour fcolourSave = fcolour;
		uint8_t gradCount = 0;

		drawHLine(x1+2, y1, x2-x1-4);
		++y1;
		if (++gradCount == gradChange)
		{
			gradCount = 0;
			fcolour += grad;
		}
		drawHLine(x1+1, y1, x2-x1-2);
		++y1;
		if (++gradCount == gradChange)
		{
			gradCount = 0;
			fcolour += grad;
		}
		while (y1 + 1 < y2)
		{
			drawHLine(x1, y1, x2-x1);
			++y1;
			if (++gradCount == gradChange)
			{
				gradCount = 0;
				fcolour += grad;
			}
		}
		drawHLine(x1+1, y1, x2-x1-2);
		++y1;
		if (++gradCount == gradChange)
		{
			gradCount = 0;
			fcolour += grad;
		}
		drawHLine(x1+2, y1, x2-x1-4);
		
		fcolour = fcolourSave;
	}
}

void UTFT::drawCircle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
	
	assertCS();
	setXY(x, y + radius, x, y + radius);
	LCD_Write_DATA16(fcolour);
	setXY(x, y - radius, x, y - radius);
	LCD_Write_DATA16(fcolour);
	setXY(x + radius, y, x + radius, y);
	LCD_Write_DATA16(fcolour);
	setXY(x - radius, y, x - radius, y);
	LCD_Write_DATA16(fcolour);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;    
		setXY(x + x1, y + y1, x + x1, y + y1);
		LCD_Write_DATA16(fcolour);
		setXY(x - x1, y + y1, x - x1, y + y1);
		LCD_Write_DATA16(fcolour);
		setXY(x + x1, y - y1, x + x1, y - y1);
		LCD_Write_DATA16(fcolour);
		setXY(x - x1, y - y1, x - x1, y - y1);
		LCD_Write_DATA16(fcolour);
		setXY(x + y1, y + x1, x + y1, y + x1);
		LCD_Write_DATA16(fcolour);
		setXY(x - y1, y + x1, x - y1, y + x1);
		LCD_Write_DATA16(fcolour);
		setXY(x + y1, y - x1, x + y1, y - x1);
		LCD_Write_DATA16(fcolour);
		setXY(x - y1, y - x1, x - y1, y - x1);
		LCD_Write_DATA16(fcolour);
	}
	removeCS();
	clrXY();
}

void UTFT::fillCircle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
	
	assertCS();
	setXY(x, y + radius, x, y + radius);
	LCD_Write_DATA16(fcolour);
	setXY(x, y - radius, x, y - radius);
	LCD_Write_DATA16(fcolour);
	setXY(x - radius, y, x + radius, y);
	LCD_Write_Repeated_DATA16(fcolour, radius + radius);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;    
		setXY(x - x1, y + y1, x + x1, y + y1);
		LCD_Write_Repeated_DATA16(fcolour, x1 + x1);
		setXY(x - x1, y - y1, x + x1, y - y1);
		LCD_Write_Repeated_DATA16(fcolour, x1 + x1);
		setXY(x - y1, y + x1, x + y1, y + x1);
		LCD_Write_Repeated_DATA16(fcolour, y1 + y1);
		setXY(x - y1, y - x1, x + y1, y - x1);
		LCD_Write_Repeated_DATA16(fcolour, y1 + y1);
	}
	removeCS();
	clrXY();
}

void UTFT::clrScr()
{
	assertCS();
	clrXY();
	LCD_Write_Repeated_DATA16(0, disp_x_size+1, disp_y_size+1);
	removeCS();
}

void UTFT::fillScr(Colour c)
{
	assertCS();
	clrXY();
	LCD_Write_Repeated_DATA16(c, disp_x_size+1, disp_y_size+1);
	removeCS();
}

void UTFT::drawPixel(int x, int y)
{
	assertCS();
	setXY(x, y, x, y);
	LCD_Write_DATA16(fcolour);
	removeCS();
	clrXY();
}

void UTFT::drawLine(int x1, int y1, int x2, int y2)
{
	if (y1 == y2)
	{
		if (x1 > x2)
		{
			swap(x1, x2);
		}
		drawHLine(x1, y1, x2-x1);
	}
	else if (x1==x2)
	{
		if (y1>y2)
		{
			swap(y1, y2);
		}
		drawVLine(x1, y1, y2-y1);
	}
	else
	{
		// Draw a line using the Bresenham Algorithm (thanks Wikipedia)
		int dx = (x2 >= x1) ? x2 - x1 : x1 - x2;
		int dy = (y2 >= y1) ? y2 - y1 : y1 - y2;
		int sx = (x1 < x2) ? 1 : -1;
		int sy = (y1 < y2) ? 1 : -1;
		int err = dx - dy;
	 
		assertCS();
		for (;;)
		{
			setXY(x1, y1, x1, y1);
			LCD_Write_DATA16(fcolour);
			if (x1 == x2 && y1 == y2) break;
			int e2 = err + err;
			if (e2 > -dy)
			{ 
				err -= dy;
				x1 += sx;
			}
			if (e2 < dx)
			{ 
				err += dx;
				y1 += sy;
			}
		}
		removeCS();
	}

	clrXY();
}

void UTFT::drawHLine(int x, int y, int len)
{
	assertCS();
	setXY(x, y, x+len, y);
	LCD_Write_Repeated_DATA16(fcolour, len + 1);
	removeCS();
	clrXY();
}

void UTFT::drawVLine(int x, int y, int len)
{
	assertCS();
	setXY(x, y, x, y+len);
	LCD_Write_Repeated_DATA16(fcolour, len + 1);
	removeCS();
	clrXY();
}

// New print functions
void UTFT::setTextPos(uint16_t x, uint16_t y, uint16_t rm)
{
	textXpos = x;
	textYpos = y;
	uint16_t xSize = (orient & SwapXY) ? disp_y_size : disp_x_size;
	textRightMargin = (rm > xSize) ? xSize + 1 : rm;
    lastCharColData = 0UL;    // flag that we just set the cursor position, so no space before next character
}

size_t UTFT::print(const char *s, uint16_t x, uint16_t y, uint16_t rm)
{
	setTextPos(x, y, rm);
	return Print::print(s);
}

void UTFT::clearToMargin()
{
	if (textXpos < textRightMargin)
	{
		uint8_t ySize = cfont.y_size;
		if (textYpos + ySize > disp_y_size)
		{
			ySize = disp_y_size + 1 - textYpos;
		}

		assertCS();		
		setXY(textXpos, textYpos, textRightMargin - 1, textYpos + ySize - 1);
		LCD_Write_Repeated_DATA16(bcolour, textRightMargin - textXpos, ySize);
		removeCS();
		clrXY();
	}
}

// Write a UTF8 byte.
// If textYpos is off the end of the display, then don't write anything, just update textXpos and lastCharColData
size_t UTFT::write(uint8_t c)
{
	if (numContinuationBytesLeft == 0)
	{
		if (c < 0x80)
		{
			return writeNative(c);
		}
		else if ((c & 0xE0) == 0xC0)
		{
			charVal = (uint32_t)(c & 0x1F);
			numContinuationBytesLeft = 1;
			return 0;
		}
		else if ((c & 0xF0) == 0xE0)
		{
			charVal = (uint32_t)(c & 0x0F);
			numContinuationBytesLeft = 2;
			return 0;
		}
		else if ((c & 0xF8) == 0xF0)
		{
			charVal = (uint32_t)(c & 0x07);
			numContinuationBytesLeft = 3;
			return 0;
		}
		else if ((c & 0xFC) == 0xF8)
		{
			charVal = (uint32_t)(c & 0x03);
			numContinuationBytesLeft = 4;
			return 0;
		}
		else if ((c & 0xFE) == 0xFC)
		{
			charVal = (uint32_t)(c & 0x01);
			numContinuationBytesLeft = 5;
			return 0;
		}
		else
		{
			return writeNative(0x7F);
		}
	}
	else if ((c & 0xC0) == 0x80)
	{
		charVal = (charVal << 6) | (c & 0x3F);
		--numContinuationBytesLeft;
		if (numContinuationBytesLeft == 0)
		{
			return writeNative((charVal < 0x100) ? (uint8_t)charVal : 0x7F);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		// Bad UTF8 state
		numContinuationBytesLeft = 0;
		return writeNative(0x7F);
	}
}

// Write a character.
// If textYpos is off the end of the display, then don't write anything, just update textXpos and lastCharColData
size_t UTFT::writeNative(uint8_t c)
{
	if (translateFrom != 0)
	{
		const char* p = strchr(translateFrom, c);
		if (p != 0)
		{
			c = translateTo[p - translateFrom];
		}
	}

    if (c < cfont.firstChar || c > cfont.lastChar)
    {
		return 0;
    }
    
	uint8_t ySize = cfont.y_size;
    const uint8_t bytesPerColumn = (ySize + 7)/8;
	if (textYpos > disp_y_size)
	{
		ySize = 0;
	}
	else if (textYpos + ySize > disp_y_size)
	{
		ySize = disp_y_size + 1 - textYpos;
	}
    const uint8_t bytesPerChar = (bytesPerColumn * cfont.x_size) + 1;
    const uint8_t *fontPtr = (const uint8_t*)cfont.font + (bytesPerChar * (c - cfont.firstChar));
    
	const uint32_t cmask = (1UL << cfont.y_size) - 1;
    
    uint8_t nCols = *(uint8_t*)(fontPtr++);
	assertCS();
	
	if (lastCharColData != 0)	// if we have written anything other than spaces
	{
		uint8_t numSpaces = cfont.spaces;

		// Decide whether to add the full number of space columns first (auto-kerning)
		// We don't add a space column before a space character.
		// We add a space column after a space character if we would have added one between the preceding and following characters.
		uint32_t thisCharColData = *(uint32_t*)(fontPtr) & cmask;    // atmega328p is little-endian
		if (thisCharColData == 0)  // for characters with deliberate space row at the start, e.g. decimal point
		{
			thisCharColData = *(uint32_t*)(fontPtr + bytesPerColumn) & cmask;	// get the next column instead
		}

		bool kern = (numSpaces >= 2)
						? ((thisCharColData & lastCharColData) == 0)
						: (((thisCharColData | (thisCharColData << 1)) & (lastCharColData | (lastCharColData << 1))) == 0);
		if (kern)
		{
			--numSpaces;	// kern the character pair
		}
		while (numSpaces != 0 && textXpos < textRightMargin)
		{
			// Add a single space column after the character
			if (ySize != 0 && !transparentBackground)
			{
				setXY(textXpos, textYpos, textXpos, textYpos + ySize - 1);
				LCD_Write_Repeated_DATA16(bcolour, ySize);
			}
			++textXpos;
			--numSpaces;
		}      
	}

    while (nCols != 0 && textXpos < textRightMargin)
    {
		uint32_t colData = *(uint32_t*)(fontPtr);
		fontPtr += bytesPerColumn;
		if (colData != 0)
		{
			lastCharColData = colData & cmask;
		}
		if (ySize != 0)
		{
			bool doSetXY = true;
			if (orient & InvertText)
			{
				uint32_t mask = 1u << (ySize - 1);
				for (uint8_t i = 0; i < ySize; ++i)
				{
					if (colData & mask)
					{
						if (doSetXY)
						{
							setXY(textXpos, textYpos, textXpos, textYpos + ySize - i - 1);
							doSetXY = false;
						}
						LCD_Write_DATA16(fcolour);
					}
					else if (transparentBackground)
					{
						doSetXY = true;
					}
					else
					{
						if (doSetXY)
						{
							setXY(textXpos, textYpos, textXpos, textYpos + ySize - i - 1);
							doSetXY = false;
						}
						LCD_Write_DATA16(bcolour);
					}
					colData <<= 1;
				}
			}
			else
			{
				for (uint8_t i = 0; i < ySize; ++i)
				{
					if (colData & 1u)
					{
						if (doSetXY)
						{
							setXY(textXpos, textYpos + i, textXpos, textYpos + ySize - 1);
							doSetXY = false;
						}
						LCD_Write_DATA16(fcolour);
					}
					else if (transparentBackground)
					{
						doSetXY = true;
					}
					else
					{
						if (doSetXY)
						{
							setXY(textXpos, textYpos + i, textXpos, textYpos + ySize - 1);
							doSetXY = false;
						}
						LCD_Write_DATA16(bcolour);
					}
					colData >>= 1;
				}
			}
		}
		--nCols;
		++textXpos;
    }
 	removeCS();
	clrXY();
   
	return 1;
}

// Set up translation for characters. Useful for translating fullstop into decimal point, or changing the width of spaces.
// Either the first string passed must be NULL, or the two strings must have equal lengths as returned by strlen().
void UTFT::setTranslation(const char *tFrom, const char *tTo)
{
	translateFrom = tFrom;
	translateTo = tTo;
}

void UTFT::setFont(const uint8_t* font)
{
	cfont.font=font;
	cfont.x_size=fontbyte(0);
	cfont.y_size=fontbyte(1);
	cfont.spaces=fontbyte(2);
	cfont.firstChar=fontbyte(3);
	cfont.lastChar=fontbyte(4);
	cfont.font += 5;
}

void UTFT::drawBitmap(int x, int y, int sx, int sy, const uint16_t * data, int scale, bool byCols)
{
	int curY = y;
	assertCS();
	for (int ty = 0; ty < sy; ty++)
	{
		for (int i = 0; i < scale; ++i)
		{
			bool xySet = false;
			for (int tx = 0; tx < sx; tx++)
			{
				const int actualX = (orient & InvertBitmap) ? sx - tx - 1 : tx;
				const uint16_t col = data[(byCols) ? (actualX * sy) + ty : (ty * sx) + actualX];
				if (transparentBackground && col == 0xFFFF)
				{
					xySet = false;
				}
				else
				{
					if (!xySet)
					{
						if (orient & InvertBitmap)
						{
							setXY(x, curY, x + ((sx - tx) * scale) - 1, curY);
						}
						else
						{
							setXY(x + (tx * scale), curY, x + (sx * scale) - 1, curY);
						}
						xySet = true;
					}
					LCD_Write_Repeated_DATA16(col, scale);
				}
			}
			++curY;
		}
	}
	removeCS();
	clrXY();
}

#ifndef DISABLE_BITMAP_ROTATE

void UTFT::drawBitmap(int x, int y, int sx, int sy, uint16_t *data, int deg, int rox, int roy)
{
	double radian = deg*0.0175;  

	if (deg==0)
	{
		drawBitmap(x, y, sx, sy, data);
	}
	else
	{
		assertCS();
		for (int ty=0; ty<sy; ty++)
		{
			for (int tx=0; tx<sx; tx++)
			{
				uint16_t col = data[(ty*sx)+tx];

				int newx=x+rox+(((tx-rox)*cos(radian))-((ty-roy)*sin(radian)));
				int newy=y+roy+(((ty-roy)*cos(radian))+((tx-rox)*sin(radian)));

				setXY(newx, newy, newx, newy);
				LCD_Write_DATA16(col);
			}
		}
		removeCS();
	}
	clrXY();
}

#endif

// Draw a compressed bitmap. Data comprises alternate (repeat count - 1, data to write) pairs, both as 16-bit values.
void UTFT::drawCompressedBitmap(int x, int y, int sx, int sy, const uint16_t *data)
{
//	int curY = y;
	uint32_t count = 0;
	uint16_t col = 0;
	assertCS();
	for (int tx = 0; tx < sx; tx++)
	{
//		bool xySet = false;
		for (int ty = 0; ty < sy; ty++)
		{
			//const int actualX = (orient & InvertBitmap) ? sx - tx - 1 : tx;
			if (count == 0)
			{
				count = *data++;
				col = *data++;
			}
			else
			{
				--count;
			}
#if 1
			setXY(tx, ty, tx, ty);
#else
			if (!xySet)
			{
				if (orient & InvertBitmap)
				{
					setXY(x, curY, x + (sx - tx) - 1, curY);
				}
				else
				{
					setXY(x + tx, curY, x + sx - 1, curY);
				}
				xySet = true;
			}
#endif
			LCD_Write_DATA16(col);
		}
//		++curY;
	}
	removeCS();
	clrXY();	
}

void UTFT::lcdOff()
{
	assertCS();
	switch (displayModel)
	{
	case PCF8833:
		LCD_Write_COM(0x28);
		break;
	default:
		break;
	}
	removeCS();
}

void UTFT::lcdOn()
{
	assertCS();
	switch (displayModel)
	{
	case PCF8833:
		LCD_Write_COM(0x29);
		break;
	default:
		break;
	}
	removeCS();
}

void UTFT::setContrast(uint8_t c)
{
	assertCS();
	switch (displayModel)
	{
	case PCF8833:
		if (c>64) c=64;
		LCD_Write_COM_DATA8(0x25, c);
		break;
	default:
		break;
	}
	removeCS();
}

uint16_t UTFT::getDisplayXSize() const
{
	return ((orient & SwapXY) ? disp_y_size : disp_x_size) + 1;
}

uint16_t UTFT::getDisplayYSize() const
{
	return ((orient & SwapXY) ? disp_x_size : disp_y_size) + 1;
}
