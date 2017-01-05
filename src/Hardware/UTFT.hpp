/*
  UTFT.h - Arduino/chipKit library support for Color TFT LCD Boards
  Copyright (C)2010-2012 Henning Karlsen. All right reserved
  
  This library is the continuation of my ITDB02_Graph, ITDB02_Graph16
  and RGB_GLCD libraries for Arduino and chipKit. As the number of 
  supported display modules and controllers started to increase I felt 
  it was time to make a single, universal library as it will be much 
  easier to maintain in the future.

  Basic functionality of this library was origianlly based on the 
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

#ifndef UTFT_h
#define UTFT_h

#include "Print.hpp"
#include "OneBitPort.hpp"
#include "DisplayOrientation.hpp"

#include "HW_AVR_defines.h"

enum DisplayType {
	HX8347A,
	ILI9327,
	SSD1289,
	ILI9325C,
	ILI9325D,
	HX8340B,
	HX8340B_S,
	HX8352A,
	ST7735,
	PCF8833,
	S1D19122,
	SSD1963_480,
	SSD1963_800,
	S6D1121,
	
	// Aliases for particular display models
	ITDB32 = HX8347A,								// HX8347-A (16bit)
	ITDB32WC = ILI9327,								// ILI9327  (16bit)
	ITDB32S	= SSD1289, TFT01_32 = SSD1289,			// SSD1289  (16bit)
	ITDB24 = ILI9325C,								// ILI9325C (8bit)
	ITDB24D = ILI9325D, ITDB24DWOT = ILI9325D,		// ILI9325C (8bit)
	ITDB28 = ILI9325D, TFT01_24_8 = ILI9325D,		// ILI9325D (8bit)
	TFT01_24_16 = ILI9325D,							// ILI9325D (16bit)
	ITDB22 = HX8340B,								// HX8340-B (8bit)
	ITDB22SP = HX8340B_S,							// HX8340-B (Serial)
	ITDB32WD = HX8352A,								// HX8352-A (16bit)
	TFT01_32WD = HX8352A,							// HX8352A	(16bit)
	ITDB18SP = ST7735,								// ST7735   (Serial)
	LPH9135 = PCF8833,								// PCF8833	(Serial)
	ITDB25H = S1D19122,								// S1D19122	(16bit)
	ITDB43 = SSD1963_480,							// SSD1963	(16bit) 480x272
	ITDB50 = SSD1963_800,							// SSD1963	(16bit) 800x480
	ITDB24E = S6D1121,								// S6D1121
};

enum TransferMode
{
	// The serial ones must come first
	TModeSerial4pin,
	TModeSerial5pin,
	
	// All the rest are parallel
	TModeLowestParallel,
	TMode8bit = TModeLowestParallel,
	TMode9bit,
	TMode16bit,
	
	// Aliased for modes of particular display models
	ITDB32Mode = TMode16bit,							// HX8347-A (16bit)
	ITDB32WCMode = TMode16bit,							// ILI9327  (16bit)
	ITDB32SMode	= TMode16bit,							// SSD1289  (16bit)
	TFT01_32Mode = TMode16bit,							// SSD1289  (16bit)
	ITDB24Mode = TMode8bit,								// ILI9325C (8bit)
	ITDB24DMode = TMode8bit,							// ILI9325D (8bit)
	ITDB24DWOTMode = TMode8bit,							// ILI9325D (8bit)
	ITDB28Mode = TMode8bit,								// ILI9325D (8bit)
	TFT01_24_8Mode = TMode8bit,							// ILI9325D (8bit)
	TFT01_24_16Mode = TMode16bit,						// ILI9325D (16bit)
	ITDB22Mode = TMode8bit,								// HX8340-B (8bit)
	ITDB22SPMode = TModeSerial4pin,						// HX8340-B (Serial)
	ITDB32WDMode = TMode16bit,							// HX8352-A (16bit)
	TFT01_32WDMode = TMode16bit,						// HX8352A	(16bit)
	ITDB18SPMode = TModeSerial5pin,						// ST7735   (Serial)
	LPH9135Mode = TModeSerial5pin,						// PCF8833	(Serial)
	ITDB25HMode = TMode16bit,							// S1D19122	(16bit)
	ITDB43Mode = TMode16bit,							// SSD1963	(16bit) 480x272
	ITDB50Mode = TMode16bit,							// SSD1963	(16bit) 800x480
	ITDB24E_8Mode = TMode8bit,							// S6D1121	(8bit)
	ITDB24E_16Mode = TMode16bit							// S6D1121	(16bit)
};

// This describes the structure we use to store font information.
// The first 5 fields are also the layout of the data in the font header.
struct FontDescriptor
{
	uint8_t x_size;
	uint8_t y_size;
	uint8_t spaces;
	uint8_t firstChar;
	uint8_t lastChar;
	const uint8_t* font;
};


typedef uint16_t Colour;

class UTFT : public Print
{
public:
	UTFT(DisplayType model, TransferMode pMode, unsigned int RS, unsigned int WR, unsigned int CS, unsigned int RST, unsigned int SER_LATCH = 0);
	void InitLCD(DisplayOrientation po, bool is24bit);
	void clrScr();
	void fillScr(Colour c);
	void drawPixel(int x, int y);
	void drawLine(int x1, int y1, int x2, int y2);
	void drawRect(int x1, int y1, int x2, int y2);
	void drawRoundRect(int x1, int y1, int x2, int y2);
	void fillRect(int x1, int y1, int x2, int y2, Colour grad = 0, uint8_t gradChange = 1);
	void fillRoundRect(int x1, int y1, int x2, int y2, Colour grad = 0, uint8_t gradChange = 1);
	void drawCircle(int x, int y, int radius);
	void fillCircle(int x, int y, int radius);
		
	// Colour management. We store colours in native 16-bit format, but support conversion from RGB.
	void setColor(Colour c) { fcolour = c; }
	void setBackColor(Colour c) { bcolour = c; }
	void setTransparentBackground(bool b) { transparentBackground = b; }
	static constexpr Colour fromRGB(uint8_t r, uint8_t g, uint8_t b);
		
	// New print functions
	// Set up translation for characters. Useful for translating fullstop into decimal point, or changing the width of spaces.
	// Either the first string passed must be NULL, or the two strings must have equal lengths as returned by strlen().
	void setTranslation(const char *tFrom, const char *tTo);
	virtual size_t write(uint8_t c) /*override*/;
	void setTextPos(uint16_t x, uint16_t y, uint16_t rm = 9999);
	void clearToMargin();
	size_t print(const char *s, uint16_t x, uint16_t y, uint16_t rm = 9999);
	using Print::print;
		
	void setFont(const uint8_t* font);
	void drawBitmap(int x, int y, int sx, int sy, const uint16_t *data, int scale = 1, bool byCols = true);
#ifndef DISABLE_BITMAP_ROTATE
	void drawBitmap(int x, int y, int sx, int sy, const uint16_t *data, int deg, int rox, int roy);
#endif
	void drawCompressedBitmap(int x, int y, int sx, int sy, const uint16_t *data);
	void lcdOff();
	void lcdOn();
	void setContrast(uint8_t c);
	uint16_t getDisplayXSize() const;
	uint16_t getDisplayYSize() const;
	uint16_t getTextX() const { return textXpos; }
	uint16_t getTextY() const { return textYpos; }
	uint16_t getFontHeight() const { return cfont.y_size; }
	static uint16_t GetFontHeight(const uint8_t *f) { return reinterpret_cast<const FontDescriptor*>(f)->y_size; }

private:
	uint16_t fcolour, bcolour;
	bool transparentBackground;
	DisplayOrientation orient;
	uint16_t disp_x_size, disp_y_size;
	DisplayType displayModel;
	TransferMode displayTransferMode;
	
	// Port descriptors. In 9-bit parallel mode, portSDA is used as the latch port. In 5-bit serial mode, portRS is used as the extra port.
	OneBitPort portRS, portWR, portCS, portRST, portSDA, portSCL;
	
	FontDescriptor cfont;
	uint16_t textXpos, textYpos, textRightMargin;
	uint32_t lastCharColData;		// used for auto kerning
	const char* translateFrom;
	const char* translateTo;
	
	uint32_t charVal;
	uint8_t numContinuationBytesLeft;

	size_t writeNative(uint8_t c);

	// Hardware interface
	void LCD_Write_Bus(uint16_t VHL);
	void LCD_Write_Again(uint16_t num);
	void _set_direction_registers();

	// Low level interface
	void LCD_Write_COM(uint8_t VL);
	void LCD_Write_DATA8(uint8_t VL);
	void LCD_Write_DATA16(uint16_t VHL);
	void LCD_Write_Repeated_DATA16(uint16_t VHL, uint16_t num);
	void LCD_Write_Repeated_DATA16(uint16_t VHL, uint16_t num1, uint16_t num2);
	void LCD_Write_COM_DATA16(uint8_t com1, uint16_t dat1);
	void LCD_Write_COM_DATA8(uint8_t com1, uint8_t dat1);
		
	void drawHLine(int x, int y, int len);
	void drawVLine(int x, int y, int len);
	void setXY(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
	void clrXY();
		
	bool isParallel() const;
	
	void assertCS() const
	{
		portCS.setLow();
	}

	void removeCS() const
	{
		portCS.setHigh();
	}

	void assertReset() const
	{
		portRST.setLow();
	}

	void removeReset() const
	{
		portRST.setHigh();
	}
		
	void setRSLow() const
	{
		portRS.setLow();
	}

	void setRSHigh() const
	{
		portRS.setHigh();
	}
};

inline constexpr uint16_t UTFT::fromRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 248) << 8) | ((g & 252) << 3) | (b >> 3);
}

#endif
