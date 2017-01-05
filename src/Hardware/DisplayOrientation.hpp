/*
 * DisplayOrientation.hpp
 *
 * Created: 04/11/2014 17:34:21
 *  Author: David
 */ 


#ifndef DISPLAYORIENTATION_H_
#define DISPLAYORIENTATION_H_

// Enumeration to define the orientation of the display.
// To keep the code small and fast, we use individual bits to say what needs to be done on the display.
// Then we define the supported orientations in terms of those bits.
enum DisplayOrientation {
	Default = 0x00,
	SwapXY = 0x01,
	ReverseX = 0x02,
	ReverseY = 0x04,
	InvertText = 0x08,
	InvertBitmap = 0x10,
	
	Portrait = Default,
	Landscape = SwapXY | ReverseY | InvertBitmap,
	InvPortrait = ReverseX | ReverseY | InvertText | InvertBitmap,
	InvLandscape = SwapXY | ReverseX | InvertText
};

#endif /* DISPLAYORIENTATION_H_ */
